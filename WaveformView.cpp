#include "WaveformView.h"

WaveformView::WaveformView(double *inSampleLength, double *inCurPos, double *inVolume, double *inStartPos, double *inEndPos, juce::AudioFormatManager *inFormatManager)
    : waveThumbnailCache(5), waveThumbnail(512, *inFormatManager, waveThumbnailCache) {
        
    sampleLength = inSampleLength;
    curPos = inCurPos;
    volume = inVolume;
    startPos = inStartPos;
    endPos = inEndPos;
    formatManager = inFormatManager;
    
    setOpaque(true);
        
    startTimer(40);
    
}

WaveformView::~WaveformView() {
    
}

void WaveformView::setSource(juce::InputSource *source) {
    waveThumbnail.setSource(source);
}

void WaveformView::mouseDown(const juce::MouseEvent &event) {
    if(event.mods.isLeftButtonDown()) {
        cropped = !cropped;
    } else if(event.mods.isRightButtonDown()) {
        juce::PopupMenu m;
        m.addSectionHeader("Zoom");
        m.addItem("Full", true, !cropped, ([this]{ cropped = false; }));
        m.addItem("Cropped", true, cropped, ([this]{ cropped = true; }));
        m.show();
    }
}

void WaveformView::timerCallback() {
    repaint();
}

void WaveformView::paint(juce::Graphics& g) {
    float width = (float)getLocalBounds().getWidth();
    float height = (float)getLocalBounds().getHeight();
    
    // background
    g.setColour(juce::Colour(25, 25, 25));
    g.fillRect(0.f, 0.f, width, height);
    g.setColour(juce::Colour(225, 225, 225));
    g.drawRect(0.f, 0.f, width, height);

    if(*sampleLength == 0) {
        g.drawText("Unloaded", 0.f, 0.f, width, height, juce::Justification::centred);
        return;
    } else {
        
        float startProportion = (float)*startPos / (float)*sampleLength;
        float endProportion = (float)*endPos / (float)*sampleLength;
        double lengthSeconds = waveThumbnail.getTotalLength();
        
        float playProportion = (float)*curPos / (float)*sampleLength;
        // TODO: fix crash if endpos and startpos are the same
        if(cropped) playProportion = ((float)*curPos - (float)*startPos) / ((float)*endPos - (float)*startPos);
        
        double drawStart = 0.0;
        double drawEnd = lengthSeconds;
        float playheadPos = width * playProportion - 2.f;
        
        if(cropped) {
            drawStart = lengthSeconds*(double)startProportion;
            drawEnd = lengthSeconds*(double)endProportion;
        }
        
        // waveform
        g.setColour(juce::Colour(225, 225, 225));
        
        // TODO: only draw wave when its changed
        waveThumbnail.drawChannels(g, getLocalBounds(), drawStart, drawEnd, (float)*volume);
        
        // play marker
        if(playheadPos > 0 && playheadPos < width) {
            g.setColour(juce::Colour(255, 0, 0));
            g.fillRect(playheadPos, 0.f, 2.f, height);
        }
        
        if(!cropped) {
            // crop left marker
            g.setColour(juce::Colour(20, 40, 255));
            g.fillRect(width * startProportion, 0.f, 2.f, height);
            
            // crop right marker
            g.setColour(juce::Colour(20, 40, 255));
            g.fillRect(width * endProportion-2.f, 0.f, 2.f, height);
        }
        
    }
    
    
    

    /*// play marker
    g.setColour(juce::Colour(255, 0, 0));
    g.fillRect(x+(width*playProportion), y, 2.f, height);
    

    
    // paint waveform*/
}
