/*
 * FloatParameter.h
 *
 * Last edited 26/12/2022.
 *
*/

#ifndef FLOATPARAMETER_H_INCLUDED
#define FLOATPARAMETER_H_INCLUDED

class FloatParameter : public AudioProcessorParameter
{
public:

    FloatParameter (float defaultParameterValue, const String& paramName)
       : defaultValue (defaultParameterValue),
         value (defaultParameterValue),
         name (paramName)
    {
    }

    float getValue() const override
    {
        return value;
    }

    void setValue (float newValue) override
    {
        value = newValue;
    }

    float getDefaultValue() const override
    {
        return defaultValue;
    }

    String getName (int maximumStringLength) const override
    {
        return name;
    }

    String getLabel() const override
    {
        return String();
    }

    float getValueForText (const String& text) const override
    {
        return text.getFloatValue();
    }

private:
    float defaultValue, value;
    String name;
};

#endif  // FLOATPARAMETER_H_INCLUDED
