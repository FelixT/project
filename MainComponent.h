#pragma once

#include <JuceHeader.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "Sample.h"
#include "Modifier.h"


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void saveState();
    void loadState();
    void resetSamples();

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    

private:
    //==============================================================================

    void getWaveValue(float &outLeft, float &outRight);
    void updateWaveParams();

    //==============================================================================
    
    int precision = 2;  // number of decimal places of beats to use in calculations
    float volume = 0.5f; // 0 - 1
    
    long pow10(float input, int power);
    void addSample();
    
    double curSampleRate = 0.0;
    float curBeat = 0.f;
    
    float curWaveAngle = 0.f;
    bool waveEnabled = true;
    float waveAngleDelta = 0.f;
    float waveNoteLength = 0.f;
    
    float samplesPerBeat = 0.f;
    float beatsPerSample = 0.f; // equal to 1/samplesPerBeat
    
    float beatDelta = 0.f; // number 
    
    // these are stored as *10^precision hence correct to precision dp when /10^precision
    long roundBeat = 0;
    long prevBeat = 0;
    
    juce::Slider freqSlider;
    juce::Label freqLabel;
    
    juce::Slider bpmSlider;
    juce::Label bpmLabel;
    
    juce::Slider noteLengthSlider;
    juce::Label noteLengthLabel;
    
    juce::Label curBeatLabel;
    
    juce::TextButton sampleAddButton;
    
    juce::TextButton resetBeatButton;
    
    juce::TextButton saveStateButton;
    juce::TextButton loadStateButton;
    
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
        STATE_COMPLETE,
        STATE_ERROR
    };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
