//
//  Sample.cpp
//  Proto - App
//
//  Created by Felix Thomas on 20/02/2021.
//

#include "Sample.h"

Sample::Sample(juce::AudioFormatManager *manager) {
    
    formatManager = manager;
    
    sampleWaveform = new WaveformView(&sampleLength, &curPos, &volume, &startPos, &endPos, formatManager);
    
    // label (name)
    sampleLabel.setText("Unloaded", juce::dontSendNotification);
    sampleLabel.setEditable(true);
    sampleLabel.setFont(juce::Font(16.f, juce::Font::bold));
    
    // collapse
    sampleCollapseButton.setButtonText("+-");
    sampleCollapseButton.onClick = [this] {
        collapsed = !collapsed;
        getParentComponent()->getParentComponent()->getParentComponent()->getParentComponent()->resized(); // i hate this but it works...
    };
    sampleCollapseButton.setTooltip("Minimise/maximise sample");
    
    // browse
    sampleBrowseButton.setButtonText ("Sample browser");
    sampleBrowseButton.onClick = [this] { browse(); };
    sampleBrowseButton.setTooltip("Browse your computer to load a new sample");
    
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
    sampleCropLeftLabel.setText("Crop sample (start %)", juce::dontSendNotification);
    sampleCropLeftLabel.setJustificationType(juce::Justification::right);
    sampleCropLeftLabel.setTooltip("At what point to start playing the sample");
    
    sampleCropRightSlider.setRange(cropRight, 100, 0.01);
    sampleCropRightSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 30);
    sampleCropRightSlider.onValueChange = [this] { getParams(); };
    sampleCropRightLabel.setText("Crop sample (end %)", juce::dontSendNotification);
    sampleCropRightLabel.setJustificationType(juce::Justification::right);
    sampleCropRightLabel.setTooltip("At what point to stop playing the sample");
    
    //interval
    sampleIntervalSlider.setRange(0.25, 16, 0.25);
    sampleIntervalSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 30);
    sampleIntervalSlider.setValue(interval);
    sampleIntervalSlider.onValueChange = [this] { getParams(); };
    sampleIntervalLabel.setText("Sample interval (beats)", juce::dontSendNotification);
    sampleIntervalLabel.setJustificationType(juce::Justification::right);
    sampleIntervalLabel.setTooltip("How often (in beats) the sample should play");

    // delay
    sampleDelaySlider.setRange(0, 10, 0.25);
    sampleDelaySlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 30);
    sampleDelaySlider.onValueChange = [this] { getParams(); };
    sampleDelaySlider.setValue(delay);
    sampleDelayLabel.setText("Sample delay (beats)", juce::dontSendNotification);
    sampleDelayLabel.setJustificationType(juce::Justification::right);
    sampleDelayLabel.setTooltip("Introduces a delay (in beats) before the sample is played");

    // volume
    sampleVolumeSlider.setRange(0, 100, 1);
    sampleVolumeSlider.setValue(volume*100.0);
    sampleVolumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    sampleVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 30);
    sampleVolumeSlider.onValueChange = [this] { getParams(); };
    sampleVolumeSlider.setTooltip("Change the sample's playback volume");
    
    // mute & solo
    sampleSoloButton.setButtonText("S");
    sampleSoloButton.setTooltip("Solo sample");
    sampleSoloButton.onClick = [this] {
        isSoloed = !isSoloed;
    };
    sampleMuteButton.setButtonText("M");
    sampleMuteButton.setTooltip("Mute sample");
    sampleMuteButton.onClick = [this] {
        isMuted = !isMuted;
    };
    
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
    addAndMakeVisible(sampleVolumeSlider);
    addAndMakeVisible(sampleMuteButton);
    addAndMakeVisible(sampleSoloButton);
    addAndMakeVisible(sampleWaveform);
}

Sample::~Sample() {
    delete sampleReader;
    delete sampleBuffer;
    formatManager = nullptr;
}

void Sample::mouseDown(const juce::MouseEvent &event) {
    if(event.getNumberOfClicks() == 2) {
        collapsed = !collapsed;
        getParentComponent()->getParentComponent()->getParentComponent()->getParentComponent()->resized(); // i hate this but it works...
    }
}

bool Sample::isCollapsed() {
    return collapsed;
}

void Sample::setCollapsed(bool c) {
    collapsed = c;
}

void Sample::setMuted(bool m) {
    isMuted = m;
}

// disables the sample for its interval
void Sample::disable() {
    isDisabled = true;
}

void Sample::paint (juce::Graphics& g)
{
    std::cout << "Painting sample" << std::endl;
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    if(!isWaiting) { // playing
        //g.fillAll (juce::Colour(50, 100, 50));
        g.setColour(juce::Colour(50, 100, 50));
        //g.fillRoundedRectangle(getX(), getY(), getWidth(), getHeight(), 20.f);
    } else { // waiting
        //g.fillAll (juce::Colour(100, 100, 100));
        g.setColour(juce::Colour(80, 80, 80));
    }
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.f);
    g.setColour(juce::Colour(150, 150, 150));
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 10.f, 1.f);
    
    // make sliders reflect true values
    sampleBpmSlider.setValue(sampleBpm);
    sampleVolumeSlider.setValue(volume*100.0);
    sampleIntervalSlider.setValue(interval);
    sampleDelaySlider.setValue(delay);
    sampleCropLeftSlider.setValue(cropLeft);
    sampleCropRightSlider.setValue(cropRight);
    slidersChanged = false;
    
    if(isMuted) sampleMuteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkred);
    else sampleMuteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkslategrey);
    
    if(isSoloed) sampleSoloButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::blue);
    else sampleSoloButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkslategrey);
    
    if(collapsed) {
        sampleBrowseButton.setVisible(false);
        sampleBpmSlider.setVisible(false);
        sampleBpmLabel.setVisible(false);
        sampleVolumeSlider.setVisible(false);
    } else {
        sampleBrowseButton.setVisible(true);
        sampleBpmSlider.setVisible(true);
        sampleBpmLabel.setVisible(true);
        sampleVolumeSlider.setVisible(true);
    }
}

void Sample::resized() {
    // label
    sampleLabel.setBounds(120, 5, 200, 20);
    
    // collapse
    sampleCollapseButton.setBounds(20, 5, 20, 20);
    
    // browse
    sampleBrowseButton.setBounds(50, 5, 60, 40);
    
    // bpm
    sampleBpmLabel.setBounds(320, 10, 275, 20);
    sampleBpmSlider.setBounds(120, 25, 475, 20);
    
    // crop
    sampleCropLeftLabel.setBounds(20, 50, 275, 20);
    sampleCropLeftSlider.setBounds(20, 65, 275, 20);
    
    sampleCropRightLabel.setBounds(320, 50, 275, 20);
    sampleCropRightSlider.setBounds(320, 65, 275, 20);
    
    // interval
    sampleIntervalLabel.setBounds(20, 90, 275, 20);
    sampleIntervalSlider.setBounds(20, 105, 275, 20);
    
    // delay
    sampleDelayLabel.setBounds(320, 90, 275, 20);
    sampleDelaySlider.setBounds(320, 105, 275, 20);
    
    // volume
    sampleVolumeSlider.setBounds(610, 30, 50, 90);
    
    // mute & solo
    sampleMuteButton.setBounds(610, 5, 25, 25);
    sampleSoloButton.setBounds(640, 5, 25, 25);
    
    // waveform
    int waveWidth = getLocalBounds().getWidth() - 675 - 15;
    if(waveWidth > 0)
        sampleWaveform->setBounds(675, 35, waveWidth, 75);
}

bool Sample::loadSample(juce::File file) {
    sampleReader = formatManager->createReaderFor(file);

    if(sampleReader != nullptr) {
        sampleBuffer = new juce::AudioBuffer<float>(2, sampleReader->lengthInSamples);
        sampleRate = sampleReader->sampleRate;

        sampleReader->read(sampleBuffer, 0, sampleReader->lengthInSamples, 0, true, true);
        
        sampleWaveform->setSource(new juce::FileInputSource(file));
        
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
        
        if(!isMuted) {
            outLeft += inLeftBuffer[sampleOffset] * volume;
            outRight += inRightBuffer[sampleOffset] * volume;
        }
        
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

void Sample::updateParams(double trackBpm, double trackSampleRate, int precision) {
    // calculations
    playbackRate = trackBpm / sampleBpm;
    playbackRate *= (sampleRate / trackSampleRate); // same rate adjust
    if(trackBpm == 0.0) playbackRate = 0.0;
    roundedInterval = pow10(interval, precision);
    roundedDelay = pow10(delay, precision);
}

void Sample::updateBuffers(int numSamples) {
    if(slidersChanged) juce::MessageManager::callAsync ([this] { repaint(); });
    
    // get samples buffers
    const juce::AudioSourceChannelInfo sampleFill(sampleBuffer, 0, numSamples);
    inLeftBuffer = sampleFill.buffer->getReadPointer(0, 0);
    inRightBuffer = sampleFill.buffer->getReadPointer(1, 0);
    sampleLength = sampleBuffer->getNumSamples();
    juce::MessageManager::callAsync ([this] { sampleWaveform->repaint(); }); // redraw waveform while we're at it
    
    startPos = cropLeft * 0.01 * sampleLength;
    endPos = (1.0 - (cropRight * 0.01)) * sampleLength;
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
    output += "bpm " + std::to_string(sampleBpm) + "\n";
    output += "start " + std::to_string(cropLeft) + "\n";
    output += "end " + std::to_string(cropRight) + "\n";
    output += "interval " + std::to_string(interval) + "\n";
    output += "delay " + std::to_string(delay) + "\n";
    output += "volume " + std::to_string(volume) + "\n";
    output += "muted " + std::to_string(isMuted) + "\n";
    output += "collapsed " + std::to_string(collapsed) + "\n";
    output += "}\n";
    return output;
}
