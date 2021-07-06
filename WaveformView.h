#ifndef WaveformView_hpp
#define WaveformView_hpp

#include <stdio.h>
#include <JuceHeader.h>

class WaveformView : public juce::Component, private juce::Timer {
public:
    WaveformView(double *inSampleLength, double *inCurPos, double *inVolume, double *inStartPos, double *inEndPos, juce::AudioFormatManager *inFormatManager);
    ~WaveformView();
    void setSource(juce::InputSource *source);
private:
    void paint (juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void timerCallback() override;
    
    double *sampleLength;
    double *curPos;
    double *volume;
    double *startPos;
    double *endPos;
    
    juce::AudioThumbnailCache waveThumbnailCache;
    juce::AudioThumbnail waveThumbnail;
    juce::AudioFormatManager *formatManager;
    
    bool cropped = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformView)
};

#endif
