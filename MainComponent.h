#pragma once

#include <JuceHeader.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "Sample.h"
#include "Modifier.h"
#include "Parameter.h"


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    std::string getState();
    void saveState();
    void loadState();
    void resetSamples();
    void resetModifiers();

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    

private:
    //==============================================================================

    void getWaveValue(float &outLeft, float &outRight);
    void updateWaveParams();
    void addSample();
    void addModifier();
    void timerCallback() override;

    //==============================================================================
    
    int precision = 2;  // number of decimal places of beats to use in calculations
    int viewsMargin = 20; // margin between samples and modifiers
    int componentMargin = 4; // margins around individual modifiers/samples
    int controlsHeight = 130; // height of main controls at the top
    
    float waveVolume = 0.5f; // 0 - 1
    float masterVolume = 1.0f;
    
    long pow10(double input, int power);
    
    double curSampleRate = 0.0;
    double curBeat = 0.0;
    
    double curWaveAngle = 0.0;
    double waveAngleDelta = 0.0;
    bool waveEnabled = true;
        
    double samplesPerBeat = 0.f;
    double beatsPerSample = 0.f; // equal to 1/samplesPerBeat
    
    float beatDelta = 0.f; // number 
    
    // these are stored as *10^precision hence correct to precision dp when /10^precision
    long roundBeat = 0;
    long prevBeat = 0;
    
    
    juce::TooltipWindow tooltipWindow;
    
    // params
    Parameter bpm;
    Parameter sineFrequency;
    Parameter sineNoteLength;
    
    juce::Slider masterVolumeSlider;
    juce::Label masterVolumeLabel;
    
    juce::Label curBeatLabel;
    
    juce::TextButton sampleAddButton;
    juce::TextButton resetBeatButton;
    juce::TextButton saveStateButton;
    juce::TextButton loadStateButton;
    juce::TextButton modifierAddButton;
    juce::TextButton newProjectButton;
    juce::TextButton playPauseButton;
    
    juce::AudioFormatManager formatManager;
    
    juce::Viewport samplesViewport;
    juce::Component samplesComponent;
    
    juce::Viewport modifiersViewport;
    juce::Component modifiersComponent;
    
    std::vector<Sample*> samples;
    std::vector<Modifier*> modifiers;
    
    enum loaderState {
        STATE_OUT_OF_BLOCK,
        STATE_IN_PROJECT_BLOCK,
        STATE_IN_SAMPLE_BLOCK,
        STATE_IN_MODIFIER_BLOCK,
        STATE_COMPLETE,
        STATE_ERROR
    };
    
    bool loading = false;
    
    bool paused = false;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
