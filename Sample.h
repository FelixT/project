//
//  Sample.hpp
//  Proto - App
//
//  Created by Felix Thomas on 20/02/2021.
//

#ifndef Sample_hpp
#define Sample_hpp

#include <stdio.h>
#include <string>
#include <JuceHeader.h>

class Sample : public juce::Component {
    
public:
    Sample(juce::AudioFormatManager *manager);
    ~Sample();
    void paint (juce::Graphics& g) override;
    void resized() override;
    void getValue(float &outLeft, float &outRight, long roundedBeat, long prevBeat);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill, float trackBpm, int precision, long roundedBeat, long prevBeat, float curBeat);
    void updateParams(float trackBpm, int precision, int numSamples);
    std::string toString();
    void setLabel(std::string label);
    void setStart(double start);
    void setEnd(double end);
    void setInterval(double val);
    void setDelay(double val);
    void setVolume(double val);
    void setPath(std::string path);
    void setBpm(double val);
    bool isCollapsed();
    void setCollapsed(bool c);
    juce::String getLabel();

    
    bool isLoaded = false;

private:
    void browse();
    long pow10(float input, int power);
    void getParams();
    
    bool collapsed = false;
    std::string samplePath;
    
    juce::Label sampleLabel;
    juce::TextButton sampleCollapseButton;
    juce::TextButton sampleBrowseButton;
    juce::Slider sampleBpmSlider;
    juce::Label sampleBpmLabel;
    juce::Slider sampleIntervalSlider;
    juce::Label sampleIntervalLabel;
    juce::Slider sampleCropLeftSlider;
    juce::Label sampleCropLeftLabel;
    juce::Slider sampleCropRightSlider;
    juce::Label sampleCropRightLabel;
    juce::Slider sampleDelaySlider;
    juce::Label sampleDelayLabel;
    juce::Slider sampleVolumeSlider;
    juce::Label sampleVolumeLabel;
    
    juce::AudioFormatReader *sampleReader = nullptr;
    juce::AudioBuffer<float> *sampleBuffer = nullptr;
    
    float curPos = 0.f;
    
    double sampleBpm = 120.f;
    double interval = 1.f;
    double delay = 0.f;
    double volume = 1.f;
    
    float playbackRate = 0.f;
    float startPos = 0.f;
    float endPos = 0.f;
    
    long roundedInterval = 1;
    long roundedDelay = 0;
    
    const float *inLeftBuffer;
    const float *inRightBuffer;
    
    bool isWaiting = true;
    bool slidersChanged = false;
    
    juce::AudioFormatManager *formatManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sample)
};

#endif /* Sample_hpp */
