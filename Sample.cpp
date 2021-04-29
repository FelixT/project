#include "Sample.h"

Sample::Sample(juce::AudioFormatManager *manager)
: bpm("Sample BPM", "", 5.0, 500.0, 0.01, 120.0),
cropStart("Crop sample (start %)", "At what point to start playing the sample", 0.0, 100.0, 0.01, 0.0),
cropEnd("Crop sample (end %)", "At what point to stop playing the sample", 0.0, 100.0, 0.01, 0.0),
interval("Sample interval (beats)", "How often (in beats) the sample should play", 0.25, 16.0, 0.25, 1.0),
delay("Delay", "Introduces a delay (in beats) before the sample is played", 0.0, 10.0, 0.25, 0.0) {
    
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

    // volume
    sampleVolumeSlider.setRange(0, 100, 1);
    sampleVolumeSlider.setValue(volume*100.0);
    sampleVolumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    sampleVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 30);
    sampleVolumeSlider.onValueChange = [this] { volume = sampleVolumeSlider.getValue() / 100.0; };
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
    addAndMakeVisible(sampleVolumeSlider);
    addAndMakeVisible(sampleMuteButton);
    addAndMakeVisible(sampleSoloButton);
    addAndMakeVisible(sampleWaveform);
    
    // params
    addAndMakeVisible(bpm);
    addAndMakeVisible(cropStart);
    addAndMakeVisible(cropEnd);
    addAndMakeVisible(interval);
    addAndMakeVisible(delay);
}

Sample::~Sample() {
    delete sampleReader;
    delete sampleBuffer;
    formatManager = nullptr;
}

std::vector<Parameter*> Sample::getParams() {
    std::vector<Parameter*> params;
    params.push_back(&bpm);
    params.push_back(&cropStart);
    params.push_back(&cropEnd);
    params.push_back(&interval);
    params.push_back(&delay);
    return params;
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
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    if(!isWaiting) { // playing
        g.setColour(juce::Colour(50, 100, 50));
    } else { // waiting
        g.setColour(juce::Colour(80, 80, 80));
    }
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.f);
    g.setColour(juce::Colour(150, 150, 150));
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 10.f, 1.f);
    
    // make sliders reflect true values
    if(slidersChanged) {
        bpm.update();
        interval.update();
        delay.update();
        cropStart.update();
        cropEnd.update();
        
        sampleVolumeSlider.setValue(volume*100.0);
        
        slidersChanged = false;
    }
        
    if(isMuted) sampleMuteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkred);
    else sampleMuteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkslategrey);
    
    if(isSoloed) sampleSoloButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::blue);
    else sampleSoloButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkslategrey);
    
    if(collapsed) {
        bpm.setVisible(false);
        sampleBrowseButton.setVisible(false);
        sampleVolumeSlider.setVisible(false);
    } else {
        bpm.setVisible(true);
        sampleBrowseButton.setVisible(true);
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
    bpm.setBounds(120, 10, 475, 40);
    
    // crop
    cropStart.setBounds(20, 50, 275, 40);
    cropEnd.setBounds(320, 50, 275, 40);
        
    // interval
    interval.setBounds(20, 90, 275, 40);
    
    // delay
    delay.setBounds(320, 90, 275, 40);
    
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

void Sample::updateParams(double trackBpm, double trackSampleRate, int precision) {
    // calculations
    playbackRate = trackBpm / bpm.getValue();
    
    playbackRate *= (sampleRate / trackSampleRate); // same rate adjust
    if(trackBpm == 0.0) playbackRate = 0.0;
    roundedInterval = pow10(interval.getValue(), precision);
    roundedDelay = pow10(delay.getValue(), precision);
}

void Sample::updateBuffers(int numSamples) {
    if(slidersChanged) juce::MessageManager::callAsync ([this] { repaint(); });
    juce::MessageManager::callAsync ([this] { sampleWaveform->repaint(); }); // redraw waveform while we're at it
    
    // get samples buffers
    const juce::AudioSourceChannelInfo sampleFill(sampleBuffer, 0, numSamples);
    inLeftBuffer = sampleFill.buffer->getReadPointer(0, 0);
    inRightBuffer = sampleFill.buffer->getReadPointer(1, 0);
    sampleLength = sampleBuffer->getNumSamples();
    
    
    startPos = cropStart.getValue() * 0.01 * sampleLength;
    endPos = (1.0 - (cropEnd.getValue() * 0.01)) * sampleLength;
}

void Sample::setLabel(std::string label) {
    sampleLabel.setText(label, juce::dontSendNotification);
}

juce::String Sample::getLabel() {
    return sampleLabel.getText();
}

void Sample::setStart(double start) {
    cropStart.setValue(start);
    slidersChanged = true;
}

void Sample::setEnd(double end) {
    cropEnd.setValue(end);
    slidersChanged = true;
}

void Sample::setInterval(double val) {
    interval.setValue(val);
    slidersChanged = true;
}

void Sample::setDelay(double val) {
    delay.setValue(val);
    slidersChanged = true;
}

void Sample::setVolume(double val) {
    volume = val;
    slidersChanged = true;
}

double Sample::getDelay() {
    return delay.getValue();
}

void Sample::setBpm(double val) {
    bpm.setValue(val);
    slidersChanged = true;
}

// TODO: parameter get value as string to remove nastyness
std::string Sample::toString() {
    std::string output = "";
    output += "Sample " + sampleLabel.getText().toStdString() + " {\n";
    output += "path " + samplePath + "\n";
    output += "bpm " + std::to_string(bpm.getValue()) + "\n";
    output += "start " + std::to_string(cropStart.getValue()) + "\n";
    output += "end " + std::to_string(cropEnd.getValue()) + "\n";
    output += "interval " + std::to_string(interval.getValue()) + "\n";
    output += "delay " + std::to_string(delay.getValue()) + "\n";
    output += "volume " + std::to_string(volume) + "\n";
    output += "muted " + std::to_string(isMuted) + "\n";
    output += "collapsed " + std::to_string(collapsed) + "\n";
    output += "}\n";
    return output;
}
