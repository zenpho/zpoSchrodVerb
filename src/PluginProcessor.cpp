/*
 * PluginProcessor.cpp
 * for zpoSchrodVerb
 *
 * Last edited 27/12/2022.
*/

#include "PluginProcessor.h"


//========================================================================
ZpoAudioProcessor::ZpoAudioProcessor()
{
  // see also setParameter(), setState__(), getState__(), and 
  addParameter( pDryWet = new FloatParameter( 0.8f, "D--W" ) );

  addParameter( pHarmonicMode = new FloatParameter( 1.0f, "Mode" ) );

  addParameter( pCombFeedback = new FloatParameter( 0.8f, "Cfbk" ) );
  addParameter( pCombDelayMain = new FloatParameter( 0.1f, "Cdly" ) );
  addParameter( pCombDelaySpread = new FloatParameter( 0.01f, "Cspr" ) );

  addParameter( pAllpassFeedback = new FloatParameter( 0.8f, "Afbk" ) );
  addParameter( pAllpassDelayMain = new FloatParameter( 0.5f, "Adly" ) );
  addParameter( pAllpassDelaySpread = new FloatParameter( 0.01f, "Aspr" ) );
  
  sranddev();
  addParameter( pSeed = new FloatParameter( (float)rand()/RAND_MAX, "Seed") );
  updateRandomisedSpreadValues( pSeed->getValue() * UINT_MAX );
  
  setupFilters();
}

ZpoAudioProcessor::~ZpoAudioProcessor()
{
}

//========================================================================
void ZpoAudioProcessor::setupFilters(void)
{
  // set up comb filters
  for (int i=0; i<kNumCombFilters; i++)
  {
    pCombFilters[i] = new CombFilter();
    fCombDelayTimes[i] = 0;
    fCombSpreadNorm[i] = 0;
  }

  // set up allpass filters
  for (int i=0; i<kNumApassFilters; i++)
  {
    pApassFilters[i] = new AllpassFilter();
    fApassDelayTimes[i] = 0;
    fApassSpreadNorm[i] = 0;
  }

  // set filter delay times
  updateCombFilterDelayTimes();
  updateApassFilterDelayTimes();

  // update filter properties
  updateFilters();
}

void ZpoAudioProcessor::updateFilters(void) // apply filter dtime and fbk
{
  // comb filter properties
  for(int i=0; i<kNumCombFilters; i++)
  {
    pCombFilters[i]->setFeedbackGain( pCombFeedback->getValue() );
    pCombFilters[i]->setDelayInSamples( fCombDelayTimes[i] );
  }

  // allpass filter properties
  for(int i=0; i<kNumApassFilters; i++)
  {
    pApassFilters[i]->setFeedbackGain( pAllpassFeedback->getValue() );
    pApassFilters[i]->setDelayInSamples( fApassDelayTimes[i] );
  }

}

void ZpoAudioProcessor::updateRandomisedSpreadValues(unsigned int seed)
{
  srand( seed );
  
  // randomised float between 0 and 1 for later spreading comb filters
  for (int i=0; i<kNumCombFilters; i++)
  {
    fCombSpreadNorm[i] = (float)rand()/RAND_MAX;
    std::cout << i << " " << fCombSpreadNorm[i] << "\n";
  }

  // randomised float between 0 and 1 for later spreading apass filters
  for (int i=0; i<kNumApassFilters; i++)
  {
    fApassSpreadNorm[i] = (float)rand()/RAND_MAX;
  }
}

void ZpoAudioProcessor::updateCombFilterDelayTimes() // calc from parameters
{
  filterDelayTimeSpread(
    fCombDelayTimes,
    fCombSpreadNorm,
    (int)kNumCombFilters,
    (pCombDelayMain->getValue() * 1500) + 500, // range 500-2000
    (pCombDelaySpread->getValue() * 25), // -25 to +25 sample spread
    (bool)(pHarmonicMode->getValue() > 0.5)
  );
}

void ZpoAudioProcessor::updateApassFilterDelayTimes() // calc from parameters
{
  filterDelayTimeSpread(
    fApassDelayTimes,
    fApassSpreadNorm,
    (int)kNumApassFilters,
    (pAllpassDelayMain->getValue() * 4210) + 200, // range 200 - 4410
    (pAllpassDelaySpread->getValue() * 50), // -50 to +50 sample spread
    false // not harmonic
  );
}

void ZpoAudioProcessor::filterDelayTimeSpread( // calc spread
  float *fDelayTimeArray,
  float *fSpreadNormArray,
  int iNumElements,
  float fBasicDelayTime,
  float fSpreadAmount,
  bool bHarmonic )
{
  // apply offsets
  for( int i=0; i < iNumElements; i++ )
  {
    // randomised float between 0 and 1
    float r = fSpreadNormArray[i];
    
    // scale and shift to range -spread to +spread
    r = (r * (fSpreadAmount*2)) - fSpreadAmount;

    // harmonic distribution?
    if (bHarmonic)
      fDelayTimeArray[i] = ((float)fBasicDelayTime / (i+1) ) + r;
    else
      fDelayTimeArray[i] = fBasicDelayTime + r;
    
    // enforce valid ranges
    if( fDelayTimeArray[i] > ASSUMEDSAMPLERATE ) fDelayTimeArray[i] = ASSUMEDSAMPLERATE;
    if( fDelayTimeArray[i] <= 0 ) fDelayTimeArray[i] = 1;
  }
}

void ZpoAudioProcessor::processBlock ( // single buffer replacing
  AudioSampleBuffer& buffer,
  MidiBuffer& midiMessages )
{
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // buffer variables
    float *pBufferL, *pBufferR;
    pBufferL = pBufferR = nullptr;
  
    if( getNumInputChannels() >= 1 )
      pBufferL = buffer.getWritePointer(0);
  
    if( getNumInputChannels() >= 2 )
      pBufferR = buffer.getWritePointer(1);
  
    // signal variables
    float inL, inR, inMono, outL, outR;
    float cmbSig;
    float apSig;
  
    // initialise
    inL = inR = inMono = cmbSig = apSig = outL = outR = 0;

    // update filter parameters
    updateFilters();
  
    int samplesToProcess = buffer.getNumSamples();
    while( samplesToProcess-- )
    {
        if( pBufferL ) inL = *pBufferL;
        if( pBufferR ) inR = *pBufferR;
      
        inMono = (inL + inR) * 0.5;
        inMono = hardclip( inMono );
      
        // parallel comb filtering
        cmbSig = 0;
        for (int i=0; i<kNumCombFilters; i++)
          cmbSig += hardclip( pCombFilters[i]->filter(inMono) );
        cmbSig /= kNumCombFilters;
      
        // feed the (summed) comb filter signal into the 0th allpass
        apSig = hardclip( pApassFilters[0]->filter(cmbSig) );

        // continue series allpass filtering - from the 1th allpass
        for (int i=1; i<kNumApassFilters; i++)
          apSig = hardclip( pApassFilters[i]->filter(apSig) );
      
        outL =  apSig * pDryWet->getValue();
        outR =  apSig * pDryWet->getValue();
      
        outL += inL * ( 1 - pDryWet->getValue() );
        outR += inR * ( 1 - pDryWet->getValue() );
      
        if( pBufferL ) (*pBufferL++) = outL;
        if( pBufferR ) (*pBufferR++) = outR;
    }
}

//========================================================================
const String ZpoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

const String ZpoAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String ZpoAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool ZpoAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool ZpoAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool ZpoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ZpoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ZpoAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double ZpoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ZpoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: at least 1, even if you're not really implementing programs
}

int ZpoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ZpoAudioProcessor::setCurrentProgram (int index)
{
}

const String ZpoAudioProcessor::getProgramName (int index)
{
    return String();
}

void ZpoAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//========================================================================
void ZpoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void ZpoAudioProcessor::releaseResources()
{
}

//========================================================================
bool ZpoAudioProcessor::hasEditor() const
{
    return false;
}

AudioProcessorEditor* ZpoAudioProcessor::createEditor()
{
    return 0;
}

//========================================================================
void ZpoAudioProcessor::setParameter(int parameterIndex, float newValue)
{
  AudioProcessorParameter* targetParam = getParameters()[parameterIndex];
 
  float oldValue = targetParam->getValue();
  targetParam->setValue( newValue );
  
  if ( targetParam == pHarmonicMode )
  {
      if( (oldValue <= 0.5 && newValue > 0.5) ||
          (oldValue >= 0.5 && newValue < 0.5) )
      {
        updateCombFilterDelayTimes();
        updateApassFilterDelayTimes();
      }
  }
  
  if ( targetParam == pCombDelayMain ||
       targetParam == pCombDelaySpread )
  {
      updateCombFilterDelayTimes();
  }
      
  if ( targetParam == pAllpassDelayMain ||
       targetParam == pAllpassDelaySpread )
  {
      updateApassFilterDelayTimes();
  }
  
  if ( targetParam == pSeed )
  {
    updateRandomisedSpreadValues( pSeed->getValue() * UINT_MAX );
  }

}

void ZpoAudioProcessor::debugParamValues()
{
  
}

void ZpoAudioProcessor::getStateInformation (MemoryBlock& destData)
{ // store
  struct paramsChunk {
      float drWe;
      float hMod;
      float cFbk;
      float cDly;
      float cSpr;
      float aFbk;
      float aDly;
      float aSpr;
      float seed;
  } save;

  save.drWe = pDryWet->getValue();
  save.hMod = pHarmonicMode->getValue();

  save.cDly = pCombDelayMain->getValue();
  save.cFbk = pCombFeedback->getValue();
  save.cSpr = pCombDelaySpread->getValue();
  
  save.aDly = pAllpassDelayMain->getValue();
  save.aFbk = pAllpassFeedback->getValue();
  save.aSpr = pAllpassDelaySpread->getValue();
  
  save.seed = pSeed->getValue();
  
  destData.append( &save, sizeof( paramsChunk ) );
}

void ZpoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{ // restore
  struct paramsChunk {
      float drWe;
      float hMod;
      float cFbk;
      float cDly;
      float cSpr;
      float aFbk;
      float aDly;
      float aSpr;
      float seed;
  } load;

  if( sizeInBytes > sizeof( paramsChunk ) ) return;
  memcpy( &load, data, sizeof( paramsChunk ) );
  
  pDryWet->setValue( load.drWe );
  pHarmonicMode->setValue( load.hMod );

  pCombDelayMain->setValue( load.cDly );
  pCombFeedback->setValue( load.cFbk );
  pCombDelaySpread->setValue( load.cSpr );
  
  pAllpassDelayMain->setValue( load.aDly );
  pAllpassFeedback->setValue( load.aFbk );
  pAllpassDelaySpread->setValue( load.aSpr );
  
  pSeed->setValue( load.seed );
  
  // calc new dtimes
  updateCombFilterDelayTimes();
  updateApassFilterDelayTimes();
}




AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZpoAudioProcessor();
}
