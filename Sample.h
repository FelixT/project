#ifndef Sample_hpp
#define Sample_hpp

#include <stdio.h>
#include <string>
#include <JuceHeader.h>
#include "WaveformView.h"
#include "Parameter.h"

class Sample : public juce::Component {
    
public:
    Sample(juce::AudioFormatManager *manager);
    ~Sample();
    std::vector<Parameter*> getParams();
    void paint (juce::Graphics& g) override;
    void resized() override;
    void getValue(float &outLeft, float &outRight, long roundedBeat, long prevBeat);
    void updateParams(double trackBpm, double trackSampleRate, int precision);
    void updateBuffers(int numSamples);
    std::string toString();
    void setLabel(std::string label);
    void setStart(double start);
    void setEnd(double end);
    void setInterval(double val);
    void setDelay(double val);
    double getDelay();
    void setVolume(double val);
    void setPath(std::string path);
    void setParameter(int index);
    void setCollapsed(bool c);
    void setMuted(bool m);
    void setBpm(double val);
    bool isCollapsed();
    void disable();
    juce::String getLabel();

    bool isLoaded = false;
    
    // parameters
    Parameter bpm;
    Parameter cropStart; // percent
    Parameter cropEnd; // percent
    Parameter interval;
    Parameter delay;

private:
    void browse();
    long pow10(float input, int power);
    bool loadSample(juce::File file);
    void mouseDown(const juce::MouseEvent &event) override;
    
    bool collapsed = false;
    std::string samplePath;
    
    juce::Label sampleLabel;
    juce::TextButton sampleCollapseButton;
    juce::TextButton sampleBrowseButton;
    juce::Slider sampleVolumeSlider;
    
    juce::TextButton sampleSoloButton;
    juce::TextButton sampleMuteButton;
    
    juce::AudioFormatReader *sampleReader = nullptr;
    juce::AudioBuffer<float> *sampleBuffer = nullptr;
    
    double curPos = 0.0;
    double sampleLength = 0.0;
    double volume = 1.0;
    
    double playbackRate = 0.0;
    double startPos = 0.0; // in samples
    double endPos = 0.0; // in samples
    
    double sampleRate = 0.0;
    
    long roundedInterval = 1;
    long roundedDelay = 0;
    
    const float *inLeftBuffer;
    const float *inRightBuffer;
    
    bool slidersChanged = false;

    bool isWaiting = true;
    bool isDisabled = false;
    bool isMuted = false;
    bool isSoloed = false;
    
    juce::AudioFormatManager *formatManager;
    
    WaveformView *sampleWaveform;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sample)
};

#endif /* Sample_hpp */
