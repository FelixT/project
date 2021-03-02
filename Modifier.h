//
//  Modifier.hpp
//  Proto - App
//
//  Created by Felix Thomas on 01/03/2021.
//

#ifndef Modifier_hpp
#define Modifier_hpp

#include <stdio.h>
#include <JuceHeader.h>

#include "Sample.h"

class Modifier : public juce::Component {
    
public:
    Modifier(std::vector<Sample*> *samplesPointer);
    ~Modifier();
    void paint (juce::Graphics& g) override;
    void resized() override;
    void updateDropdown();
    void tick(long roundedBeat, long prevBeat);
    void updateParams(int precision);
private:
    long pow10(float input, int power);
    
    enum modifierState {
        MODIFIER_IDLE,
        MODIFIER_INTERVAL,
        MODIFIER_DELAY,
        MODIFIER_BPM
    };
    
    juce::ComboBox modifierSelect;
    juce::ComboBox modifierFunction;
    juce::Slider modifierInterval;
    juce::Slider modifierMin;
    juce::Slider modifierMax;
    juce::Slider modifierStep;
    
    std::vector<Sample*> *samples;
    long roundedInterval;
    
};

#endif /* Modifier_hpp */
