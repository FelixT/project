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
    sampleBpmSlider.setValue(sampleBpm);
    sampleBpmSlider.onValueChange = [this] { getParams(); };
    sampleBpmLabel.setText("Sample BPM", juce::dontSendNotification);
    sampleBpmLabel.setJustificationType(juce::Justification::right);
        
    // crop
    sampleCropLeftSlider.setRange(cropLeft, 100, 0.01);
    sampleCropLeftSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 30);
    sampleCropLeftSlider.onValueChange = [this] { getParams(); };
    sampleCropLeftLabel.setText("Crop sample (left %)", juce::dontSendNotification);
    sampleCropLeftLabel.setJustificationType(juce::Justification::right);
    
    sampleCropRightSlider.setRange(cropRight, 100, 0.01);
    sampleCropRightSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 30);
    sampleCropRightSlider.onValueChange = [this] { getParams(); };
    sampleCropRightLabel.setText("Crop sample (right %)", juce::dontSendNotification);
    sampleCropRightLabel.setJustificationType(juce::Justification::right);
    
    //interval
    sampleIntervalSlider.setRange(0.25, 16, 0.25);
    sampleIntervalSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 30);
    sampleIntervalSlider.setValue(interval);
    sampleIntervalSlider.onValueChange = [this] { getParams(); };
    sampleIntervalLabel.setText("Sample interval (beats)", juce::dontSendNotification);
    sampleIntervalLabel.setJustificationType(juce::Justification::right);

    // delay
    sampleDelaySlider.setRange(0, 10, 0.25);
    sampleDelaySlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 30);
    sampleDelaySlider.onValueChange = [this] { getParams(); };
    sampleDelaySlider.setValue(delay);
    sampleDelayLabel.setText("Sample delay (beats)", juce::dontSendNotification);
    sampleDelayLabel.setJustificationType(juce::Justification::right);

    // volume
    sampleVolumeSlider.setRange(0, 100, 1);
    sampleVolumeSlider.setValue(volume*100.0);
    sampleVolumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    sampleVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 30);
    sampleVolumeSlider.onValueChange = [this] { getParams(); };
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

// disables the sample for its interval
void Sample::disable() {
    isDisabled = true;
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
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.f);
    
    // make sliders reflect true values
    sampleBpmSlider.setValue(sampleBpm);
    sampleVolumeSlider.setValue(volume*100.0);
    sampleIntervalSlider.setValue(interval);
    sampleDelaySlider.setValue(delay);
    sampleCropLeftSlider.setValue(cropLeft);
    sampleCropRightSlider.setValue(cropRight);
    slidersChanged = false;
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

bool Sample::loadSample(juce::File file) {
    sampleReader = formatManager->createReaderFor(file);

    if(sampleReader != nullptr) {
        sampleBuffer = new juce::AudioBuffer<float>(2, sampleReader->lengthInSamples);

        sampleReader->read(sampleBuffer, 0, sampleReader->lengthInSamples, 0, true, true);
        
        isLoaded = true;
        
        return true;
    } else {
        
        if(samplePath != "") {
            const std::string err = "Couldn't open file with path '" + samplePath + "'";
            
            juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "ERROR", err);
        }
        
        return false;
    }
}

void Sample::setPath(std::string path) {
    samplePath = path;
    juce::File file(samplePath);
    
    loadSample(file);
}

void Sample::browse() {
    juce::FileChooser sampleChooser("Select a sample", {}, formatManager->getWildcardForAllFormats());
    
    if(sampleChooser.browseForFileToOpen()) {
        juce::File file = sampleChooser.getResult();

        if(loadSample(file)) {
            isLoaded = true;
            
            samplePath = file.getFullPathName().toStdString();
            setLabel(file.getFileName().toStdString());
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
    if(isDisabled) {
        // if disabled, check if we can reenable
        if((roundedBeat > prevBeat) && (((roundedBeat-roundedDelay) % roundedInterval) == 0)) {
            isDisabled = false;
        }
    } else {
        if((roundedBeat > prevBeat) && (((roundedBeat-roundedDelay) % roundedInterval) == 0)) {
            curPos = startPos;
            isWaiting = false;
            
            // repaint
            juce::MessageManager::callAsync ([this] { repaint(); });
        }
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

void Sample::getParams() {
    sampleBpm = sampleBpmSlider.getValue();
    interval = sampleIntervalSlider.getValue();
    delay = sampleDelaySlider.getValue();
    volume = sampleVolumeSlider.getValue() / 100.0;
    cropLeft = sampleCropLeftSlider.getValue();
    cropRight = sampleCropRightSlider.getValue();
}

void Sample::updateParams(float trackBpm, int precision) {
    // calculations
    playbackRate = trackBpm / sampleBpm;
    if(trackBpm == 0.f) playbackRate = 0.f;
    roundedInterval = pow10(interval, precision);
    roundedDelay = pow10(delay, precision);
}

void Sample::updateBuffers(int numSamples) {
    if(slidersChanged) juce::MessageManager::callAsync ([this] { repaint(); });
    
    // get samples buffers
    const juce::AudioSourceChannelInfo sampleFill(sampleBuffer, 0, numSamples);
    inLeftBuffer = sampleFill.buffer->getReadPointer(0, 0);
    inRightBuffer = sampleFill.buffer->getReadPointer(1, 0);
    
    startPos = (float)cropLeft * 0.01f * sampleBuffer->getNumSamples();
    endPos = (1.f - ((float)cropRight * 0.01f)) * sampleBuffer->getNumSamples();
}

void Sample::setLabel(std::string label) {
    sampleLabel.setText(label, juce::dontSendNotification);
}

juce::String Sample::getLabel() {
    return sampleLabel.getText();
}

void Sample::setStart(double start) {
    //sampleCropLeftSlider.setValue(start);
    cropLeft = start;
    slidersChanged = true;
}

void Sample::setEnd(double end) {
    cropRight = end;
    //sampleCropRightSlider.setValue(end);
    slidersChanged = true;
}

void Sample::setInterval(double val) {
    if(val <= 0) return;
    interval = val;
    //roundedInterval = pow10(interval, 2);
    slidersChanged = true;
}

void Sample::setDelay(double val) {
    delay = val;
    slidersChanged = true;
}

double Sample::getDelay() {
    return delay;
}

void Sample::setVolume(double val) {
    volume = val;
    slidersChanged = true;
}

void Sample::setBpm(double val) {
    sampleBpm = val;
    slidersChanged = true;
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
