//
//  Sample.cpp
//  Proto - App
//
//  Created by Felix Thomas on 20/02/2021.
//

#include "Sample.h"

Sample::Sample(juce::AudioFormatManager *manager) {
    formatManager = manager;
    
    // label (name)
    sampleLabel.setText("Unloaded", juce::dontSendNotification);
    
    // collapse
    sampleCollapseButton.setButtonText("+-");
    sampleCollapseButton.onClick = [this] {
        collapsed = !collapsed;
        getParentComponent()->getParentComponent()->getParentComponent()->getParentComponent()->resized(); // i hate this but it works...
    };
    
    // browse
    sampleBrowseButton.setButtonText ("Sample browser");
    sampleBrowseButton.onClick = [this] { browse(); };
    
    // bpm
    sampleBpmSlider.setRange(5, 500, 0.01);
    sampleBpmSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 75, 20);
    sampleBpmSlider.setValue(120);
    sampleBpmLabel.setText("Sample BPM", juce::dontSendNotification);
    sampleBpmLabel.setJustificationType(juce::Justification::right);
        
    // crop
    sampleCropLeftSlider.setRange(0, 100, 0.01);
    sampleCropLeftSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 30);
    sampleCropLeftLabel.setText("Crop sample (left %)", juce::dontSendNotification);
    sampleCropLeftLabel.setJustificationType(juce::Justification::right);
    
    sampleCropRightSlider.setRange(0, 100, 0.01);
    sampleCropRightSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 30);
    sampleCropRightLabel.setText("Crop sample (right %)", juce::dontSendNotification);
    sampleCropRightLabel.setJustificationType(juce::Justification::right);
    
    //interval
    sampleIntervalSlider.setRange(0.25, 16, 0.25);
    sampleIntervalSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 30);
    sampleIntervalSlider.setValue(1);
    sampleIntervalLabel.setText("Sample interval (beats)", juce::dontSendNotification);
    sampleIntervalLabel.setJustificationType(juce::Justification::right);

    // delay
    sampleDelaySlider.setRange(0, 10, 0.25);
    sampleDelaySlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 30);
    sampleDelayLabel.setText("Sample delay (beats)", juce::dontSendNotification);
    sampleDelayLabel.setJustificationType(juce::Justification::right);

    // volume
    sampleVolumeSlider.setRange(0, 100, 1);
    sampleVolumeSlider.setValue(100);
    sampleVolumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    sampleVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 30);
    sampleVolumeLabel.setText("Sample volume", juce::dontSendNotification);
    sampleVolumeLabel.setJustificationType(juce::Justification::right);
    
    addAndMakeVisible(sampleLabel);
    addAndMakeVisible(sampleCollapseButton);
    addAndMakeVisible(sampleBrowseButton);
    addAndMakeVisible(sampleBpmSlider);
    addAndMakeVisible(sampleBpmLabel);
    addAndMakeVisible(sampleIntervalSlider);
    addAndMakeVisible(sampleIntervalLabel);
    addAndMakeVisible(sampleCropLeftSlider);
    addAndMakeVisible(sampleCropLeftLabel);
    addAndMakeVisible(sampleCropRightSlider);
    addAndMakeVisible(sampleCropRightLabel);
    addAndMakeVisible(sampleDelayLabel);
    addAndMakeVisible(sampleDelaySlider);
    addAndMakeVisible(sampleVolumeLabel);
    addAndMakeVisible(sampleVolumeSlider);
}

Sample::~Sample() {
    delete sampleReader;
    delete sampleBuffer;
    formatManager = nullptr;
}

bool Sample::isCollapsed() {
    return collapsed;
}

void Sample::setCollapsed(bool c) {
    collapsed = c;
}

void Sample::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    if(!isWaiting) { // playing
        //g.fillAll (juce::Colour(50, 100, 50));
        g.setColour(juce::Colour(50, 100, 50));
        //g.fillRoundedRectangle(getX(), getY(), getWidth(), getHeight(), 20.f);
    } else { // waiting
        //g.fillAll (juce::Colour(100, 100, 100));
        g.setColour(juce::Colour(100, 100, 100));
    }
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 20.f);
}

void Sample::resized() {
    // label
    sampleLabel.setBounds(120, 0, 200, 20);
    
    // collapse
    sampleCollapseButton.setBounds(20, 0, 20, 20);
    
    // browse
    sampleBrowseButton.setBounds(50, 0, 60, 40);
    
    // bpm
    sampleBpmLabel.setBounds(120, 5, 475, 20);
    sampleBpmSlider.setBounds(120, 20, 475, 20);
    
    // crop
    sampleCropLeftLabel.setBounds(20, 45, 275, 20);
    sampleCropLeftSlider.setBounds(20, 60, 275, 20);
    
    sampleCropRightLabel.setBounds(320, 45, 275, 20);
    sampleCropRightSlider.setBounds(320, 60, 275, 20);
    
    // interval
    sampleIntervalLabel.setBounds(20, 85, 275, 20);
    sampleIntervalSlider.setBounds(20, 100, 275, 20);
    
    // delay
    sampleDelayLabel.setBounds(320, 85, 275, 20);
    sampleDelaySlider.setBounds(320, 100, 275, 20);
    
    // volume
    sampleVolumeSlider.setBounds(670, 0, 50, 120);
    
}

void Sample::setPath(std::string path) {
    samplePath = path;
    juce::File file(samplePath);
    
    sampleReader = formatManager->createReaderFor(file);

    if(sampleReader != nullptr) {
        sampleBuffer = new juce::AudioBuffer<float>(2, sampleReader->lengthInSamples);

        sampleReader->read(sampleBuffer, 0, sampleReader->lengthInSamples, 0, true, true);
        
        isLoaded = true;
    }
}

void Sample::browse() {
    juce::FileChooser sampleChooser("Select a sample", {}, formatManager->getWildcardForAllFormats());
    
    if(sampleChooser.browseForFileToOpen()) {
        juce::File file = sampleChooser.getResult();
        samplePath = file.getFullPathName().toStdString();
        setLabel(file.getFileName().toStdString());
        
        sampleReader = formatManager->createReaderFor(file);
        
        if(sampleReader != nullptr) {
            sampleBuffer = new juce::AudioBuffer<float>(2, sampleReader->lengthInSamples);

            sampleReader->read(sampleBuffer, 0, sampleReader->lengthInSamples, 0, true, true);
            
            isLoaded = true;
        }
    }
}

long Sample::pow10(float input, int power) {
    for(int i = 0; i < power; i++) {
        input*=10.f;
    }
    return (long)input;
}

void Sample::getValue(float &outLeft, float &outRight, long roundedBeat, long prevBeat) {
    if(!isLoaded) {
        return;
    }
    
    // start playing sample when curBeat is multiple of interval,
    // and if curBeat > previous, i.e. only calculate this every 1/(10^precision)th of a beat
    // note undefined behaviour (crash) if interval is 0
    //if((roundedBeat > prevBeat) && (((roundedBeat-roundedDelay) % roundedInterval) == 0)) {
    if((roundedBeat > prevBeat) && (((roundedBeat-roundedDelay) % roundedInterval) == 0)) {
        curPos = startPos;
        isWaiting = false;
        
        // repaint
        juce::MessageManager::callAsync ([this] { repaint(); });
    }
    
    if(!isWaiting) { // sample playing
        //int adjustedSampleOffset = std::round((float)sampleOffset * samplePlaybackRate);
        int sampleOffset = (int)std::round(curPos);
        
        outLeft += inLeftBuffer[sampleOffset] * volume;
        outRight += inRightBuffer[sampleOffset] * volume;
        
        curPos += playbackRate;
        if(curPos >= endPos) {
            // reached end of sample
            isWaiting = true;
            
            // repaint
            juce::MessageManager::callAsync ([this] { repaint(); });
        }
    }
}

void Sample::updateParams(float trackBpm, int precision, int numSamples) {
    // get value of sliders
    // todo: use listeners so value always correct in class
    sampleBpm = (float)sampleBpmSlider.getValue();
    
    startPos = (float)sampleCropLeftSlider.getValue() * 0.01f * sampleBuffer->getNumSamples();
    endPos = (1.f - ((float)sampleCropRightSlider.getValue() * 0.01f)) * sampleBuffer->getNumSamples();
    
    interval = (float)sampleIntervalSlider.getValue();
    delay = (float)sampleDelaySlider.getValue();
    
    volume = (float)sampleVolumeSlider.getValue() / 100.f;
    
    // calculations
    playbackRate = trackBpm / sampleBpm;
    if(trackBpm == 0.f) playbackRate = 0.f;
    roundedInterval = pow10(interval, precision);
    roundedDelay = pow10(delay, precision);
    
    // get samples buffers
    const juce::AudioSourceChannelInfo sampleFill(sampleBuffer, 0, numSamples);
    inLeftBuffer = sampleFill.buffer->getReadPointer(0, 0);
    inRightBuffer = sampleFill.buffer->getReadPointer(1, 0);
    
}

void Sample::setLabel(std::string label) {
    sampleLabel.setText(label, juce::dontSendNotification);
}

juce::String Sample::getLabel() {
    return sampleLabel.getText();
}

void Sample::setStart(double start) {
    sampleCropLeftSlider.setValue(start);
}

void Sample::setEnd(double end) {
    sampleCropRightSlider.setValue(end);
}

void Sample::setInterval(double interval) {
    sampleIntervalSlider.setValue(interval);
}

void Sample::setDelay(double delay) {
    sampleDelaySlider.setValue(delay);
}

void Sample::setVolume(double volume) {
    sampleVolumeSlider.setValue(volume);
}

void Sample::setBpm(double bpm) {
    sampleBpmSlider.setValue(bpm);
}

void Sample::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill, float trackBpm, int precision, long roundedBeat, long prevBeat, float curBeat) {
 
    // unused
    /*float decimal = 1.f / pow10(1, precision);
    
    std::cout << decimal << std::endl;
    
    // get value of sliders
    // todo: use listeners so value always correct in class
    sampleBpm = (float)sampleBpmSlider.getValue();
    
    startPos = (float)sampleCropLeftSlider.getValue() * 0.01f * sampleBuffer->getNumSamples();
    endPos = (1.f - ((float)sampleCropRightSlider.getValue() * 0.01f)) * sampleBuffer->getNumSamples();
    
    interval = (float)sampleIntervalSlider.getValue();
    delay = (float)sampleDelaySlider.getValue();
    
    
    // calculations
    playbackRate = trackBpm / sampleBpm;
    if(trackBpm == 0.f) playbackRate = 0.f;
    long roundedInterval = pow10(interval, precision);
    long roundedDelay = pow10(delay, precision);

    
    // get buffers
    float* outLeftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    float* outRightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
    
    
    // start playing sample when curBeat is multiple of interval,
    // and if curBeat > previous, i.e. only calculate this every 1/(10^precision)th of a beat
    // note undefined behaviour (crash) if interval is 0
    //if((roundedBeat > prevBeat) && (((roundedBeat-roundedDelay) % roundedInterval) == 0)) {
    if((roundedBeat > prevBeat) && (((roundedBeat-roundedDelay) % roundedInterval) == 0)) {
        curPos = startPos;
        isWaiting = false;
        
        // repaint
        juce::MessageManager::callAsync ([this] { repaint(); });
    }

    if(!isWaiting) { // sample playing
    
        for(int outOffset = 0; outOffset < bufferToFill.numSamples; outOffset++) {
            //int adjustedSampleOffset = std::round((float)sampleOffset * samplePlaybackRate);
            int sampleOffset = (int)std::round(curPos);
            
            outLeftBuffer[outOffset] += inLeftBuffer[sampleOffset];
            outRightBuffer[outOffset] += inRightBuffer[sampleOffset];
            
            curPos += playbackRate;
            if(curPos >= endPos) {
                // reached end of sample
                isWaiting = true;
                
                // repaint
                juce::MessageManager::callAsync ([this] { repaint(); });
            }
        }
    }*/
}

std::string Sample::toString() {
    std::string output = "";
    output += "Sample " + sampleLabel.getText().toStdString() + " {\n";
    output += "path " + samplePath + "\n";
    output += "bpm " + std::to_string(sampleBpmSlider.getValue()) + "\n";
    output += "start " + std::to_string(sampleCropLeftSlider.getValue()) + "\n";
    output += "end " + std::to_string(sampleCropRightSlider.getValue()) + "\n";
    output += "interval " + std::to_string(sampleIntervalSlider.getValue()) + "\n";
    output += "delay " + std::to_string(sampleDelaySlider.getValue()) + "\n";
    output += "volume " + std::to_string(sampleVolumeSlider.getValue()) + "\n";
    output += "}\n";
    return output;
}
