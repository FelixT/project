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
    Modifier(std::vector<Sample*> *samplesPointer, std::vector<Modifier*> *modifiersPointer);
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
    void setSelected(int index);
    void setMode(int index);
    void setState(int index);
    void setParameter(int index);
    void setEquation(std::string eqn);
    void updateEuclidean();
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
    
    enum modifierParameter {
        PARAMETER_IDLE,
        PARAMETER_INTERVAL,
        PARAMETER_DELAY,
        PARAMETER_BPM
    };
    
    enum modifierState {
        STATE_IDLE,
        STATE_SAMPLE,
        STATE_MODIFIER
    };
    
    struct euclideanPreset {
        std::string name;
        int hits;
        int interval;
    };
    
    std::vector<euclideanPreset> euclideanPresets;
    
    juce::Label modifierSelectLabel;
    juce::ComboBox modifierSelect;
    juce::Label modifierParameterLabel;
    juce::ComboBox modifierParameter;
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
    
    juce::TextButton modifierRefresh;
    
    std::string equation = "X";
    
    juce::Label modifierPosition;
    
    double interval = 16.0;
    double min = 2.0; // integer doubles up as number of pulses in a Euclidean rhythm
    double max = 4.0;
    double step = 0.25;
    int selected = -1; // index of selected sample/modifier (depending on state)
    
    enum modifierParameter parameter = PARAMETER_IDLE;
    modifierState state = STATE_IDLE;
    
    bool slidersChanged = false;
    
    std::vector<bool> modifierEuclideanRhythm;
    
    enum modifierMode {
        MODE_RANDOM,
        MODE_EUCLIDEAN,
        MODE_EQUATION
    };
    
    modifierMode mode = MODE_RANDOM;
    
    int euclideanPosition = -1; // also used for equations
    
    std::vector<Sample*> *samples;
    std::vector<Modifier*> *modifiers;
    long roundedInterval;
    long roundedStep;
    
    juce::Colour background = juce::Colour(100, 100, 100);
    
};

#endif /* Modifier_hpp */
