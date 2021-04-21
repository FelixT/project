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
#include <vector>
#include <string>
#include <sstream>
#include <stack>
#include <cctype>

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
    std::string toString();
    
    void setInterval(double val);
    void setMin(double val);
    void setMax(double val);
    void setStep(double val);
    void setSample(int index);
    void setMode(int index);
    void setFunction(int index);
private:
    long pow10(float input, int power);
    std::vector<bool> genEuclideanRhythm(int length, int pulses);
    void tickEuclidean(long roundedBeat, long prevBeat);
    void tickRandom(long roundedBeat, long prevBeat);
    void tickEquation(long roundedBeat, long prevBeat);
    void getParams();
    void changeMode();
    void populatePresets();
    void selectPreset();
    double parseEquation(std::string input);
    std::string toolTip();
    
    enum modifierState {
        MODIFIER_IDLE,
        MODIFIER_INTERVAL,
        MODIFIER_DELAY,
        MODIFIER_BPM
    };
    
    struct euclideanPreset {
        std::string name;
        int hits;
        int interval;
    };
    
    std::vector<euclideanPreset> euclideanPresets;
    
    juce::Label modifierSelectLabel;
    juce::ComboBox modifierSelect;
    juce::Label modifierFunctionLabel;
    juce::ComboBox modifierFunction;
    juce::Label modifierIntervalLabel;
    juce::Slider modifierInterval;
    juce::Label modifierMinLabel;
    juce::Slider modifierMin;
    juce::Label modifierMaxLabel;
    juce::Slider modifierMax;
    juce::Label modifierStepLabel;
    juce::Slider modifierStep;
    juce::TextButton modifierChangeMode;
    juce::TextEditor modifierEquation;
    juce::Label modifierEquationLabel;
    juce::Label modifierPresetLabel;
    juce::ComboBox modifierPresetMenu;
    juce::TextButton modifierHelp;
    juce::TextButton modifierForward;
    juce::TextButton modifierBack;
    
    std::string equation = "X";
    
    juce::Label modifierPosition;
    
    double interval = 16.0;
    double min = 2.0; // integer doubles up as number of pulses in a Euclidean rhythm
    double max = 4.0;
    double step = 0.25;
    int sampleIndex = -1;
    int functionIndex = 0;
    
    bool slidersChanged = false;
    
    std::vector<bool> modifierEuclideanRhythm;
    
    enum modifierMode {
        MODE_RANDOM,
        MODE_EUCLIDEAN,
        MODE_EQUATION
    };
    
    modifierMode mode = MODE_RANDOM;
    
    int euclideanPosition = -1;
    
    std::vector<Sample*> *samples;
    long roundedInterval;
    long roundedStep;
    
    juce::Colour background = juce::Colour(100, 100, 100);
    
    int tmp = 0; // delete this
};

#endif /* Modifier_hpp */
