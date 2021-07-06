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
#include "PatternView.h"

class Modifier : public juce::Component, private juce::Timer {
    
public:
    Modifier(std::vector<Sample*> *samplesPointer, std::vector<Modifier*> *modifiersPointer, int index);
    ~Modifier();
    void paint (juce::Graphics& g) override;
    void resized() override;
    void refreshDropdownItems();
    void tick(long roundedBeat, long prevBeat);
    void updateParams(int precision);
    void highlight(int frameInterval = 20, int length = 8);
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
    std::vector<Parameter*> getParams();
    
    void updateEuclidean();
    
    Parameter randomInterval;
    Parameter randomMin;
    Parameter randomMax;
    Parameter randomStep;
    
    Parameter cycleLength; // euclidean & equation
    Parameter pulseDuration; // euclidean & equation
    Parameter euclideanNumHits;
    
private:
    long pow10(double input, int power);
    std::vector<bool> genEuclideanRhythm(int length, int pulses);
    void tickPlayPattern(long roundedBeat, long prevBeat);
    void tickRandom(long roundedBeat, long prevBeat);
    void tickEquation(long roundedBeat, long prevBeat);
    void hideAllControls();
    void showRandomControls();
    void showCustomControls();
    void showEuclideanControls();
    void showEquationControls();
    void changeMode();
    void populateParameters();
    void parameterChanged();
    void populatePresets();
    void selectPreset();
    void updateSelected();
    bool isValidParam(int index);
    bool isValidSample(int index);
    bool isValidModifier(int index);
    void timerCallback() override;
    void mouseDown(const juce::MouseEvent &event) override;
    double parseEquation(std::string input);
    std::string toolTip();
    
    int modifierIndex;
    
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
    
    PatternView patternView;
    juce::Viewport patternViewport;
    
    juce::Label modifierIndexLabel;
    
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
    
    juce::Label modifierCyclePositionLabel;
    juce::TextButton modifierForward;
    juce::TextButton modifierBack;
    
    std::vector<euclideanPreset> euclideanPresets;

    std::string equation = "X";
    
    juce::Label modifierPosition;
    
    int selected = -1; // index of selected sample/modifier (depending on state)
    
    modifierState state = STATE_IDLE;
    
    
    bool slidersChanged = false;
    bool dropdownChanged = false;
    bool modeChanged = false;
    
    std::vector<bool> pattern;
    int patternPosition = -1; // also used for equations

    
    enum modifierMode {
        MODE_RANDOM,
        MODE_EUCLIDEAN,
        MODE_EQUATION,
        MODE_CUSTOM
    };
    
    modifierMode mode = MODE_RANDOM;
    
    std::vector<Sample*> *samples;
    std::vector<Modifier*> *modifiers;
    long roundedInterval;
    long roundedStep;
    long roundedPulseDuration;
    
    juce::Colour background = juce::Colour(80, 80, 80);
    
    std::vector<Parameter*> params;
    int parameterIndex = -1;
    
    bool highlighted = false;
    int highlightPos = 0; // highlight animation position
    int highlightLen = 8; // number of frames of highlight animation
    
};

#endif /* Modifier_hpp */
