#include "Modifier.h"


// implementation of the Bjorklund algorithm (aka Euclidean Rhythm algorithm) as outlined in
// 'The Euclidean Algorithm Generates Traditional Musical Rhythms'
// by Godfried Toussaint

// Theproblemofprimaryinterestiswhenkandnarerelativelyprime numbers [23], i.e., when k and n are evenly divisible only by 1.

// interval = n, pulses = k


// implementation of euclid algorithm as outlined in
// 'The Euclidean Algorithm Generates Traditional Musical Rhythms'
// by Godfried Toussaint
// http://cgm.cs.mcgill.ca/~godfried/publications/banff.pdf

Modifier::Modifier(std::vector<Sample*> *samplesPointer, std::vector<Modifier*> *modifiersPointer, int index)
: randomInterval("Interval (beats)", "How often (in beats) the modifier should fire", 1.0, 32.0, 1.0, 8.0),
randomMin("Minimum value", "Lowest possible value which the parameter could be assigned", 0.25, 10.0, 0.25, 2.0),
randomMax("Maximum value", "Highest possible value which the parameter could be assigned", 0.25, 10.0, 0.25, 4.0),
randomStep("Value step", "Constrains output to be a multiple of this", 0.25, 10.0, 0.25, 0.25),
cycleLength("Cycle length (beats)", "How long the rhythm cycle should be (in beats)", 1.0, 32.0, 1.0, 16.0),
pulseDuration("Pulse duration (beats)", "How long each 'pulse' ('hit'/'break') should last (in beats)", 0.25, 10.0, 0.25, 0.5),
euclideanNumHits("Hits per cycle", "How many 'hits' (i.e. times the sample will be played) per cycle", 1.0, 32.0, 1.0, 4.0),
patternView(&pattern, &patternPosition)
{
    setOpaque(true); // ??
    
    startTimer(200); // refresh drop down
    
    samples = samplesPointer;
    modifiers = modifiersPointer;
    
    // modifier index
    modifierIndex = index;
    modifierIndexLabel.setText(std::to_string(modifierIndex), juce::dontSendNotification);
    
    // select sample/modifier
    modifierSelectLabel.setText("Object to modify:", juce::dontSendNotification);
    modifierSelectLabel.setTooltip("Select a sample or even another modifier to have its parameters automatically changed by this modifier");
    modifierSelectLabel.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.f, juce::Font::plain));
    modifierSelect.onChange = [this] { updateSelected(); };
    
    // parameter
    populateParameters();
    modifierParameterLabel.setText("Parameter to modify:", juce::dontSendNotification);
    modifierParameterLabel.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.f, juce::Font::plain));
    modifierParameter.onChange = [this] {
        // tell previous parameter we're not modifying it anymore
        if(isValidParam(parameterIndex)) params.at(parameterIndex)->removeModifier(modifierIndex);
        parameterIndex = modifierParameter.getSelectedItemIndex(); parameterChanged();
        // tell new we are
        if(isValidParam(parameterIndex)) params.at(parameterIndex)->addModifier(modifierIndex);
    };
   
    // presets
    populatePresets();
    modifierPresetMenu.onChange = [this] { selectPreset(); };
    modifierPresetLabel.setText("Preset", juce::dontSendNotification);
    
    // mode
    modifierChangeMode.onClick = [this] { changeMode(); };
    modifierChangeMode.setTooltip("Change modifier mode");
    
    // help
    modifierHelp.setButtonText("?");
    modifierHelp.onClick = [this] {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::InfoIcon, "Modifier Help", toolTip());
    };
    modifierHelp.setTooltip("What does this do?");
    
    // cycle position
    modifierCyclePositionLabel.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.f, juce::Font::plain));
    modifierCyclePositionLabel.setText("Cycle position", juce::dontSendNotification);
    
    modifierBack.setButtonText("<");
    modifierBack.onClick = [this] { patternPosition -= 1; patternPosition %= pattern.size(); };
    modifierBack.setTooltip("Change pattern position");
    
    modifierPosition.setJustificationType(juce::Justification::centred);
    modifierPosition.setTooltip("The current position in the cycle, starting on 0");
    
    modifierForward.setButtonText(">");
    modifierForward.onClick = [this] { patternPosition += 1; patternPosition %= pattern.size(); };
    modifierForward.setTooltip("Change pattern position");
    
    // equation
    modifierEquation.setText(equation, juce::dontSendNotification);
    modifierEquation.onTextChange = [this] { equation = modifierEquation.getText().toStdString(); };
    modifierEquationLabel.setText("Equation", juce::dontSendNotification);
    modifierEquationLabel.setJustificationType(juce::Justification::left);
    modifierEquationLabel.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.f, juce::Font::plain));

    patternViewport.setViewedComponent(&patternView, false);
    
    addAndMakeVisible(modifierIndexLabel);
    
    addChildComponent(modifierEquation);
    addChildComponent(modifierEquationLabel);
    
    addChildComponent(modifierPresetLabel);
    addChildComponent(modifierPresetMenu);
    
    addChildComponent(modifierCyclePositionLabel);
    addChildComponent(modifierBack);
    addChildComponent(modifierForward);
    addChildComponent(modifierPosition);
    
    addChildComponent(patternViewport);
    
    addAndMakeVisible(modifierSelect);
    addAndMakeVisible(modifierParameter);
    addAndMakeVisible(modifierChangeMode);
    addAndMakeVisible(modifierParameterLabel);
    addAndMakeVisible(modifierSelectLabel);
    addAndMakeVisible(modifierHelp);

    // params
    addChildComponent(randomInterval);
    addChildComponent(randomMin);
    addChildComponent(randomMax);
    addChildComponent(randomStep);
    addChildComponent(cycleLength);
    addChildComponent(pulseDuration);
    addChildComponent(euclideanNumHits);
    
    modeChanged = true;
    
}

Modifier::~Modifier() {
    
}

void Modifier::highlight(int frameInterval, int length) {
    highlighted = true;
    highlightPos = 0;
    
    highlightLen = length;
    
    repaint();
    
    startTimer(frameInterval); // speed up timer for highlight animation
}

std::vector<Parameter*> Modifier::getParams() {
    std::vector<Parameter*> params;
    params.push_back(&randomInterval);
    params.push_back(&randomMin);
    params.push_back(&randomMax);
    params.push_back(&randomStep);
    
    params.push_back(&cycleLength);
    params.push_back(&pulseDuration);
    params.push_back(&euclideanNumHits);
    return params;
}

void Modifier::parameterChanged() {

    if(isValidParam(parameterIndex)) {
        
        double min = params.at(parameterIndex)->getMin();
        double max = params.at(parameterIndex)->getMax();
        double step = params.at(parameterIndex)->getStep();
        
        randomMin.setMin(min);
        randomMin.setMax(max);
        randomMin.setStep(step);
        
        randomMax.setMin(min);
        randomMax.setMax(max);
        randomMax.setStep(step);
        
        randomStep.setMin(step);
        
        juce::MessageManager::callAsync ([this] { randomMin.updateAll(); randomMax.updateAll(); randomStep.updateAll(); });
        
        if((state == STATE_SAMPLE && isValidSample(selected))
        || (state == STATE_MODIFIER && isValidModifier(selected))) {
            juce::MessageManager::callAsync ([this] { params.at(parameterIndex)->highlight(20, 20); }); // highlight new param
        }
    }
}

void Modifier::populateParameters() {
    
    std::cout << "populating params" << std::endl;
    
    // tell previous parameter we're not modifying it anymore
    if(isValidParam(parameterIndex)) params.at(parameterIndex)->removeModifier(modifierIndex);
    
    int selectedID = modifierParameter.getSelectedId();
    
    modifierParameter.clear();
    
    if(state == STATE_SAMPLE && isValidSample(selected)) {
        Sample *sample = samples->at(selected);
        
        params = sample->getParams();
        
        for(int i = 0; i < params.size(); i++) {
            modifierParameter.addItem(params.at(i)->getLabel(), i+1);
        }
    } else if(state == STATE_MODIFIER && isValidModifier(selected)) {
        
        Modifier *modifier = modifiers->at(selected);
        
        params = modifier->getParams();
        
        for(int i = 0; i < params.size(); i++) {
            modifierParameter.addItem(params.at(i)->getLabel(), i+1);
        }
    }
    
    if(selectedID <= modifierParameter.getNumItems())
        modifierParameter.setSelectedId(selectedID, juce::dontSendNotification);
    
    
    // tell new we are
    if(isValidParam(parameterIndex)) params.at(parameterIndex)->addModifier(modifierIndex);

    
}

void Modifier::changeMode() {
    
    if(mode == MODE_RANDOM) {
        mode = MODE_EUCLIDEAN;
        updateEuclidean();
    } else if(mode == MODE_EUCLIDEAN) {
        mode = MODE_CUSTOM;
    }  else if(mode == MODE_CUSTOM) {
        mode = MODE_EQUATION;
    } else if(mode == MODE_EQUATION) {
        mode = MODE_RANDOM;
    }
    
    slidersChanged = true;
    modeChanged = true;
}

void euclideanRhythmFix(int &numLeft, int &numRight, std::vector<bool> &vecLeft, std::vector<bool> &vecRight) {
    // display input
    std::cout << numLeft << "x[";
    for(int i = 0; i < vecLeft.size(); i++) {
        if(vecLeft.at(i)) std::cout << "1";
        else std::cout << "0";
    }
    std::cout << "], " << numRight << "x[";
    for(int i = 0; i < vecRight.size(); i++) {
        if(vecRight.at(i)) std::cout << "1";
        else std::cout << "0";
    }
    std::cout << "]" << std::endl;
    
    // calculate next interation
    
    if(numRight <= 1) return; // stopping condition of euclid
    
    int oldLeftLen = (int)vecLeft.size();
    
    bool firstIteration = numLeft < numRight;
    
    if(firstIteration) {
        std::swap(numLeft, numRight);
    }
    
    // new left becomes old left + elements from old right
    
    if(numRight == 0) return; // crashh

    
    for(int i = 0; i < numLeft/numRight; i++) { // division thing isnt right
        for(int x = 0; x < vecRight.size(); x++) {
            vecLeft.push_back(vecRight.at(x));
        }
    }
    
    // new right becomes old left, except if its the first iteration
    if(!firstIteration) {
        vecRight.clear();
        for(int i = 0; i < oldLeftLen; i++) {
            vecRight.push_back(vecLeft.at(i));
        }
    }
    
    int newLeft = numRight;
    int newRight = numLeft % numRight;
    
    numLeft = newLeft;
    numRight = newRight;
    
    euclideanRhythmFix(numLeft, numRight, vecLeft, vecRight);
}

std::vector<bool> Modifier::genEuclideanRhythm(int length, int pulses) {
    int breaks = length - pulses;
    
    std::vector<bool> vecLeft;
    vecLeft.push_back(1);
    
    std::vector<bool> vecRight;
    vecRight.push_back(0);
    
    int numLeft = pulses;
    int numRight = breaks;
    euclideanRhythmFix(numLeft, numRight, vecLeft, vecRight);
    
    // recombine
    std::vector<bool> output;
    for(int n = 0; n < numLeft; n++) {
        for(int i = 0; i < vecLeft.size(); i++) {
            output.push_back(vecLeft.at(i));
        }
    }
    for(int n = 0; n < numRight; n++) {
        for(int i = 0; i < vecRight.size(); i++) {
            output.push_back(vecRight.at(i));
        }
    }
    
    // display output
    std::cout << std::endl;
    for(int i = 0; i < output.size(); i++) {
        if(output.at(i)) std::cout << "1";
        else std::cout << "0";
    }
    std::cout << std::endl;
    
    return output;
}

void Modifier::refreshDropdownItems() {
    int selectedID = modifierSelect.getSelectedId();
    
    modifierSelect.clear(juce::dontSendNotification);
    
    // 0-1 = idle
    modifierSelect.addItem("None/disable modifier", 1);
    
    modifierSelect.addSeparator();
    
    modifierSelect.addSectionHeading("Samples:");
    
    // 2 - samples->size+1 = samples
    for(int i = 0; i < samples->size(); i++) {
        juce::String label = std::to_string(i) + " - " + samples->at(i)->getLabel().toStdString();
        modifierSelect.addItem(label, i+2);
        //std::cout << i+2 << std::endl;
    }
    
    modifierSelect.addSeparator();
    
    modifierSelect.addSectionHeading("Modifiers:");
    
    // > samples->size+1 = modifiers
    
    // modifiers
    for(int i = 0; i < modifiers->size(); i++) {
        juce::String label = std::to_string(i) + " - (modifier)";
        
        modifierSelect.addItem(label, i+samples->size()+2);
        //std::cout << i + samples->size() + 2 << std::endl;
    }
    
    if(selectedID <= modifierParameter.getNumItems())
        modifierSelect.setSelectedId(selectedID, juce::dontSendNotification);
}


void Modifier::hideAllControls() {
    // called to hide all, then we can just show the ones we need for the mode we're in
    
    modifierPresetMenu.setVisible(false);
    modifierPresetLabel.setVisible(false);
    modifierParameter.setVisible(false);
    modifierParameterLabel.setVisible(false);
    modifierCyclePositionLabel.setVisible(false);
    modifierBack.setVisible(false);
    modifierPosition.setVisible(false);
    modifierForward.setVisible(false);
    modifierEquation.setVisible(false);
    modifierEquationLabel.setVisible(false);
    patternViewport.setVisible(false);

    // params
    randomInterval.setActive(false);
    randomMin.setActive(false);
    randomMax.setActive(false);
    randomStep.setActive(false);
    cycleLength.setActive(false);
    pulseDuration.setActive(false);
    euclideanNumHits.setActive(false);
}

void Modifier::showRandomControls() {
    modifierChangeMode.setButtonText("Random");
    
    randomInterval.setActive(true);
    randomMin.setActive(true);
    randomMax.setActive(true);
    randomStep.setActive(true);
    
    modifierParameter.setVisible(true);
    modifierParameterLabel.setVisible(true);
}

void Modifier::showEuclideanControls() {
    modifierChangeMode.setButtonText("Euclidean");
    
    euclideanNumHits.setActive(true);
    pulseDuration.setActive(true);
    cycleLength.setActive(true);
    
    modifierPresetLabel.setVisible(true);
    modifierPresetMenu.setVisible(true);
    
    modifierCyclePositionLabel.setVisible(true);
    
    modifierPosition.setVisible(true);
    
    modifierBack.setVisible(true);
    modifierForward.setVisible(true);
    
    patternViewport.setVisible(true);
}

void Modifier::showCustomControls() {
    modifierChangeMode.setButtonText("Custom");
    
    pulseDuration.setActive(true);
    cycleLength.setActive(true);
    
    
    modifierCyclePositionLabel.setVisible(true);
    
    modifierPosition.setVisible(true);
    
    modifierBack.setVisible(true);
    modifierForward.setVisible(true);
    
    patternViewport.setVisible(true);
}

void Modifier::showEquationControls() {
    modifierChangeMode.setButtonText("Equation");
    
    cycleLength.setActive(true);
    pulseDuration.setActive(true);
    
    modifierParameter.setVisible(true);
    modifierParameterLabel.setVisible(true);
    
    modifierEquation.setVisible(true);
    modifierEquationLabel.setVisible(true);
        
    modifierPosition.setVisible(true);
    
    modifierBack.setVisible(true);
    modifierForward.setVisible(true);
}

void Modifier::paint(juce::Graphics& g) {
    
    std::cout << "paint" << std::endl;
    
    if(dropdownChanged) {
        refreshDropdownItems();
        
        // update which dropdown item is selected
        if(state == STATE_IDLE)
            modifierSelect.setSelectedId(1, juce::dontSendNotification);
        if(state == STATE_SAMPLE)
            modifierSelect.setSelectedId(selected+2, juce::dontSendNotification);
        if(state == STATE_MODIFIER)
            modifierSelect.setSelectedId(selected+(int)samples->size()+2, juce::dontSendNotification);
        
        populateParameters();
        
        dropdownChanged = false;
    }
    
    if(slidersChanged) {
        // make sliders reflect true values
        randomInterval.update();
        randomMin.update();
        randomMax.update();
        randomStep.update();
        cycleLength.update();
        pulseDuration.update();
        euclideanNumHits.update();
        
        modifierParameter.setSelectedItemIndex(parameterIndex, juce::dontSendNotification);
        
        slidersChanged = false;
    }
    
    // draw background
    g.setColour(juce::Colour(30, 30, 30));
    g.fillAll();
    
    g.setColour(background);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.f);
    g.setColour(juce::Colour(150, 150, 150));
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 10.f, 1.f);
    
    if(highlighted) {
        g.setColour(juce::Colours::whitesmoke);
        g.setOpacity(((highlightLen-highlightPos)/(float)highlightLen)*0.75f);
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.f, 6.f);
    }
    
    // update position
    modifierPosition.setText(std::to_string(patternPosition), juce::dontSendNotification);
        
    
    if(modeChanged) {
        hideAllControls();
        if(mode == MODE_RANDOM) {
            showRandomControls();
        } else if(mode == MODE_EUCLIDEAN) {
            showEuclideanControls();
            patternView.repaint();
        } else if(mode == MODE_EQUATION) {
            showEquationControls();
        } else if(mode == MODE_CUSTOM) {
            showCustomControls();
            patternView.repaint();
        }
        modeChanged = false;
    }
}


void Modifier::resized() {
    
    modifierIndexLabel.setBounds(0, 0, 40, 40);
    
    
    // divide up into 4 columns
    int marginX = 40; // margins on far left & right of modifier
    int columnWidth = (getWidth() - 2*marginX) / 4;
    
    int margin = 4; // inidividual parameter margins

    int componentHeight = 34;
    int rowHeight = componentHeight+margin*2;
    int componentWidth = columnWidth-margin*2;
    
    // column 1: select, parameter, preset
    int x = marginX;
    int y = margin;
    modifierSelectLabel.setBounds(x, y, componentWidth, 14);
    modifierSelect.setBounds(x, y+14, componentWidth, 20);
    
    y += rowHeight; // these share the same position
    modifierParameterLabel.setBounds(x, y, componentWidth, 14);
    modifierParameter.setBounds(x, y+14, componentWidth, 20);
    modifierPresetLabel.setBounds(x, y, componentWidth, 14);
    modifierPresetMenu.setBounds(x, y+14, componentWidth, 20);

    // column 2: interval, cycle length, min, num hits, equation
    x += columnWidth;
    y = margin;
    randomInterval.setBounds(x, y, componentWidth, 40);
    cycleLength.setBounds(x, y, componentWidth, 40);
    
    y += rowHeight;
    randomMin.setBounds(x, y, componentWidth, 40);
    euclideanNumHits.setBounds(x, y, componentWidth, 40);
    modifierEquationLabel.setBounds(x, y, componentWidth, 14);
    modifierEquation.setBounds(x, y+14, componentWidth, 20);
    
    // column 3: step, duration max, cycle position
    x += columnWidth;
    y = margin;
    randomStep.setBounds(x, y, columnWidth, 40);
    pulseDuration.setBounds(x, y, columnWidth, 40);
    
    y += rowHeight;
    randomMax.setBounds(x, y, columnWidth, 40);
    modifierCyclePositionLabel.setBounds(x, y, columnWidth, 14);
    modifierBack.setBounds(x, y+14, 25, 20);
    modifierPosition.setBounds(x+25, y+14, 25, 20);
    modifierForward.setBounds(x+50, y+14, 25, 20);
    
    // column 4: changemode & help, euclidean drawing
    x += columnWidth;
    y = margin;
    modifierChangeMode.setBounds(x, y, columnWidth, 20);
    modifierHelp.setBounds(x+columnWidth-20, y+20, 20, 20);
    
    y += rowHeight;
    patternViewport.setBounds(x, y, columnWidth, 20 + patternViewport.getHorizontalScrollBar().getHeight());
}

void Modifier::setInterval(double val) {
    randomInterval.setValue(val);
    slidersChanged = true;
}

void Modifier::setMin(double val) {
    randomMin.setValue(val);
    slidersChanged = true;
}

void Modifier::setMax(double val) {
    randomMax.setValue(val);
    slidersChanged = true;
}

void Modifier::setStep(double val) {
    randomStep.setValue(val);
    slidersChanged = true;
}

void Modifier::setSelected(int index) {
    selected = index;
    dropdownChanged = true;
}

void Modifier::setMode(int index) {
    mode = (modifierMode)index;
    slidersChanged = true;
    modeChanged = true;
}

void Modifier::setState(int index) {
    state = (modifierState)index;
    slidersChanged = true;
}

void Modifier::setParameter(int index) {
    populateParameters();
    parameterIndex = index;
    slidersChanged = true;
    dropdownChanged = true;
    parameterChanged();
}

void Modifier::setEquation(std::string eqn) {
    equation = eqn;
    slidersChanged = true;
}

void Modifier::setCycleLength(double len) {
    cycleLength.setValue(len);
    slidersChanged = true;
}

void Modifier::setPulseDuration(double len) {
    pulseDuration.setValue(len);
    slidersChanged = true;
}

void Modifier::setEuclideanHits(double num) {
    euclideanNumHits.setValue(num);
    slidersChanged = true;
}

long Modifier::pow10(double input, int power) {
    for(int i = 0; i < power; i++) {
        input*=10.0;
    }
    return input;
}

void Modifier::tickPlayPattern(long roundedBeat, long prevBeat) {
    if(state != STATE_SAMPLE) return;
    
    if(pattern.size() == 0) return;
    
    if((roundedBeat > prevBeat) && ((roundedBeat % roundedPulseDuration) == 0)) {
        // every step, work out whether the sample should be playing
        
        patternPosition += 1;
        patternPosition %= pattern.size();
        
        if(selected >= 0 & selected < samples->size()) {
           // valid sample selected
            Sample *sample = samples->at(selected);
        
            if(pattern.at(patternPosition)) {
                // play
                background = juce::Colour(50, 100, 50);
                juce::MessageManager::callAsync ([this] { repaint(); });

                
                sample->setInterval(pulseDuration.getValue()); // play immediately?
            } else {
                // don't play
                background = juce::Colour(80, 80, 80);
                juce::MessageManager::callAsync ([this] { repaint(); });
                
                // disable the sample from starting for an interval of 'step'
                sample->setInterval(pulseDuration.getValue());
                sample->disable();

            }
        }
    }
}

bool Modifier::isValidParam(int index) {
    return (index >= 0 && index < params.size());
}

bool Modifier::isValidSample(int index) {
    return (index >= 0 && index < samples->size());
}

bool Modifier::isValidModifier(int index) {
    return (index >= 0 && index < modifiers->size());
}

void Modifier::tickRandom(long roundedBeat, long prevBeat) {
    if(roundedInterval == 0) return; // something br0ke
    if((roundedBeat > prevBeat) && ((roundedBeat % roundedInterval) == 0)) {
        // every interval we do something?
        
        // check we have a valid object to modify
        if(state == STATE_IDLE) return;
        if(state == STATE_SAMPLE && ((selected < 0) || (selected >= samples->size()))) return;
        if(state == STATE_MODIFIER && ((selected < 0) || (selected >= modifiers->size()))) return;
        
        juce::Random r = juce::Random();
        
        int iMin = (int)std::ceil(randomMin.getValue() / randomStep.getValue());
        int iMax = (int)std::floor(randomMax.getValue() / randomStep.getValue());
        int iRange = iMax - iMin;
        
        if(iMax > 0 && iRange > 0) {
            int n = r.nextInt(iRange + 1) + iMin;
            double newVal = (double)n*randomStep.getValue();
            
            if((state == STATE_SAMPLE && isValidSample(selected))
            || (state == STATE_MODIFIER && isValidModifier(selected))) {
                
                if(isValidParam(parameterIndex)) {
                    params.at(parameterIndex)->setValue(newVal);
                    juce::MessageManager::callAsync ([this] { params.at(parameterIndex)->update(); params.at(parameterIndex)->highlight(); highlight(); });
                }
                
            }
        } // otherwise user has entered parameters wrong
    }
}

void Modifier::tickEquation(long roundedBeat, long prevBeat) {
    if(state != STATE_SAMPLE) return;
    
    if((roundedBeat > prevBeat) && ((roundedBeat % roundedPulseDuration) == 0)) {
        patternPosition += 1;
        patternPosition %= (int)cycleLength.getValue();
        
        if(selected >= 0 & selected < samples->size()) {
           // valid sample selected
            
            juce::MessageManager::callAsync ([this] { repaint(); });
            
            double newVal = parseEquation(modifierEquation.getText().toStdString());
            std::cout << newVal << std::endl;
            
            if((state == STATE_SAMPLE && isValidSample(selected))
            || (state == STATE_MODIFIER && isValidModifier(selected))) {
                
                if(isValidParam(parameterIndex)) {
                    params.at(parameterIndex)->setValue(newVal);
                    juce::MessageManager::callAsync ([this] { params.at(parameterIndex)->update(); });
                }
                
            }
            
            /*if(parameter == PARAMETER_INTERVAL)
                sample->setInterval(newVal);
            if(parameter == PARAMETER_DELAY)
                sample->setDelay(newVal);
            if(parameter == PARAMETER_BPM)
                sample->setBpm(newVal);*/
        }
    }
}

void Modifier::tick(long roundedBeat, long prevBeat) {
    if(mode == MODE_EUCLIDEAN) {
        tickPlayPattern(roundedBeat, prevBeat);
    } else if(mode == MODE_RANDOM) {
        tickRandom(roundedBeat, prevBeat);
    } else if(mode == MODE_EQUATION) {
       tickEquation(roundedBeat, prevBeat);
    } else if(mode == MODE_CUSTOM) {
        tickPlayPattern(roundedBeat, prevBeat);
    }
}

void Modifier::updateEuclidean() {
    int length = (int)cycleLength.getValue();
    int beats = (int)euclideanNumHits.getValue();
    
    pattern = genEuclideanRhythm(length, beats);
    
    // get current sample
    if(selected >= 0 & selected < samples->size()) {
       // valid sample selected
        Sample *sample = samples->at(selected);
        sample->setInterval(cycleLength.getValue());
    }
    
    juce::MessageManager::callAsync([this] { patternView.setBounds(0, 0, patternView.patternWidth(), 20); });
}

void Modifier::updateSelected() {
    
    int selectedId = modifierSelect.getSelectedId();
    
    if(selectedId <= 1) {
        state = STATE_IDLE;
    } else if(selectedId > 1 && selectedId < samples->size() + 2) { // sample
        selected = modifierSelect.getSelectedId() - 2;
        state = STATE_SAMPLE;
    } else { // modifier
        selected = (int)modifierSelect.getSelectedId() - samples->size() - 2;
        state = STATE_MODIFIER;
    }
    
    populateParameters();
    parameterChanged();

}

void Modifier::updateParams(int precision) {
    if(slidersChanged || dropdownChanged) juce::MessageManager::callAsync ([this] { repaint(); });
    
    roundedInterval = pow10(randomInterval.getValue(), precision);
    roundedStep = pow10(randomStep.getValue(), precision);
    roundedPulseDuration = pow10(pulseDuration.getValue(), precision);
    
    if(mode == MODE_EUCLIDEAN) {
        // if any parameters are changed we update the euclidean
        if(cycleLength.isChanged() || pulseDuration.isChanged() || euclideanNumHits.isChanged()) {
            std::cout << "UPDATEEUCLID" << std::endl;
            updateEuclidean();
            
            juce::MessageManager::callAsync ([this] { repaint(); });
            
            // todo: check if it matches a preset before doing this
            juce::MessageManager::callAsync ([this] { modifierPresetMenu.setSelectedId(0, juce::dontSendNotification); });
            // then these are all auto set to false
        }
        
        patternView.setEditable(false);
    }
    
    if(mode == MODE_CUSTOM) {
        if(cycleLength.isChanged()) {
            pattern.resize((int)cycleLength.getValue(), false);
            
            juce::MessageManager::callAsync([this] { patternView.setBounds(0, 0, patternView.patternWidth(), 20); repaint(); });
            
        }
        
        patternView.setEditable(true);
    }
}

std::string Modifier::toString() {
    std::string output = "";
    output += "Modifier <name> {\n";
    output += "mode " + std::to_string(mode) + "\n";
    output += "state " + std::to_string(state) + "\n";
    output += "selected " + std::to_string(selected) + "\n";
    output += "parameter " + std::to_string(parameterIndex) + "\n";
    output += "interval " + std::to_string(randomInterval.getValue()) + "\n";
    output += "min " + std::to_string(randomMin.getValue()) + "\n";
    output += "max " + std::to_string(randomMax.getValue()) + "\n";
    output += "step " + std::to_string(randomStep.getValue()) + "\n";
    output += "cyclelength " + std::to_string(cycleLength.getValue()) + "\n";
    output += "pulseduration " + std::to_string(pulseDuration.getValue()) + "\n";
    output += "euclideanhits " + std::to_string(euclideanNumHits.getValue()) + "\n";
    output += "equation " + equation + "\n";
    output += "}\n";
    return output;
}

double Modifier::parseEquation(std::string input) {
    std::stack<double> st;
    
    // split equation by space into tokens
    
    std::string tmp;
    std::stringstream ss(input);
    std::vector<std::string> tokens;
    
    while(ss >> tmp)
        tokens.push_back(tmp);

    // parse these tokens using RPN
    
    for(int i = 0; i < tokens.size(); i++) {
        
        // add numbers / variables to stack
        
        bool isNumber = (tokens.at(i).find_first_not_of( "0123456789" ) == std::string::npos); // from https://stackoverflow.com/questions/2844817/how-do-i-check-if-a-c-string-is-an-int/37864920
        bool isVariable = (tokens.at(i).length() == 1) && (isupper(tokens.at(i).at(0)));
        
        if(isNumber) st.push((double)std::stoi(tokens.at(i)));
        if(isVariable) st.push((double)patternPosition); // let all variables be euclideanPosition for now? haha
        
        
        // TODO: otherwise throw error
        
        // functions
        if(tokens.at(i) == "+") {
            if(st.size() < 2) return -1;
            
            double arg2 = st.top();
            st.pop();
            double arg1 = st.top();
            st.pop();
            
            double result = arg1 + arg2;
            st.push(result);
        }
        
        if(tokens.at(i) == "-") {
            if(st.size() < 2) return -1;
            
            double arg2 = st.top();
            st.pop();
            double arg1 = st.top();
            st.pop();
            
            double result = arg1 - arg2;
            st.push(result);
        }
        
        if(tokens.at(i) == "*") {
            if(st.size() < 2) return -1;
            
            double arg2 = st.top();
            st.pop();
            double arg1 = st.top();
            st.pop();
            
            double result = arg1 * arg2;
            st.push(result);
        }
        
        if(tokens.at(i) == "/") {
            if(st.size() < 2) return -1;
            
            double arg2 = st.top();
            st.pop();
            double arg1 = st.top();
            st.pop();
            
            if(arg2 == 0) return -1;
            
            double result = arg1 / arg2;
            st.push(result);
        }
        
        if(tokens.at(i) == "sin") {
            if(st.size() < 1) return -1;
            
            double arg1 = st.top();
            st.pop();
            
            double result = std::sin(arg1);
            st.push(result);
        }
        
        if(tokens.at(i) == "cos") {
            if(st.size() < 1) return -1;
            
            double arg1 = st.top();
            st.pop();
            
            double result = std::cos(arg1);
            st.push(result);
        }
        
        if(tokens.at(i) == "**") {
            if(st.size() < 2) return -1;
            
            double arg2 = st.top();
            st.pop();
            double arg1 = st.top();
            st.pop();
            
            double result = std::pow(arg1, arg2);
            st.push(result);
        }
        
        
    }
    
    // return top of stack
    if(st.size() > 0) return st.top();
    else return -1;
}

void Modifier::populatePresets() {
    // populating with some examples from the euclidean rhythms paper
    //euclideanPresets.push_back({ "Afro-Cuban", 2, 3 });
    euclideanPresets.push_back({ "Khafif-e-ramal", 2, 5 });
    euclideanPresets.push_back({ "tresillo", 3, 8 });
    euclideanPresets.push_back({ "Agsag-Samai", 5, 9 });
    
    for(int i = 0; i < euclideanPresets.size(); i++) {
        modifierPresetMenu.addItem(euclideanPresets.at(i).name, i+1);
    }
}

void Modifier::selectPreset() {
    int id = modifierPresetMenu.getSelectedId();
    
    if(id > 0) {
        euclideanPreset preset = euclideanPresets.at(id-1);
        
        cycleLength.setValue((double)preset.interval);
        euclideanNumHits.setValue((double)preset.hits);
        cycleLength.update();
        euclideanNumHits.update();
        
        updateEuclidean();
    }
}

std::string Modifier::toolTip() {
    std::string str;
    
    std::string sampleName = modifierSelect.getText().toStdString();
    if(sampleName.empty()) sampleName = "<not selected>";
    std::string parameterName = modifierParameter.getText().toStdString();
    if(parameterName.empty()) parameterName = "<not selected>";
    
    if(mode == MODE_RANDOM) {
        str = "This modifier changes the sample <" + sampleName + ">'s " + parameterName + " to a random value between " + juce::String(randomMin.getValue(), 2).toStdString() + " and " + juce::String(randomMax.getValue(), 2).toStdString() + " which is divisible by " + juce::String(randomStep.getValue(), 2).toStdString() + ", every " + juce::String(randomInterval.getValue(), 2).toStdString() + " beats.";
    }
    
    if(mode == MODE_EUCLIDEAN) {
        str = "This modifier uses a Euclidean algorithm to generate a rhythm for sample <" + sampleName + "> of length " + std::to_string((int)cycleLength.getValue()) + " , containing " + std::to_string((int)euclideanNumHits.getValue()) + " 'hits' and " + std::to_string((int)cycleLength.getValue()-(int)euclideanNumHits.getValue()) + " 'breaks'. Each hit or break lasts " + juce::String(pulseDuration.getValue(), 2).toStdString() + " beats.";
    }
    
    if(mode == MODE_EQUATION) {
        str = "Enter an equation using reverse polish notation, using X as a variable"; // TODO: this
    }
    
    if(mode == MODE_CUSTOM) {
        str = "Create a custom pattern"; // TODO: this
    }
    
    return str;
}

void Modifier::timerCallback() {
    
    if(highlighted) {
        if(highlightPos == highlightLen) {
            highlighted = false;
            startTimer(200); // go back to slow ticks to update dropdown
        }
    
        repaint();
        highlightPos++;
    
    } else {
        refreshDropdownItems();
    }
    
}

void Modifier::mouseDown(const juce::MouseEvent& event) {

    if(event.mods.isRightButtonDown()) {
        // show right click menu
        
        juce::PopupMenu m;
        m.addSectionHeader("Mode");
        m.addItem("Random", true, (mode == MODE_RANDOM), ([this]{mode = MODE_RANDOM; slidersChanged = true; modeChanged = true; }));
        m.addItem("Euclidean", true, (mode == MODE_EUCLIDEAN), ([this]{mode = MODE_EUCLIDEAN; updateEuclidean(); slidersChanged = true; modeChanged = true; }));
        m.addItem("Custom", true, (mode == MODE_CUSTOM), ([this]{mode = MODE_CUSTOM; slidersChanged = true; modeChanged = true; }));
        m.addItem("Equation", true, (mode == MODE_EQUATION), ([this]{mode = MODE_EQUATION; slidersChanged = true; modeChanged = true; }));
        m.show();
    }
}
