#include "MainComponent.h"
#include <math.h>

//==============================================================================
MainComponent::MainComponent()
: bpm("Project BPM", "Overall beats per minute for the session", 5.0, 500.0, 0.1, 120.0),
sineFrequency("Sine frequency", "Frequency for sine wave generator", 0.0, 2000.0, 1.0, 0.0),
sineNoteLength("Sine note length (%)", "Proportion of each beat to play the sine wave for. Set to 0 to disable sine.", 0.0, 100.0, 1.0, 0.0)
{
    startTimer(25);
    
    masterVolumeSlider.setRange(0, 1, 0.01);
    masterVolumeSlider.setValue(1.0);
    masterVolumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    masterVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 75, 17);
    masterVolumeLabel.setText("Master volume:", juce::dontSendNotification);
    masterVolumeLabel.setJustificationType(juce::Justification::centred);
    
    curBeatLabel.setText("0.00", juce::dontSendNotification);
    curBeatLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0,0,0));
    curBeatLabel.setJustificationType(juce::Justification::centred);
    curBeatLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.f, juce::Font::bold));
    curBeatLabel.setOpaque(true);
    curBeatLabel.setTooltip("Current beat count: click this to reset it to 0.");
    
    playPauseButton.setButtonText("||");
    playPauseButton.onClick = [this] {
        paused = !paused;
        
        if(paused) playPauseButton.setButtonText(">");
        else playPauseButton.setButtonText("||");
    };
    
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
        bpm.setValue(120.0); bpm.update();
        resetSamples(); resetModifiers(); curBeat = -0.1f; prevBeat = -0.2f; resized();
        loading = false;
    };
    newProjectButton.setTooltip("Discards the current project and starts a new one");

    samplesViewport.setViewedComponent(&samplesComponent, false);
    modifiersViewport.setViewedComponent(&modifiersComponent, false);
    
    addAndMakeVisible(tooltipWindow);
    addAndMakeVisible(masterVolumeLabel);
    addAndMakeVisible(masterVolumeSlider);
    addAndMakeVisible(curBeatLabel);
    addAndMakeVisible(playPauseButton);
    addAndMakeVisible(sampleAddButton);
    addAndMakeVisible(resetBeatButton);
    addAndMakeVisible(saveStateButton);
    addAndMakeVisible(loadStateButton);
    addAndMakeVisible(modifierAddButton);
    addAndMakeVisible(newProjectButton);
    addAndMakeVisible(samplesViewport);
    addAndMakeVisible(modifiersViewport);
    
    addAndMakeVisible(bpm);
    addAndMakeVisible(sineFrequency);
    addAndMakeVisible(sineNoteLength);
    
    formatManager.registerBasicFormats();
    
    setSize (880, 660);

    setAudioChannels (0, 2);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
    
    resetSamples(); resetModifiers();
}

std::string MainComponent::getState() {
    std::string output = "";
    output += "Project <name> {\n";
    output += "bpm " + std::to_string(bpm.getValue()) + "\n";
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

                        if(first == "bpm") { bpm.setValue(std::stod(second)); bpm.update(); }
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
                        if(first == "selected") modifiers.back()->setSelected(std::stoi(second));
                        if(first == "parameter") modifiers.back()->setParameter(std::stoi(second));
                        if(first == "interval") modifiers.back()->setInterval(std::stod(second));
                        if(first == "min") modifiers.back()->setMin(std::stod(second));
                        if(first == "max") modifiers.back()->setMax(std::stod(second));
                        if(first == "step")
                            modifiers.back()->setStep(std::stod(second));
                        if(first == "cyclelength") modifiers.back()->setCycleLength(std::stod(second));
                        if(first == "pulseduration") modifiers.back()->setPulseDuration(std::stod(second));
                        if(first == "euclideanhits") modifiers.back()->setPulseDuration(std::stod(second));
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

long MainComponent::pow10(double input, int power) {
    for(int i = 0; i < power; i++) {
        input*=10.0;
    }
    return input;
}

void MainComponent::addSample() {
    Sample *sample = new Sample(&formatManager, samples.size());
    samples.push_back(sample);
    
    samplesComponent.addAndMakeVisible(sample);
    
    resized();
    
    // scroll viewport to bottom
    samplesViewport.setViewPosition(samplesViewport.getViewPositionX(), samplesComponent.getHeight());
}

void MainComponent::addModifier() {
    Modifier *modifier = new Modifier(&samples, &modifiers, modifiers.size());
    modifiers.push_back(modifier);
    
    modifiersComponent.addAndMakeVisible(modifier);
    
    resized();
    
    // scroll viewport to bottom
    modifiersViewport.setViewPosition(modifiersViewport.getViewPositionX(), modifiersComponent.getHeight());
}

// TODO: make wave its on separate class
void MainComponent::getWaveValue(float &outLeft, float &outRight) {
    curWaveAngle+=waveAngleDelta;
    if(waveEnabled && (fmod(curBeat, 1) < sineNoteLength.getValue()/100.0)) {
        float waveValue = std::sinf(curWaveAngle) * waveVolume;
        outLeft += waveValue;
        outRight += waveValue;
    }
}

void MainComponent::updateWaveParams() {
    // get values from sliders and that
    curWaveAngle = fmod(curWaveAngle, 2.0*M_PI); // make sure angle stays within range 0 - 2*PI
    
    double wavePeriod = curSampleRate / sineFrequency.getValue(); // in samples
    waveAngleDelta = 2.0*M_PI / wavePeriod;
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    
    //if(paused && samples.size() > 0) juce::MessageManager::callAsync ([this] { samples.at(0)->repaint(); });
    
    if(loading) return;
    
    if(paused) {
        // we do this so the sample's waveform can update even while the track is paused
        for(int i = 0; i < samples.size(); i++) {
            Sample *sample = samples.at(i);
            if(sample->isLoaded) {
                sample->updateBuffers(bufferToFill.numSamples);
            }
        }
        
        // TODO: update modifiers pattern display while paused
        
        return;
    }
    
    // get buffers
    float* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    float* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
        
    // BEAT/SAMPLE CALCULATIONS
    double beatFrequency = bpm.getValue()/60.0; // Hz (beats/second)
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
        
        bool anySoloed = false;
        float soloLeft = 0.f;
        float soloRight = 0.f;
        
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
            sample->updateParams(bpm.getValue(), curSampleRate, precision);
            if(sample->getSoloed()) {
                sample->getValue(soloLeft, soloRight, roundBeat, prevBeat);
                anySoloed = true;
            } else {
                sample->getValue(outLeft, outRight, roundBeat, prevBeat);
            }
        }
        
        if(anySoloed) {
            outLeft = soloLeft;
            outRight = soloRight;
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
    
    std::cout << "all paint" << std::endl;
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
     
    int width = getWidth();
    
    int avaliableHeight = getHeight() - controlsHeight;
    int samplesHeight = avaliableHeight/2 - viewsMargin;
    int modifiersHeight = avaliableHeight/2 - viewsMargin;
    
    if(samplesHeight >= 20) {
        g.setColour(juce::Colour(30, 30, 30));
        g.fillRect(0, controlsHeight, width, samplesHeight); // fill samples area
        g.setColour(juce::Colour(0, 0, 0));
        g.drawRect(0, controlsHeight, width, samplesHeight);
        if(samples.size() == 0) {
            g.setFont(20);
            g.drawText("< Samples >", 0, controlsHeight+1, width, samplesHeight-1, juce::Justification::centred);

            g.setColour(juce::Colour(255, 255, 255));
            g.drawText("< Samples >", 0, controlsHeight, width, samplesHeight, juce::Justification::centred);
        }
    }
    
    if(modifiersHeight >= 20) {
        g.setColour(juce::Colour(30, 30, 30));
        g.fillRect(0, controlsHeight+samplesHeight+viewsMargin, width, modifiersHeight); // fill modifiers area
        g.setColour(juce::Colour(0, 0, 0));
        g.drawRect(0, controlsHeight+samplesHeight+viewsMargin, width, modifiersHeight);
        if(modifiers.size() == 0) {
            g.setFont(20);
            g.drawText("< Modifiers >", 0, controlsHeight+samplesHeight+viewsMargin+1, width, modifiersHeight - 1, juce::Justification::centred);

            g.setColour(juce::Colour(255, 255, 255));
            g.drawText("< Modifiers >", 0, controlsHeight+samplesHeight+viewsMargin, width, modifiersHeight, juce::Justification::centred);
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
    
    masterVolumeLabel.setBounds(20, 18, 45, 30);
    //masterVolumeLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0,0,0));
    masterVolumeLabel.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 11.f, juce::Font::plain));
    masterVolumeSlider.setBounds(20, 20, 75, 75);
    
    int controlswidth = std::min(700, width-110); // max width of controls
    
    bpm.setBounds(120, 10, controlswidth, 40);
    sineFrequency.setBounds(120, 50, controlswidth/2 - 10, 40);
    sineNoteLength.setBounds(120 + controlswidth/2 + 10, 50, controlswidth/2 - 10, 40);
    
    curBeatLabel.setBounds(0, 0, 80, 20);
    curBeatLabel.setEditable(true);
    curBeatLabel.onEditorShow = [this] {
        curBeat = -0.1f; prevBeat = -0.2f;
        // terrible & hacky way to reset beat if the counter is clicked
    };
    playPauseButton.setBounds(80, 0, 25, 20);
    
    sampleAddButton.setBounds(10, 100, 60, 20);
    modifierAddButton.setBounds(80, 100, 60, 20);
    resetBeatButton.setBounds(290, 100, 60, 20);
    
    saveStateButton.setBounds(150, 100, 60, 20);
    loadStateButton.setBounds(220, 100, 60, 20);
    newProjectButton.setBounds(360, 100, 60, 20);
    
    int avaliableHeight = getHeight() - controlsHeight;
    
    // some nasty code here but it does work
    
    // == samples ==
    
    auto sampleViewPos = samplesViewport.getViewPosition();
    
    int samplesHeight = avaliableHeight/2 - viewsMargin;

    samplesViewport.setBounds(0, controlsHeight, width, samplesHeight);
    
    int relativeY = 0;
    
    int scrollBarMargin;
    
    if(samplesViewport.getVerticalScrollBar().isShowing())
        scrollBarMargin = samplesViewport.getVerticalScrollBar().getWidth();
    else scrollBarMargin = 0;
    
    for(int i = 0; i < samples.size(); i++) {
        if(samples[i]->isCollapsed()) {
            int height = 40;
            samples[i]->setBounds(componentMargin, relativeY + componentMargin, width - 2*componentMargin - scrollBarMargin, height - 2*componentMargin);
            relativeY+=height;
            
        } else {
            int height = 140;
            samples[i]->setBounds(componentMargin, relativeY + componentMargin, width - 2*componentMargin - scrollBarMargin, height- 2*componentMargin);
            relativeY+=height;
        }
    }
    
    
    samplesComponent.setBounds(0, 0, width, relativeY);
    
    samplesViewport.setViewPosition(sampleViewPos);

    
    // == modifiers ==
    
    auto modifierViewPos = samplesViewport.getViewPosition();
    
    int modifiersHeight = avaliableHeight/2 - viewsMargin;

    modifiersViewport.setBounds(0, controlsHeight + samplesHeight + viewsMargin, width, modifiersHeight);
    
    relativeY = 0;
    
    if(modifiersViewport.getVerticalScrollBar().isShowing())
        scrollBarMargin = modifiersViewport.getVerticalScrollBar().getWidth();
    else scrollBarMargin = 0;
    
    for(int i = 0; i < modifiers.size(); i++) {
        int height = 100;
        modifiers[i]->setBounds(componentMargin, relativeY + componentMargin, width - 2*componentMargin - scrollBarMargin, height - 2*componentMargin);
        relativeY+=height;
    }

    modifiersComponent.setBounds(0, 0, width, relativeY);
    
    modifiersViewport.setViewPosition(modifierViewPos);
    
}

void MainComponent::timerCallback() {
    curBeatLabel.setText(juce::String((double)roundBeat/(std::pow(10, precision)), 1), juce::dontSendNotification);
    curBeatLabel.repaint();
}
