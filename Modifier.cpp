//
//  Modifier.cpp
//  Proto - App
//
//  Created by Felix Thomas on 01/03/2021.
//

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

Modifier::Modifier(std::vector<Sample*> *samplesPointer, std::vector<Modifier*> *modifiersPointer) {
    setOpaque(true);
    
    samples = samplesPointer;
    modifiers = modifiersPointer;
    
    modifierFunction.addItem("Interval", 1);
    modifierFunction.addItem("Delay", 2);
    modifierFunction.addItem("BPM", 3);
    
    modifierFunctionLabel.setText("Parameter to modify:", juce::dontSendNotification);
    modifierFunction.onChange = [this] { getParams(); };
    modifierSelectLabel.setText("Object to modify:", juce::dontSendNotification);
    modifierSelectLabel.setTooltip("Select a sample or even another modifier to have its parameters automatically changed by this modifier");
    modifierSelect.onChange = [this] { getParams(); };
    
    
    modifierIntervalLabel.setText("Interval (beats)", juce::dontSendNotification);
    modifierInterval.setRange(1, 32, 1);
    modifierInterval.setValue(interval, juce::dontSendNotification);
    modifierInterval.onValueChange = [this] { getParams(); };

    modifierMinLabel.setText("Minimum value", juce::dontSendNotification);
    modifierMin.setRange(0.25, 10, 0.25);
    modifierMin.setValue(min, juce::dontSendNotification);
    modifierMin.onValueChange = [this] { getParams(); };

    modifierMaxLabel.setText("Maximum value", juce::dontSendNotification);
    modifierMaxLabel.setTooltip("Highest possible value which the parameter could be assigned");
    modifierMax.setRange(0.25, 10, 0.25);
    modifierMax.setValue(max, juce::dontSendNotification);
    modifierMax.onValueChange = [this] { getParams(); };

    modifierStepLabel.setText("Value step", juce::dontSendNotification);
    modifierStepLabel.setTooltip("Value generated must be divisible by this");
    modifierStep.setRange(0.25, 10, 0.25);
    modifierStep.setValue(step, juce::dontSendNotification);
    modifierStep.onValueChange = [this] { getParams(); };
    
    modifierChangeMode.setButtonText("Random");
    modifierChangeMode.onClick = [this] { changeMode(); };
    modifierChangeMode.setTooltip("Change modifier mode");
    
    modifierHelp.setButtonText("?");
    modifierHelp.onClick = [this] {
        // TODO: change this to be a tool tip
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::InfoIcon, "Modifier Help", toolTip());
        };
    modifierHelp.setTooltip("What does this do?");
    
    modifierBack.setButtonText("<");
    modifierBack.onClick = [this] { euclideanPosition-=1; euclideanPosition%=modifierEuclideanRhythm.size(); };
    modifierBack.setTooltip("Change pattern position");
    
    modifierForward.setButtonText(">");
    modifierForward.onClick = [this] { euclideanPosition+=1; euclideanPosition%=modifierEuclideanRhythm.size(); };
    modifierBack.setTooltip("Change pattern position");
    
    modifierPosition.setJustificationType(juce::Justification::centred);
    
    modifierEquation.setText(equation, juce::dontSendNotification);
    modifierEquation.onTextChange = [this] { getParams(); };
    modifierEquationLabel.setText("Equation", juce::dontSendNotification);
    
    populatePresets();
    modifierPresetMenu.onChange = [this] { selectPreset(); };
    modifierPresetLabel.setText("Preset", juce::dontSendNotification);
    
    modifierRefresh.setButtonText("Refresh");
    modifierRefresh.onClick = [this] { updateDropdown(); };
    modifierRefresh.setTooltip("Refresh the dropdown");
    
    addChildComponent(modifierEquation);
    addChildComponent(modifierEquationLabel);
    
    addChildComponent(modifierPresetLabel);
    addChildComponent(modifierPresetMenu);
    
    addChildComponent(modifierBack);
    addChildComponent(modifierForward);
    addChildComponent(modifierPosition);
    
    addAndMakeVisible(modifierSelect);
    addAndMakeVisible(modifierRefresh);
    addAndMakeVisible(modifierFunction);
    addAndMakeVisible(modifierInterval);
    addAndMakeVisible(modifierMin);
    addAndMakeVisible(modifierMax);
    addAndMakeVisible(modifierStep);
    addAndMakeVisible(modifierChangeMode);
    addAndMakeVisible(modifierIntervalLabel);
    addAndMakeVisible(modifierMaxLabel);
    addAndMakeVisible(modifierMinLabel);
    addAndMakeVisible(modifierStepLabel);
    addAndMakeVisible(modifierFunctionLabel);
    addAndMakeVisible(modifierSelectLabel);
    addAndMakeVisible(modifierHelp);

    
}

Modifier::~Modifier() {
    
}

void Modifier::changeMode() {
        
    if(mode == MODE_RANDOM) {
        
        // set mode to MODE_EUCLIDEAN
        mode = MODE_EUCLIDEAN;
        
        
    } else if(mode == MODE_EUCLIDEAN) {
        
        // set mode to MODE_RANDOM
        mode = MODE_EQUATION;

    } else if(mode == MODE_EQUATION) {
        mode = MODE_RANDOM;
    }
    
    slidersChanged = true;
    
    getParams();
}

void euclideanRhythm(int &numLeft, int &numRight, std::vector<bool> &vecLeft, std::vector<bool> &vecRight) {
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
    
    
    // new left = old left + old right
    
    int leftSize = (int)vecLeft.size();
    
    for(int i = 0; i < vecRight.size(); i++) {
        vecLeft.push_back(vecRight.at(i));
    }
    
    if(numLeft < numRight) {
        // handle edge case of first iteration
        std::swap(numLeft, numRight);
        
    } else {
        // new right = old left
        vecRight.clear();
        for(int i = 0; i < leftSize; i++) {
            vecRight.push_back(vecLeft.at(i));
        }
    }
    
    
    int newLeft = numRight;
    int newRight = numLeft % numRight;
    
    numLeft = newLeft;
    numRight = newRight;
    
    euclideanRhythm(numLeft, numRight, vecLeft, vecRight);
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

void Modifier::updateDropdown() {
    modifierSelect.clear(juce::dontSendNotification);
    
    // 0-1 = idle
    modifierSelect.addItem("None selected", 1);
    
    modifierSelect.addSectionHeading("SAMPLES");
    
    // 2 - samples->size+1 = samples
    for(int i = 0; i < samples->size(); i++) {
        const juce::String label = samples->at(i)->getLabel();
        modifierSelect.addItem(label, i+2);
        std::cout << i+2 << std::endl;
    }
    
    modifierSelect.addSeparator();
    
    modifierSelect.addSectionHeading("MODIFIERS");
    
    // > samples->size+1 = modifiers
    
    // modifiers
    for(int i = 0; i < modifiers->size(); i++) {
        const juce::String label = "Modifier" + std::to_string(i);
        modifierSelect.addItem(label, i+samples->size()+2);
        std::cout << i + samples->size() + 2 << std::endl;
    }
}

void Modifier::paint(juce::Graphics& g) {
    
    std::cout << "MODIFIER repaint " << std::endl;
    
    //g.setColour(juce::Colour(100, 100, 100));
    g.setColour(background);
    
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.f);
    
    // update position
    modifierPosition.setText(std::to_string(euclideanPosition), juce::dontSendNotification);
    
    // make sliders reflect true values
    modifierInterval.setValue(interval, juce::dontSendNotification);
    modifierMin.setValue(min, juce::dontSendNotification);
    modifierMax.setValue(max, juce::dontSendNotification);
    modifierStep.setValue(step, juce::dontSendNotification);
    
    if(state == STATE_IDLE)
        modifierSelect.setSelectedId(1, juce::dontSendNotification);
    if(state == STATE_SAMPLE)
        modifierSelect.setSelectedId(sampleIndex+2, juce::dontSendNotification);
    if(state == STATE_MODIFIER)
        modifierSelect.setSelectedId(modifierIndex+samples->size()+2, juce::dontSendNotification);
    
    modifierFunction.setSelectedId(functionIndex, juce::dontSendNotification);
    
    if(mode == MODE_RANDOM) {
        
        
        modifierChangeMode.setButtonText("Random");
        
        modifierMin.setVisible(true);
        modifierMinLabel.setVisible(true);
        modifierMinLabel.setText("Minimum value", juce::dontSendNotification);
        modifierMinLabel.setTooltip("Lowest possible value which the parameter could be assigned");
        
        modifierMax.setVisible(true);
        modifierMaxLabel.setVisible(true);
        
        modifierStep.setVisible(true);
        modifierStepLabel.setVisible(true);
        modifierStepLabel.setText("Value step", juce::dontSendNotification);
        //modifierStepLabel.setText("Step");
        
        modifierIntervalLabel.setText("Interval (beats)", juce::dontSendNotification);
        modifierIntervalLabel.setTooltip("How often (in beats) the modifier should fire");
        modifierFunction.setVisible(true);
        modifierFunctionLabel.setVisible(true);
        modifierEquation.setVisible(false);
        modifierEquationLabel.setVisible(false);
        
        modifierPresetLabel.setVisible(false);
        modifierPresetMenu.setVisible(false);
        
        modifierPosition.setVisible(false);
        
        modifierBack.setVisible(false);
        modifierForward.setVisible(false);
        
    } else if(mode == MODE_EUCLIDEAN) {
        
        modifierChangeMode.setButtonText("Euclidean");
        
        modifierMin.setVisible(true);
        modifierMinLabel.setVisible(true);
        modifierMinLabel.setText("Hits per cycle", juce::dontSendNotification);
        modifierMinLabel.setTooltip("How many 'hits' (i.e. times the sample will be played) per cycle");
        
        modifierMax.setVisible(false);
        modifierMaxLabel.setVisible(false);
        
        modifierStep.setVisible(true);
        modifierStepLabel.setVisible(true);
        modifierStepLabel.setText("Rhythm pulse length (beats)", juce::dontSendNotification);
        modifierStepLabel.setTooltip("How long each 'pulse' ('hit' or 'break') should be (in beats)");
        
        modifierIntervalLabel.setText("Euclidean rhythm length (beats)", juce::dontSendNotification);
        modifierIntervalLabel.setTooltip("How long the Euclidean rhythm cycle should be (in beats)");
        
        modifierFunction.setVisible(false);
        modifierFunctionLabel.setVisible(false);
        
        modifierEquation.setVisible(false);
        modifierEquationLabel.setVisible(false);
        
        modifierPresetLabel.setVisible(true);
        modifierPresetMenu.setVisible(true);
        
        modifierPosition.setVisible(true);
        
        modifierBack.setVisible(true);
        modifierForward.setVisible(true);
        
        if(modifierEuclideanRhythm.size() >= 0) {
            // paint graphical representation of the euclidean rhythm
            int x = 550;
            int y = 25;
            
            // white background rectangle
            g.setColour(juce::Colours::white);
            g.fillRoundedRectangle(x, y, 20*modifierEuclideanRhythm.size(), 20, 4.f);
            
            for(int i = 0; i < modifierEuclideanRhythm.size(); i++) {
                if(modifierEuclideanRhythm.at(i)) g.setColour(juce::Colours::green);
                else g.setColour(juce::Colours::red);
                
                if(euclideanPosition == i) g.setOpacity(1.0f);
                else g.setOpacity(0.6f);
                
                
                g.fillRoundedRectangle(x+1, y+1, 20-2, 20-2, 7.f);
                x+=20;
            }
        }
        
    } else if(mode == MODE_EQUATION) {
       
        modifierChangeMode.setButtonText("Equation");
        
        modifierMin.setVisible(false);
        modifierMinLabel.setVisible(false);
        
        modifierMax.setVisible(false);
        modifierMaxLabel.setVisible(false);
        
        modifierStep.setVisible(true);
        modifierStepLabel.setVisible(true);
        modifierStepLabel.setText("Rhythm pulse length (beats)", juce::dontSendNotification);

        modifierIntervalLabel.setText("Interval", juce::dontSendNotification);
        
        modifierFunction.setVisible(true);
        modifierFunctionLabel.setVisible(true);
        
        modifierEquation.setVisible(true);
        modifierEquationLabel.setVisible(true);
        
        modifierPresetLabel.setVisible(false);
        modifierPresetMenu.setVisible(false);
        
        modifierPosition.setVisible(true);
        
        modifierBack.setVisible(false);
        modifierForward.setVisible(false);
       
   }
    
    slidersChanged = false;
}


void Modifier::resized() {
    modifierSelectLabel.setBounds(15, 0, 120, 20);
    modifierSelect.setBounds(15, 20, 120, 20);
    
    modifierRefresh.setBounds(135, 20, 40, 20);
    
    modifierFunctionLabel.setBounds(15, 40, 120, 20);
    modifierFunction.setBounds(15, 60, 120, 20);
    
    modifierIntervalLabel.setBounds(200, 0, 150, 20);
    modifierInterval.setBounds(200, 20, 150, 20);
    
    modifierMinLabel.setBounds(200, 40, 150, 20);
    modifierMin.setBounds(200, 60, 150, 20);
    modifierMaxLabel.setBounds(350, 40, 150, 20);
    modifierMax.setBounds(350, 60, 150, 20);
    modifierStepLabel.setBounds(550, 40, 150, 20);
    modifierStep.setBounds(550, 60, 150, 20);
    
    modifierChangeMode.setBounds(550, 5, 120, 20);
    
    modifierBack.setBounds(465, 25, 25, 20);
    modifierPosition.setBounds(490, 25, 25, 20);
    modifierForward.setBounds(515, 25, 25, 20);
    
    modifierHelp.setBounds(680, 5, 20, 20);
    
    modifierEquationLabel.setBounds(200, 40, 150, 20);
    modifierEquation.setBounds(200, 60, 150, 20);
    
    modifierPresetLabel.setBounds(15, 40, 120, 20);
    modifierPresetMenu.setBounds(15, 60, 120, 20);
    
}

void Modifier::setInterval(double val) {
    interval = val;
    slidersChanged = true;
}

void Modifier::setMin(double val) {
    min = val;
    slidersChanged = true;
}

void Modifier::setMax(double val) {
    max = val;
    slidersChanged = true;
}

void Modifier::setStep(double val) {
    step = val;
    slidersChanged = true;
}

void Modifier::setSample(int index) {
    sampleIndex = index;
    slidersChanged = true;
}

void Modifier::setMode(int index) {
    mode = (modifierMode)index;
    slidersChanged = true;
    
    getParams();
}

void Modifier::setFunction(int index) {
    functionIndex = index;
    slidersChanged = true;
}

long Modifier::pow10(float input, int power) {
    for(int i = 0; i < power; i++) {
        input*=10.f;
    }
    return (long)input;
}

void Modifier::tickEuclidean(long roundedBeat, long prevBeat) {
    if(modifierEuclideanRhythm.size() == 0) return;
    
    if((roundedBeat > prevBeat) && ((roundedBeat % roundedStep) == 0)) {
        // every step, work out whether the sample should be playing
        
        euclideanPosition+=1;
        euclideanPosition%=modifierEuclideanRhythm.size();
        //if(euclideanPosition == 0) std::cout << std::endl << "NEW" << std::endl;
        
        if(sampleIndex >= 0 & sampleIndex < samples->size()) {
           // valid sample selected
            Sample *sample = samples->at(sampleIndex);
        
            if(modifierEuclideanRhythm.at(euclideanPosition)) {
                // play
                background = juce::Colour(50, 100, 50);
                juce::MessageManager::callAsync ([this] { repaint(); });
                //std::cout << "1";
                //sample->setDelay(0);
                sample->setInterval(step); // play immediately?
            } else {
                // don't play
                background = juce::Colour(100, 100, 100);
                juce::MessageManager::callAsync ([this] { repaint(); });
                //std::cout << "0";
                
                // disable the sample from starting for an interval of 'step'
                sample->setInterval(step);
                sample->disable();
                
                //sample->setInterval(interval); // disable
                //if(sample->getDelay() == step) sample->setDelay(0);
                //else sample->setDelay(step);
            }
        }
    }
}

void Modifier::tickRandom(long roundedBeat, long prevBeat) {
    if(roundedInterval == 0) return; // something br0ke
    if((roundedBeat > prevBeat) && ((roundedBeat % roundedInterval) == 0)) {
        // every interval we do something?
        
        // check we have a valid object to modify
        if(state == STATE_IDLE) return;
        if(state == STATE_SAMPLE && ((sampleIndex < 0) || (sampleIndex >= samples->size()))) return;
        if(state == STATE_MODIFIER && ((modifierIndex < 0) || (modifierIndex >= modifiers->size()))) return;
        
        juce::Random r = juce::Random();
        
        // min 1.5, max 2.25, step = 0.25
        // iMin = 1.5/0.25 = 6
        // iMax = 9
        // iRange = 3
        // inclusive
        
        int iMin = (int)std::ceil(min / step);
        int iMax = (int)std::floor(max / step);
        int iRange = iMax - iMin;
        
        if(iMax > 0 && iRange > 0) {
            int n = r.nextInt(iRange + 1) + iMin;
            double newVal = (double)n*step;
            
            if(state == STATE_SAMPLE) {
                Sample *sample = samples->at(sampleIndex);

                if(functionIndex == FUNCTION_INTERVAL)
                    sample->setInterval(newVal);
                if(functionIndex == FUNCTION_DELAY)
                    sample->setDelay(newVal);
                if(functionIndex == FUNCTION_BPM)
                    sample->setBpm(newVal*10);
            } else {
                Modifier *modifier = modifiers->at(modifierIndex);

                if(functionIndex == FUNCTION_INTERVAL)
                    modifier->setInterval(newVal);
            }
        } // otherwise user has entered parameters wrong
    }
}

void Modifier::tickEquation(long roundedBeat, long prevBeat) {
    
    if((roundedBeat > prevBeat) && ((roundedBeat % roundedStep) == 0)) {
        euclideanPosition+=1;
        euclideanPosition%=(int)interval;
        
        if(sampleIndex >= 0 & sampleIndex < samples->size()) {
           // valid sample selected
            
            juce::MessageManager::callAsync ([this] { repaint(); });
            Sample *sample = samples->at(sampleIndex);
            
            double newVal = parseEquation(modifierEquation.getText().toStdString());
            std::cout << newVal << std::endl;
            
            if(newVal < 0) return;
            
            if(functionIndex == FUNCTION_INTERVAL)
                sample->setInterval(newVal);
            if(functionIndex == FUNCTION_DELAY)
                sample->setDelay(newVal);
            if(functionIndex == FUNCTION_BPM)
                sample->setBpm(newVal);
        }
    }
}

void Modifier::tick(long roundedBeat, long prevBeat) {
    if(mode == MODE_EUCLIDEAN) {
        tickEuclidean(roundedBeat, prevBeat);
    } else if(mode == MODE_RANDOM) {
        tickRandom(roundedBeat, prevBeat);
    } else if(mode == MODE_EQUATION) {
       tickEquation(roundedBeat, prevBeat);
   }
}

void Modifier::getParams() {
    std::cout << "GET PARAMS" << tmp++ << std::endl;
        
    modifierPresetMenu.setSelectedId(0);
    
    interval = modifierInterval.getValue();
    min = modifierMin.getValue();
    max = modifierMax.getValue();
    step = modifierStep.getValue();
    
    int selectedId = modifierSelect.getSelectedId();
    
    functionIndex = modifierFunction.getSelectedId();
    
    
    std::cout << "id" << selectedId << std::endl;
    if(selectedId <= 1) {
        state = STATE_IDLE;
        std::cout << "n/a" << std::endl;
    } else if(selectedId > 1 && selectedId < samples->size() + 2) { // sample
        sampleIndex = modifierSelect.getSelectedId() - 2;
        state = STATE_SAMPLE;
        std::cout << "sample" << sampleIndex << std::endl;
    } else { // modifier
        modifierIndex = modifierSelect.getSelectedId() - samples->size() - 2;
        state = STATE_MODIFIER;
        std::cout << "modifier " << modifierIndex << std::endl;
    }
    std::cout << "function " << functionIndex << std::endl;
    
    
    if(mode == MODE_EUCLIDEAN) {
        int length = (int)interval;
        int beats = (int)min;
        
        modifierEuclideanRhythm = genEuclideanRhythm(length, beats);
        //std::cout << "terminated" << std::endl;
        
        // get current sample
        if(sampleIndex >= 0 & sampleIndex < samples->size()) {
           // valid sample selected
            Sample *sample = samples->at(sampleIndex);
            sample->setInterval(interval);
        }
    } else if(mode == MODE_EQUATION) {
        equation = modifierEquation.getText().toStdString();
        
        // somehow parse this lol
    }
}

void Modifier::updateParams(int precision) {
    if(slidersChanged) juce::MessageManager::callAsync ([this] { repaint(); });
    
    roundedInterval = pow10((float)interval, precision);
    roundedStep = pow10((float)step, precision);
}

std::string Modifier::toString() {
    std::string output = "";
    output += "Modifier <name> {\n";
    output += "mode " + std::to_string(mode) + "\n";
    output += "function " + std::to_string(functionIndex) + "\n";
    output += "sample " + std::to_string(sampleIndex) + "\n";
    output += "interval " + std::to_string(interval) + "\n";
    output += "min " + std::to_string(min) + "\n";
    output += "max " + std::to_string(max) + "\n";
    output += "step " + std::to_string(step) + "\n";
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
        if(isVariable) st.push((double)euclideanPosition); // let all variables be euclideanPosition for now? haha
        
        
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
        
        interval = (double)preset.interval;
        min = (double)preset.hits;
        
        int length = (int)interval;
        int beats = (int)min;
        
        modifierEuclideanRhythm = genEuclideanRhythm(length, beats);
    }
    modifierPresetMenu.setSelectedId(id);
}

std::string Modifier::toolTip() {
    std::string str;
    
    std::string sampleName = modifierSelect.getText().toStdString();
    if(sampleName.empty()) sampleName = "<not selected>";
    std::string parameterName = modifierFunction.getText().toStdString();
    if(parameterName.empty()) parameterName = "<not selected>";
    
    if(mode == MODE_RANDOM) {
        str = "This modifier changes the sample " + sampleName + "'s " + parameterName + " to a random value between " + std::to_string(min) + " and " + std::to_string(max) + " which is divisible by " + std::to_string(step) + ", every " + std::to_string(interval) + " beats.";
    }
    
    if(mode == MODE_EUCLIDEAN) {
        str = "This modifier uses a Euclidean algorithm to generate a rhythm for sample " + sampleName + " of length " + std::to_string((int)interval) + " , containing " + std::to_string((int)min) + " pulses and " + std::to_string((int)interval-(int)min) + " breaks. Each pulse or break lasts " + std::to_string(step) + " beats.";
    }
    
    return str;
}
