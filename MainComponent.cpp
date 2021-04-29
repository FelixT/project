#include "MainComponent.h"
#include <math.h>

//==============================================================================
MainComponent::MainComponent()
{
        
    bpmSlider.setRange(5, 500, 0.1);
    bpmSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    bpmSlider.setValue(120);
    bpmLabel.setText("BPM", juce::dontSendNotification);
    
    masterVolumeSlider.setRange(0, 1, 0.01);
    masterVolumeSlider.setValue(1.0);
    masterVolumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    masterVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 30);
    masterVolumeLabel.setText("Master volume", juce::dontSendNotification);
    
    freqSlider.setRange(0, 2000, 0.1);
    freqSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 20);
    freqLabel.setText("Frequency", juce::dontSendNotification);
    
    noteLengthSlider.setRange(0, 100, 0.1);
    noteLengthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 20);
    noteLengthLabel.setText("Note Length (%)", juce::dontSendNotification);
    
    curBeatLabel.setText("0", juce::dontSendNotification);
    curBeatLabel.setColour(juce::Label::backgroundColourId, findColour(juce::ResizableWindow::backgroundColourId));
    //curBeatLabel.setPaintingIsUnclipped(true);
    curBeatLabel.setOpaque(true);
    
    sampleAddButton.setButtonText("Add sample");
    sampleAddButton.onClick = [this] { addSample(); };
    
    resetBeatButton.setButtonText("Reset beat");
    resetBeatButton.onClick = [this] { curBeat = -0.1f; prevBeat = -0.2f; };
    resetBeatButton.setTooltip("Reset the current beat count to 0. This may fix some issues.");
    
    saveStateButton.setButtonText("Save state");
    saveStateButton.onClick = [this] { saveState(); };
    saveStateButton.setTooltip("Save the currently open project");
    
    loadStateButton.setButtonText("Load state");
    loadStateButton.onClick = [this] { loadState(); };
    loadStateButton.setTooltip("Load a previous project");
    
    modifierAddButton.setButtonText("Add modifier");
    modifierAddButton.onClick = [this] { addModifier(); };
    
    newProjectButton.setButtonText("New project");
    newProjectButton.onClick = [this] {
        loading = true;
        resetSamples(); resetModifiers(); curBeat = -0.1f; prevBeat = -0.2f; resized();
        loading = false;
    };
    newProjectButton.setTooltip("Discards the current project and starts a new one");

    samplesViewport.setViewedComponent(&samplesComponent, false);
    modifiersViewport.setViewedComponent(&modifiersComponent, false);
    
    addAndMakeVisible(tooltipWindow);
    addAndMakeVisible(freqSlider);
    addAndMakeVisible(freqLabel);
    addAndMakeVisible(bpmSlider);
    addAndMakeVisible(bpmLabel);
    addAndMakeVisible(masterVolumeLabel);
    addAndMakeVisible(masterVolumeSlider);
    addAndMakeVisible(noteLengthSlider);
    addAndMakeVisible(noteLengthLabel);
    addAndMakeVisible(curBeatLabel);
    addAndMakeVisible(sampleAddButton);
    addAndMakeVisible(resetBeatButton);
    addAndMakeVisible(saveStateButton);
    addAndMakeVisible(loadStateButton);
    addAndMakeVisible(modifierAddButton);
    addAndMakeVisible(newProjectButton);
    addAndMakeVisible(samplesViewport);
    addAndMakeVisible(modifiersViewport);
    
    formatManager.registerBasicFormats();
    
    setSize (800, 600);

    setAudioChannels (0, 2);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
    
}

std::string MainComponent::getState() {
    std::string output = "";
    output += "Project <name> {\n";
    output += "bpm " + std::to_string(bpmSlider.getValue()) + "\n";
    output += "}\n";
    for(int i = 0; i < samples.size(); i++) {
        Sample *sample = samples.at(i);
        output += sample->toString();
    }
    for(int i = 0; i < modifiers.size(); i++) {
        Modifier *modifier = modifiers.at(i);
        output += modifier->toString();
    }
    return output;
}

// save state to file
void MainComponent::saveState() {
    std::string output = getState();
    
    std::cout << "\n\n---- STATE ----\n" << std::endl;
    std::cout << output << std::endl;
    std::cout << "---- END STATE ----" << std::endl;
    
    juce::FileChooser chooser("Save as project file (.PR)", {}, "*.pr");
    if(chooser.browseForFileToSave(true)) {
        loading = true;
        
        juce::File jfile = chooser.getResult();
        std::string path = jfile.getFullPathName().toStdString();
        
        std::ofstream file(path, std::ofstream::out);
        file << output;
        file.close();
        
        loading = false;
    }
    
}

void MainComponent::loadState() {
    // choose file
    juce::FileChooser chooser("Select project file (.PR)", {}, "*.pr");
    
    if(chooser.browseForFileToOpen()) {
        loading = true;
        
        resetSamples();
        resetModifiers();
        
        juce::File jfile = chooser.getResult();
        
        std::string path = jfile.getFullPathName().toStdString();
        
        std::ifstream file(path);
        std::string line;
        
        int state = false;
        int lineNo = 1;
        while(std::getline(file, line)) {
            // go through line by line
            
            if(state == STATE_OUT_OF_BLOCK) {
                // split by space
                auto splitPos = line.find(' ');
                
                //std::cout << splitPos << std::endl;
                
                std::string first = line.substr(0, splitPos);
                std::string second = line.substr(splitPos+1, line.length()-splitPos-3);
                
                std::string third = line.substr(line.length()-2, 2);
                if(third != " {") { // sanity check
                    std::string err = "ERROR on line " + std::to_string(lineNo) + ": Misplaced '{'";
                    std::cout << err << std::endl; return;
                }
                
                if(first == "Project") {
                    std::cout << "PROJECT" << std::endl;
                    std::cout << "with name " << second << std::endl;
                    state = STATE_IN_PROJECT_BLOCK;
                } else if(first == "Sample") {
                    // create new sample
                    addSample();
                    samples.back()->setCollapsed(true);
                    
                    std::cout << "SAMPLE" << std::endl;
                    std::cout << "with label " << second << std::endl;
                    samples.back()->setLabel(second);
                    state = STATE_IN_SAMPLE_BLOCK;
                } else if(first == "Modifier") {
                    // create new modifier
                    addModifier();
                    
                    std::cout << "MODIFIER" << std::endl;
                    std::cout << "with label " << second << std::endl;
                    state = STATE_IN_MODIFIER_BLOCK;
                }
                
            } else { // (we're within a block)
                if(line == "}") {
                    std::cout << "END OF BLOCK" << std::endl;
                    if(state == STATE_IN_MODIFIER_BLOCK) modifiers.back()->updateEuclidean();
                    state = STATE_OUT_OF_BLOCK;
                } else {
                    // split by space
                    auto splitPos = line.find(' ');
                    
                    std::string first = line.substr(0, splitPos);
                    std::string second = line.substr(splitPos+1, line.length()-1);
                    
                    if(state == STATE_IN_PROJECT_BLOCK) {
                        std::cout << "with " << first << " '" << second << "'" << std::endl;

                        if(first == "bpm") bpmSlider.setValue(std::stod(second));
                    } else if(state == STATE_IN_SAMPLE_BLOCK) {
                        
                        if(first == "path") samples.back()->setPath(second);
                        if(first == "bpm") samples.back()->setBpm(std::stod(second));
                        if(first == "start") samples.back()->setStart(std::stod(second));
                        if(first == "end") samples.back()->setEnd(std::stod(second));
                        if(first == "interval") samples.back()->setInterval(std::stod(second));
                        if(first == "delay") samples.back()->setDelay(std::stod(second));
                        if(first == "volume") samples.back()->setVolume(std::stod(second));
                        if(first == "muted") samples.back()->setMuted(std::stod(second));
                        if(first == "collapsed") samples.back()->setCollapsed(std::stod(second));
                        
                    } else if(state == STATE_IN_MODIFIER_BLOCK) {
                        std::cout << "with " << first << " '" << second << "'" << std::endl;
                        
                        if(first == "mode") modifiers.back()->setMode(std::stoi(second));
                        if(first == "state") modifiers.back()->setState(std::stoi(second));
                        if(first == "parameter") modifiers.back()->setParameter(std::stoi(second));
                        if(first == "selected") modifiers.back()->setSelected(std::stoi(second));
                        if(first == "interval") modifiers.back()->setInterval(std::stod(second));
                        if(first == "min") modifiers.back()->setMin(std::stod(second));
                        if(first == "max") modifiers.back()->setMax(std::stod(second));
                        if(first == "step") modifiers.back()->setStep(std::stod(second));
                        if(first == "equation") modifiers.back()->setEquation(second);
                        
                    }
                    
                }
                
                lineNo++;
            }
            
        }
        
        loading = false;
    }
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    
    curSampleRate = sampleRate;
    
}

long MainComponent::pow10(float input, int power) {
    for(int i = 0; i < power; i++) {
        input*=10.f;
    }
    return (long)input;
}

void MainComponent::addSample() {
    Sample *sample = new Sample(&formatManager);
    samples.push_back(sample);
    
    samplesComponent.addAndMakeVisible(sample);
    
    resized();

}

void MainComponent::addModifier() {
    Modifier *modifier = new Modifier(&samples, &modifiers);
    modifiers.push_back(modifier);
    
    modifiersComponent.addAndMakeVisible(modifier);
    
    resized();

}

// TODO: make wave its on separate class
void MainComponent::getWaveValue(float &outLeft, float &outRight) {
    curWaveAngle+=waveAngleDelta;
    if(waveEnabled && (fmod(curBeat, 1) < waveNoteLength)) {
        float waveValue = std::sinf(curWaveAngle) * waveVolume;
        outLeft += waveValue;
        outRight += waveValue;
    }
}

void MainComponent::updateWaveParams() {
    // get values from sliders and that
    curWaveAngle = fmod(curWaveAngle, 2.f*(float)M_PI); // make sure angle stays within range 0 - 2*PI
    float waveFrequency = (float)freqSlider.getValue(); // Hz
    float wavePeriod = (float)curSampleRate / waveFrequency; // in samples
    waveAngleDelta = 2.f*(float)M_PI / wavePeriod;
    waveNoteLength = (float)noteLengthSlider.getValue()/100.f;
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    
    if(loading) return;
    
    // redraw the beat count
    juce::MessageManager::callAsync ([this] { curBeatLabel.setText(juce::String((double)roundBeat/(std::pow(10, precision))), juce::dontSendNotification); curBeatLabel.repaint(); });
    
    
    // get buffers
    float* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    float* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
        
    // BEAT/SAMPLE CALCULATIONS
    double beatFrequency = bpmSlider.getValue()/60.0; // Hz (beats/second)
    samplesPerBeat = curSampleRate / beatFrequency; // samples/beat
    beatsPerSample = 1.0 / samplesPerBeat; // i.e. delta beat added for each sample
    masterVolume = (float)masterVolumeSlider.getValue();
    
    // SINE
    updateWaveParams();
    
    // MODIFIERS
    for(int i = 0; i < modifiers.size(); i++) {
        Modifier *modifier = modifiers.at(i);
        modifier->updateParams(precision);
    }
    
    // SAMPLES
    for(int i = 0; i < samples.size(); i++) {
        Sample *sample = samples.at(i);
        if(sample->isLoaded) {
            sample->updateBuffers(bufferToFill.numSamples);
        }
    }
        
    
    for(int sampleOffset = 0; sampleOffset < bufferToFill.numSamples; sampleOffset++) {
        // todo: at some point we should reset the curBeat to 0 to prevent overflow
        // rounding becomes a problem at high bpm and with high preicision, low bpms don't increment beat count as delta is too low
        // or modulo on high ints messes things up
        prevBeat = roundBeat;
        roundBeat = pow10(curBeat, precision);
        
        float outLeft = 0.f;
        float outRight = 0.f;
        
        // perform any modifier actions
        for(int i = 0; i < modifiers.size(); i++) {
            Modifier *modifier = modifiers.at(i);
            modifier->tick(roundBeat, prevBeat);
        }
        
        // wave
        getWaveValue(outLeft, outRight);
        
        // samples
        for(int i = 0; i < samples.size(); i++) {
            Sample *sample = samples.at(i);
            sample->updateParams(bpmSlider.getValue(), curSampleRate, precision);
            sample->getValue(outLeft, outRight, roundBeat, prevBeat);
        }
        

        leftBuffer[sampleOffset] = outLeft*masterVolume;
        rightBuffer[sampleOffset] = outRight*masterVolume;
        
        curBeat+=beatsPerSample;
        
    }
}

void MainComponent::resetSamples() {
    for(int i = 0; i < samples.size(); i++) {
        delete samples[i];
    }
    samples.clear();
}

void MainComponent::resetModifiers() {
    for(int i = 0; i < modifiers.size(); i++) {
        delete modifiers[i];
    }
    modifiers.clear();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    resetSamples();
    resetModifiers();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
     
    int width = getWidth();
    int avaliableHeight = getHeight() - 150;
    int samplesHeight = avaliableHeight/2 - 25;
    int modifiersHeight = avaliableHeight/2 - 25;
    
    if(samplesHeight >= 20) {
        g.setColour(juce::Colour(30, 30, 30));
        g.fillRect(0, 150, width, samplesHeight); // fill samples area
        g.setColour(juce::Colour(0, 0, 0));
        g.drawRect(0, 150, width, samplesHeight);
        if(samples.size() == 0) {
            g.setFont(20);
            g.drawText("< Samples >", 0, 151, width, samplesHeight-1, juce::Justification::centred);

            g.setColour(juce::Colour(255, 255, 255));
            g.drawText("< Samples >", 0, 150, width, samplesHeight, juce::Justification::centred);
        }
    }
    
    if(modifiersHeight >= 20) {
        g.setColour(juce::Colour(30, 30, 30));
        g.fillRect(0, 150+samplesHeight+25, width, modifiersHeight); // fill modifiers area
        g.setColour(juce::Colour(0, 0, 0));
        g.drawRect(0, 150+samplesHeight+25, width, modifiersHeight);
        if(modifiers.size() == 0) {
            g.setFont(20);
            g.drawText("< Modifiers >", 0, 150+samplesHeight+26, width, modifiersHeight - 1, juce::Justification::centred);

            g.setColour(juce::Colour(255, 255, 255));
            g.drawText("< Modifiers >", 0, 150+samplesHeight+25, width, modifiersHeight, juce::Justification::centred);
        }
    }

}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    
    int width = getWidth();
    
    // == top parameters ==
    // inhabits y: 0-125
    
    bpmLabel.setBounds(10, 10, 100, 20);
    bpmSlider.setBounds(10, 30, 500, 20);

    masterVolumeLabel.setBounds(600, 10, 50, 40);
    masterVolumeSlider.setBounds(600, 45, 50, 85);
    
    freqLabel.setBounds(10, 50, 100, 20);
    freqSlider.setBounds(10, 70, 250, 20);
    
    noteLengthLabel.setBounds(275, 50, 100, 20);
    noteLengthSlider.setBounds(275, 70, 250, 20);
    
    curBeatLabel.setBounds(10, 95, 100, 20);
    curBeatLabel.setTooltip("Current beat count");
        
    sampleAddButton.setBounds(10, 120, 60, 20);
    resetBeatButton.setBounds(80, 120, 60, 20);
    saveStateButton.setBounds(150, 120, 60, 20);
    loadStateButton.setBounds(220, 120, 60, 20);
    modifierAddButton.setBounds(290, 120, 60, 20);
    newProjectButton.setBounds(360, 120, 60, 20);
    
    int avaliableHeight = getHeight() - 150;
    
    
    // == samples ==
    
    int samplesHeight = avaliableHeight/2 - 25;
    
    samplesViewport.setBounds(0, 150, width, samplesHeight); 
    
    int relativeY = 0;
    int margin = 3; // (bottom)

    for(int i = 0; i < samples.size(); i++) {
        if(samples[i]->isCollapsed()) {
            int height = 40;
            samples[i]->setBounds(margin, relativeY + margin, width - 2*margin, height - 2*margin);
            relativeY+=height;
            
        } else {
            int height = 140;
            samples[i]->setBounds(margin, relativeY + margin, width - 2*margin, height- 2*margin);
            relativeY+=height;
        }
    }
    
    samplesComponent.setBounds(0, 0, width, relativeY);
    
    // == modifiers ==
    
    int modifiersHeight = avaliableHeight/2 - 25;
    
    modifiersViewport.setBounds(0, 150 + samplesHeight + 25, width, modifiersHeight);
    
    relativeY = 0;
    
    for(int i = 0; i < modifiers.size(); i++) {
        int height = 100;
        modifiers[i]->setBounds(margin, relativeY + margin, width - 2*margin, height - 2*margin);
        relativeY+=height;
    }

    modifiersComponent.setBounds(0, 0, width, relativeY);
    
}

