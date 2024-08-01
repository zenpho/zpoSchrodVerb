/*
 * PluginProcessor.cpp
 * for zpoSchrodVerb
 *
 * Last edited 27/12/2022.
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "FloatParameter.h"

#include "CombFilter.h"
const int kNumCombFilters = 8;

#include "AllpassFilter.h"
const int kNumApassFilters = 4;

//========================================================================
/**
*/
class ZpoAudioProcessor  : public AudioProcessor
{
public:
    //====================================================================
    ZpoAudioProcessor();
    ~ZpoAudioProcessor();

    //====================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //====================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //====================================================================
    const String getName() const override;

    const String getInputChannelName (int channelIndex) const override;
    const String getOutputChannelName (int channelIndex) const override;
    bool isInputChannelStereoPair (int index) const override;
    bool isOutputChannelStereoPair (int index) const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool silenceInProducesSilenceOut() const override;
    double getTailLengthSeconds() const override;

    //====================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //====================================================================
    void setParameter (int parameterIndex, float newValue) override;
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:

  //-----------------------------------------
  // private methods
  void filterDelayTimeSpread( float *fDelayTimeArray,
                float *fSpreadNormArray,
                int iNumElements,
                float fBasicDelayTime,
                float fSpreadAmount,
                bool bHarmonic );
  
  void updateRandomisedSpreadValues( unsigned int seed );
  void updateCombFilterDelayTimes( void );
  void updateApassFilterDelayTimes( void );
  
  void setupFilters( void );
  void updateFilters( void );
  
  inline float hardclip( float x )
  {
    if( x > 1 )  return 1;
    if( x < -1)  return -1;
    return x;
  }

  //-----------------------------------------
  // filter object arrays
  CombFilter *pCombFilters[kNumCombFilters];
  AllpassFilter *pApassFilters[kNumApassFilters];
  
  //-----------------------------------------
  // filter delay time arrays
  float fCombDelayTimes[kNumCombFilters];
  float fApassDelayTimes[kNumApassFilters];
  
  // spreading values in normalised range 0 to 1
  float fCombSpreadNorm[kNumCombFilters];
  float fApassSpreadNorm[kNumApassFilters];
  
  //-----------------------------------------
  // parameters
  FloatParameter* pDryWet = nullptr;
  FloatParameter* pHarmonicMode = nullptr;

  FloatParameter* pCombFeedback = nullptr;
  FloatParameter* pCombDelayMain = nullptr;
  FloatParameter* pCombDelaySpread = nullptr;

  FloatParameter* pAllpassFeedback = nullptr;
  FloatParameter* pAllpassDelayMain = nullptr;
  FloatParameter* pAllpassDelaySpread = nullptr;
  
  FloatParameter* pSeed = nullptr;

  void debugParamValues();

  // the next line must be the final line of the class definition
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZpoAudioProcessor)
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
