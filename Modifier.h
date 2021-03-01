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

class Modifier : public juce::Component {
    
public:
    Modifier();
    ~Modifier();
    void paint (juce::Graphics& g) override;
    void resized() override;
    
private:
    juce::ComboBox modifierObject;
    
};

#endif /* Modifier_hpp */
