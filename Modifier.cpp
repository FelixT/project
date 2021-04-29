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

Modifier::Modifier(std::vector<Sample*> *samplesPointer, std::vector<Modifier*> *modifiersPointer)
: interval("Interval (beats)", "", 1.0, 32.0, 1.0, 16.0),
min("Minimum value", "Lowest possible value which the parameter could be assigned", 0.25, 10.0, 0.25, 2.0),
max("Maximum value", "Highest possible value which the parameter could be assigned", 0.25, 10.0, 0.25, 4.0),
step("Value step", "Constrains output to be a multiple of this", 0.25, 10.0, 0.25, 0.25)
{
    setOpaque(true); // ??
    
    samples = samplesPointer;
    modifiers = modifiersPointer;
    
    // select sample/modifier
    modifierSelectLabel.setText("Object to modify:", juce::dontSendNotification);
    modifierSelectLabel.setTooltip("Select a sample or even another modifier to have its parameters automatically changed by this modifier");
    modifierSelect.onChange = [this] { getParams(); };
    
    // parameter
    modifierParameter.addItem("Interval", 1);
    modifierParameter.addItem("Delay", 2);
    modifierParameter.addItem("BPM", 3);
    modifierParameterLabel.setText("Parameter to modify:", juce::dontSendNotification);
    modifierParameter.onChange = [this] { getParams(); };
   
    // presets
    populatePresets();
    modifierPresetMenu.onChange = [this] { selectPreset(); };
    modifierPresetLabel.setText("Preset", juce::dontSendNotification);
    
    // mode
    modifierChangeMode.setButtonText("Random");
    modifierChangeMode.onClick = [this] { changeMode(); };
    modifierChangeMode.setTooltip("Change modifier mode");
    
    // help
    modifierHelp.setButtonText("?");
    modifierHelp.onClick = [this] {
        // TODO: change this to be a tool tip
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::InfoIcon, "Modifier Help", toolTip());
    };
    modifierHelp.setTooltip("What does this do?");
    
    // euclidean position
    modifierBack.setButtonText("<");
    modifierBack.onClick = [this] { euclideanPosition-=1; euclideanPosition%=modifierEuclideanRhythm.size(); };
    modifierBack.setTooltip("Change pattern position");
    
    modifierPosition.setJustificationType(juce::Justification::centred);
    
    modifierForward.setButtonText(">");
    modifierForward.onClick = [this] { euclideanPosition+=1; euclideanPosition%=modifierEuclideanRhythm.size(); };
    modifierBack.setTooltip("Change pattern position");
    
    // equation
    modifierEquation.setText(equation, juce::dontSendNotification);
    modifierEquation.onTextChange = [this] { getParams(); };
    modifierEquationLabel.setText("Equation", juce::dontSendNotification);
    
    modifierRefresh.setButtonText("Refresh");
    modifierRefresh.onClick = [this] { refreshDropdownItems(); };
    modifierRefresh.setTooltip("Refresh the dropdown items");
    
    addChildComponent(modifierEquation);
    addChildComponent(modifierEquationLabel);
    
    addChildComponent(modifierPresetLabel);
    addChildComponent(modifierPresetMenu);
    
    addChildComponent(modifierBack);
    addChildComponent(modifierForward);
    addChildComponent(modifierPosition);
    
    addAndMakeVisible(modifierSelect);
    addAndMakeVisible(modifierRefresh);
    addAndMakeVisible(modifierParameter);
    addAndMakeVisible(modifierChangeMode);
    addAndMakeVisible(modifierParameterLabel);
    addAndMakeVisible(modifierSelectLabel);
    addAndMakeVisible(modifierHelp);

    addAndMakeVisible(interval);
    addAndMakeVisible(min);
    addAndMakeVisible(max);
    addAndMakeVisible(step);
    
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
    std::cout << "refreshing the drop down" << std::endl;
    
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

void Modifier::showRandomControls() {
    modifierChangeMode.setButtonText("Random");
    
    min.setVisible(true);
    min.setLabel("Minimum value");
    min.setTooltip("Lowest possible value which the parameter could be assigned");
    min.updateAll();
            
    max.setVisible(true);
    
    step.setVisible(true);
    step.setLabel("Value step");
    step.updateAll();
    
    interval.setLabel("Interval (beats)");
    interval.setTooltip("How often (in beats) the modifier should fire");
    
    modifierParameter.setVisible(true);
    modifierParameterLabel.setVisible(true);
    modifierEquation.setVisible(false);
    modifierEquationLabel.setVisible(false);
    
    modifierPresetLabel.setVisible(false);
    modifierPresetMenu.setVisible(false);
    
    modifierPosition.setVisible(false);
    
    modifierBack.setVisible(false);
    modifierForward.setVisible(false);
}

void Modifier::showEuclideanControls() {
    modifierChangeMode.setButtonText("Euclidean");
    
    min.setVisible(true);
    min.setLabel("Hits per cycle");
    min.setTooltip("How many 'hits' (i.e. times the sample will be played) per cycle");
    min.updateAll();
    
    max.setVisible(false);
    
    step.setVisible(true);
    
    step.setVisible(true);
    step.setLabel("Rhythm pulse length (beats)");
    step.setTooltip("How long each 'pulse' ('hit' or 'break') should be (in beats)");
    step.updateAll();
    
    interval.setLabel("Euclidean rhythm length (beats)");
    interval.setTooltip("How long the Euclidean rhythm cycle should be (in beats)");
    interval.updateAll();
    
    modifierParameter.setVisible(false);
    modifierParameterLabel.setVisible(false);
    
    modifierEquation.setVisible(false);
    modifierEquationLabel.setVisible(false);
    
    modifierPresetLabel.setVisible(true);
    modifierPresetMenu.setVisible(true);
    
    modifierPosition.setVisible(true);
    
    modifierBack.setVisible(true);
    modifierForward.setVisible(true);
}

void Modifier::drawEuclideanPattern(juce::Graphics& g) {
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
}

void Modifier::showEquationControls() {
    modifierChangeMode.setButtonText("Equation");
    
    min.setVisible(false);
    
    max.setVisible(false);
    
    step.setVisible(true);
    step.setLabel("Rhythm pulse length (beats)");
    step.updateAll();
    
    interval.setLabel("Interval");
    interval.updateAll();
    
    modifierParameter.setVisible(true);
    modifierParameterLabel.setVisible(true);
    
    modifierEquation.setVisible(true);
    modifierEquationLabel.setVisible(true);
    
    modifierPresetLabel.setVisible(false);
    modifierPresetMenu.setVisible(false);
    
    modifierPosition.setVisible(true);
    
    modifierBack.setVisible(false);
    modifierForward.setVisible(false);
}

void Modifier::paint(juce::Graphics& g) {
        
    // TODO: implement slidersChanged
    
    if(slidersChanged) {
        // make sliders reflect true values
        interval.update();
        min.update();
        max.update();
        step.update();
        slidersChanged = false;
    }
    
    // draw background
    g.setColour(background);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.f);
    
    // update position
    modifierPosition.setText(std::to_string(euclideanPosition), juce::dontSendNotification);
    
    // refresh dropdown items
    refreshDropdownItems();
    
    // update which dropdown item is selected
    if(state == STATE_IDLE)
        modifierSelect.setSelectedId(1, juce::dontSendNotification);
    if(state == STATE_SAMPLE)
        modifierSelect.setSelectedId(selected+2, juce::dontSendNotification);
    if(state == STATE_MODIFIER)
        modifierSelect.setSelectedId(selected+(int)samples->size()+2, juce::dontSendNotification);
    
    modifierParameter.setSelectedId(parameter, juce::dontSendNotification);
    
    if(mode == MODE_RANDOM) {
        showRandomControls();
    } else if(mode == MODE_EUCLIDEAN) {
        showEuclideanControls();
        drawEuclideanPattern(g);
    } else if(mode == MODE_EQUATION) {
        showEquationControls();
   }
}


void Modifier::resized() {
    modifierSelectLabel.setBounds(15, 0, 120, 20);
    modifierSelect.setBounds(15, 20, 120, 20);
    
    modifierRefresh.setBounds(135, 20, 40, 20);
    
    modifierParameterLabel.setBounds(15, 40, 120, 20);
    modifierParameter.setBounds(15, 60, 120, 20);
    
    interval.setBounds(200, 0, 150, 40);
    min.setBounds(200, 40, 150, 40);
    max.setBounds(350, 40, 150, 40);
    step.setBounds(550, 40, 150, 40);
    
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
    interval.setValue(val);
    slidersChanged = true;
}

void Modifier::setMin(double val) {
    min.setValue(val);
    slidersChanged = true;
}

void Modifier::setMax(double val) {
    max.setValue(val);
    slidersChanged = true;
}

void Modifier::setStep(double val) {
    step.setValue(val);
    slidersChanged = true;
}

void Modifier::setSelected(int index) {
    selected = index;
    slidersChanged = true;
}

void Modifier::setMode(int index) {
    mode = (modifierMode)index;
    slidersChanged = true;
}

void Modifier::setState(int index) {
    state = (modifierState)index;
    slidersChanged = true;
}

void Modifier::setParameter(int index) {
    parameter = (enum modifierParameter)index;
    slidersChanged = true;
}

void Modifier::setEquation(std::string eqn) {
    equation = eqn;
    slidersChanged = true;
}

long Modifier::pow10(float input, int power) {
    for(int i = 0; i < power; i++) {
        input*=10.f;
    }
    return (long)input;
}

void Modifier::tickEuclidean(long roundedBeat, long prevBeat) {
    if(state != STATE_SAMPLE) return;
    
    if(modifierEuclideanRhythm.size() == 0) return;
    
    if((roundedBeat > prevBeat) && ((roundedBeat % roundedStep) == 0)) {
        // every step, work out whether the sample should be playing
        
        euclideanPosition+=1;
        euclideanPosition%=modifierEuclideanRhythm.size();
        
        if(selected >= 0 & selected < samples->size()) {
           // valid sample selected
            Sample *sample = samples->at(selected);
        
            if(modifierEuclideanRhythm.at(euclideanPosition)) {
                // play
                background = juce::Colour(50, 100, 50);
                juce::MessageManager::callAsync ([this] { repaint(); });

                
                sample->setInterval(step.getValue()); // play immediately?
            } else {
                // don't play
                background = juce::Colour(100, 100, 100);
                juce::MessageManager::callAsync ([this] { repaint(); });
                
                // disable the sample from starting for an interval of 'step'
                sample->setInterval(step.getValue());
                sample->disable();

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
        if(state == STATE_SAMPLE && ((selected < 0) || (selected >= samples->size()))) return;
        if(state == STATE_MODIFIER && ((selected < 0) || (selected >= modifiers->size()))) return;
        
        juce::Random r = juce::Random();
        
        int iMin = (int)std::ceil(min.getValue() / step.getValue());
        int iMax = (int)std::floor(max.getValue() / step.getValue());
        int iRange = iMax - iMin;
        
        if(iMax > 0 && iRange > 0) {
            int n = r.nextInt(iRange + 1) + iMin;
            double newVal = (double)n*step.getValue();
            
            if(state == STATE_SAMPLE) {
                Sample *sample = samples->at(selected);

                if(parameter == PARAMETER_INTERVAL)
                    sample->setInterval(newVal);
                if(parameter == PARAMETER_DELAY)
                    sample->setDelay(newVal);
                if(parameter == PARAMETER_BPM)
                    sample->setBpm(newVal*10);
            } else {
                Modifier *modifier = modifiers->at(selected);

                if(parameter == PARAMETER_INTERVAL)
                    modifier->setInterval(newVal);
            }
        } // otherwise user has entered parameters wrong
    }
}

void Modifier::tickEquation(long roundedBeat, long prevBeat) {
    if(state != STATE_SAMPLE) return;
    
    if((roundedBeat > prevBeat) && ((roundedBeat % roundedStep) == 0)) {
        euclideanPosition+=1;
        euclideanPosition%=(int)interval.getValue();
        
        if(selected >= 0 & selected < samples->size()) {
           // valid sample selected
            
            juce::MessageManager::callAsync ([this] { repaint(); });
            Sample *sample = samples->at(selected);
            
            double newVal = parseEquation(modifierEquation.getText().toStdString());
            std::cout << newVal << std::endl;
            
            if(newVal < 0) return;
            
            if(parameter == PARAMETER_INTERVAL)
                sample->setInterval(newVal);
            if(parameter == PARAMETER_DELAY)
                sample->setDelay(newVal);
            if(parameter == PARAMETER_BPM)
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

void Modifier::updateEuclidean() {
    int length = (int)interval.getValue();
    int beats = (int)min.getValue();
    
    modifierEuclideanRhythm = genEuclideanRhythm(length, beats);
    
    // get current sample
    if(selected >= 0 & selected < samples->size()) {
       // valid sample selected
        Sample *sample = samples->at(selected);
        sample->setInterval(interval.getValue());
    }
}

void Modifier::getParams() {
        
    modifierPresetMenu.setSelectedId(0);
    
    int selectedId = modifierSelect.getSelectedId();
    
    parameter = (enum modifierParameter)modifierParameter.getSelectedId();
    
    if(selectedId <= 1) {
        state = STATE_IDLE;
    } else if(selectedId > 1 && selectedId < samples->size() + 2) { // sample
        selected = modifierSelect.getSelectedId() - 2;
        state = STATE_SAMPLE;
    } else { // modifier
        selected = (int)modifierSelect.getSelectedId() - samples->size() - 2;
        state = STATE_MODIFIER;
    }
    
    
    if(mode == MODE_EUCLIDEAN && state == STATE_SAMPLE) {
        updateEuclidean();
        
    } else if(mode == MODE_EQUATION) {
        equation = modifierEquation.getText().toStdString();
        
        // somehow parse this lol
    }
}

void Modifier::updateParams(int precision) {
    if(slidersChanged) juce::MessageManager::callAsync ([this] { repaint(); });
    
    roundedInterval = pow10((float)interval.getValue(), precision);
    roundedStep = pow10((float)step.getValue(), precision);
}

std::string Modifier::toString() {
    std::string output = "";
    output += "Modifier <name> {\n";
    output += "mode " + std::to_string(mode) + "\n";
    output += "state " + std::to_string(state) + "\n";
    output += "parameter " + std::to_string(parameter) + "\n";
    output += "selected " + std::to_string(selected) + "\n";
    output += "interval " + std::to_string(interval.getValue()) + "\n";
    output += "min " + std::to_string(min.getValue()) + "\n";
    output += "max " + std::to_string(max.getValue()) + "\n";
    output += "step " + std::to_string(step.getValue()) + "\n";
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
        
        interval.setValue((double)preset.interval);
        min.setValue((double)preset.hits);
        
        int length = (int)interval.getValue();
        int beats = (int)min.getValue();
        
        modifierEuclideanRhythm = genEuclideanRhythm(length, beats);
    }
    modifierPresetMenu.setSelectedId(id);
}

std::string Modifier::toolTip() {
    std::string str;
    
    std::string sampleName = modifierSelect.getText().toStdString();
    if(sampleName.empty()) sampleName = "<not selected>";
    std::string parameterName = modifierParameter.getText().toStdString();
    if(parameterName.empty()) parameterName = "<not selected>";
    
    if(mode == MODE_RANDOM) {
        str = "This modifier changes the sample " + sampleName + "'s " + parameterName + " to a random value between " + std::to_string(min.getValue()) + " and " + std::to_string(max.getValue()) + " which is divisible by " + std::to_string(step.getValue()) + ", every " + std::to_string(interval.getValue()) + " beats.";
    }
    
    if(mode == MODE_EUCLIDEAN) {
        str = "This modifier uses a Euclidean algorithm to generate a rhythm for sample " + sampleName + " of length " + std::to_string((int)interval.getValue()) + " , containing " + std::to_string((int)min.getValue()) + " pulses and " + std::to_string((int)interval.getValue()-(int)min.getValue()) + " breaks. Each pulse or break lasts " + std::to_string(step.getValue()) + " beats.";
    }
    
    return str;
}
