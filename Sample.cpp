#include "Sample.h"

Sample::Sample(juce::AudioFormatManager *manager, int index)
: bpm("Sample BPM", "", 5.0, 500.0, 0.01, 120.0),
cropStart("Crop sample (start %)", "At what point to start playing the sample", 0.0, 100.0, 0.01, 0.0),
cropEnd("Crop sample (end %)", "At what point to stop playing the sample", 0.0, 100.0, 0.01, 100.0),
interval("Sample interval (beats)", "How often (in beats) the sample should play", 0.25, 16.0, 0.25, 1.0),
delay("Delay", "Introduces a delay (in beats) before the sample is played", 0.0, 10.0, 0.25, 0.0) {
    
    setOpaque(true);
    
    formatManager = manager;
    
    sampleWaveform = new WaveformView(&sampleLength, &curPos, &volume, &startPos, &endPos, formatManager);
    
    sampleIndex = index;
    
    // label (name)
    sampleLabel.setText("Unloaded", juce::dontSendNotification);
    sampleLabel.setEditable(true);
    sampleLabel.setFont(juce::Font(16.f, juce::Font::bold));
    
    // collapse
    sampleCollapseButton.setButtonText("-");
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
    sampleVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 50, 24);
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
    delete sampleWaveform;
    formatManager = nullptr;
    sampleWaveform = nullptr;
    
    
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
    std::cout << event.getNumberOfClicks() << std::endl;
    if((event.getNumberOfClicks() % 2) == 0 && event.mods.isLeftButtonDown()) {
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

bool Sample::getSoloed() {
    return isSoloed;
}

// disables the sample for its interval
void Sample::disable() {
    isDisabled = true;
}

void Sample::paint (juce::Graphics& g)
{
    std::cout << "sample paint" << std::endl;
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.setColour(juce::Colour(30, 30, 30));
    g.fillAll();
    
    
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
        
    if(isMuted) sampleMuteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(165, 40, 0));
    else sampleMuteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkslategrey);
    
    if(isSoloed) sampleSoloButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0, 40, 165));
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
    
    // sample index label
    g.setColour(juce::Colours::white);
    g.drawText(std::to_string(sampleIndex), 25, 4, 20, 18, juce::Justification::centred);
}

void Sample::resized() {
    int width = getWidth();
    int height = getHeight();
        
    // label
    sampleLabel.setBounds(50, 4, 150, 20);

    // browse
    sampleBrowseButton.setBounds(50, 25, 150, 20);
    
    // bpm
    bpm.setBounds(210, 10, 425, 40);
    
    // crop
    cropStart.setBounds(50, 50, 275, 40);
    cropEnd.setBounds(350, 50, 275, 40);
        
    // interval
    interval.setBounds(50, 90, 275, 40);
    
    // delay
    delay.setBounds(350, 90, 275, 40);
    
    // volume
    sampleVolumeSlider.setBounds(640, 4, 50, height-8);
        
    // collapse
    sampleCollapseButton.setBounds(0, 0, 18, 18);
    
    // waveform
    int waveWidth = getLocalBounds().getWidth() - 705 - 20 - 25;
    if(waveWidth > 0)
        sampleWaveform->setBounds(705, 35, waveWidth, 85);
    
    // mute & solo
    sampleMuteButton.setBounds(705, 4, waveWidth/2-5, 24);
    sampleSoloButton.setBounds(705+waveWidth/2+5, 4, waveWidth/2-5, 24);
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
        isLoaded = false;
        juce::File file = sampleChooser.getResult();

        if(loadSample(file)) {
            isLoaded = true;
            
            samplePath = file.getFullPathName().toStdString();
            setLabel(file.getFileName().toStdString());
        }
    }
}

long Sample::pow10(double input, int power) {
    for(int i = 0; i < power; i++) {
        input*=10.0;
    }
    return input;
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
    
    // get samples buffers
    const juce::AudioSourceChannelInfo sampleFill(sampleBuffer, 0, numSamples);
    inLeftBuffer = sampleFill.buffer->getReadPointer(0, 0);
    inRightBuffer = sampleFill.buffer->getReadPointer(1, 0);
    sampleLength = sampleBuffer->getNumSamples();
    
    
    startPos = cropStart.getValue() * 0.01 * sampleLength;
    endPos = cropEnd.getValue() * 0.01 * sampleLength;
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
    output += "bpm " + bpm.getStrValue() + "\n";
    output += "start " + cropStart.getStrValue() + "\n";
    output += "end " + cropEnd.getStrValue() + "\n";
    output += "interval " + interval.getStrValue() + "\n";
    output += "delay " + delay.getStrValue() + "\n";
    output += "volume " + std::to_string(volume) + "\n";
    output += "muted " + std::to_string(isMuted) + "\n";
    output += "collapsed " + std::to_string(collapsed) + "\n";
    output += "}\n";
    return output;
}
