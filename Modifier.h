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
    void refreshDropdownItems();
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
    void setCycleLength(double len);
    void setPulseDuration(double len);
    void setEuclideanHits(double num);
    
    void updateEuclidean();
    
    Parameter randomInterval;
    Parameter randomMin;
    Parameter randomMax;
    Parameter randomStep;
    
    Parameter cycleLength; // euclidean & equation
    Parameter pulseDuration; // euclidean & equation
    Parameter euclideanNumHits;
    
private:
    long pow10(float input, int power);
    std::vector<bool> genEuclideanRhythm(int length, int pulses);
    void tickEuclidean(long roundedBeat, long prevBeat);
    void tickRandom(long roundedBeat, long prevBeat);
    void tickEquation(long roundedBeat, long prevBeat);
    void hideAllControls();
    void showRandomControls();
    void showEuclideanControls();
    void drawEuclideanPattern(juce::Graphics& g);
    void showEquationControls();
    void changeMode();
    void populatePresets();
    void selectPreset();
    void updateSelected();
    
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
    
    int selected = -1; // index of selected sample/modifier (depending on state)
    
    enum modifierParameter parameter = PARAMETER_IDLE;
    modifierState state = STATE_IDLE;
    
    
    bool slidersChanged = false;
    bool dropdownChanged = false;
    //bool modeChanged = false;
    
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
    long roundedPulseDuration;
    
    juce::Colour background = juce::Colour(100, 100, 100);
    
};

#endif /* Modifier_hpp */
