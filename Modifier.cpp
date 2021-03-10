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

Modifier::Modifier(std::vector<Sample*> *samplesPointer) {
    samples = samplesPointer;
    
    modifierFunction.addItem("Interval", 1);
    modifierFunction.addItem("Delay", 2);
    modifierFunction.addItem("BPM", 3);
    updateDropdown();
    
    modifierFunctionLabel.setText("Parameter to modify:", juce::dontSendNotification);
    modifierFunction.onChange = [this] { getParams(); };
    modifierSelectLabel.setText("Sample:", juce::dontSendNotification);
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
    modifierMax.setRange(0.25, 10, 0.25);
    modifierMax.setValue(max, juce::dontSendNotification);
    modifierMax.onValueChange = [this] { getParams(); };

    modifierStepLabel.setText("Value step", juce::dontSendNotification);
    modifierStep.setRange(0.25, 10, 0.25);
    modifierStep.setValue(step, juce::dontSendNotification);
    modifierStep.onValueChange = [this] { getParams(); };
    
    modifierChangeMode.setButtonText("Random");
    modifierChangeMode.onClick = [this] { changeMode(); };
    
    modifierEquation.setText(equation, juce::dontSendNotification);
    modifierEquation.onTextChange = [this] { getParams(); };
    modifierEquationLabel.setText("Equation", juce::dontSendNotification);
    
    populatePresets();
    modifierPresetMenu.onChange = [this] { selectPreset(); };
    modifierPresetLabel.setText("Preset", juce::dontSendNotification);
    
    addChildComponent(modifierEquation);
    addChildComponent(modifierEquationLabel);
    
    addChildComponent(modifierPresetLabel);
    addChildComponent(modifierPresetMenu);
    
    addAndMakeVisible(modifierSelect);
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
    addAndMakeVisible(modifierPosition);
    
}

Modifier::~Modifier() {
    
}

void Modifier::changeMode() {
    
    updateDropdown(); // put this here cus dont know where else to...
    
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
    //std::cout << numLeft << "x[";
    for(int i = 0; i < vecLeft.size(); i++) {
        //if(vecLeft.at(i)) std::cout << "1";
        //else std::cout << "0";
    }
    //std::cout << "], " << numRight << "x[";
    for(int i = 0; i < vecRight.size(); i++) {
        //if(vecRight.at(i)) std::cout << "1";
        //else std::cout << "0";
    }
    //std::cout << "]" << std::endl;
    
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

std::vector<bool> Modifier::genEuclideanRhythm(int length, int pulses) {
    int breaks = length - pulses;
    
    std::vector<bool> vecLeft;
    vecLeft.push_back(1);
    
    std::vector<bool> vecRight;
    vecRight.push_back(0);
    
    int numLeft = pulses;
    int numRight = breaks;
    euclideanRhythm(numLeft, numRight, vecLeft, vecRight);
    
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
    //std::cout << std::endl;
    for(int i = 0; i < output.size(); i++) {
        //if(output.at(i)) std::cout << "1";
        //else std::cout << "0";
    }
    //std::cout << std::endl;
    
    return output;
}

void Modifier::updateDropdown() {
    int id = modifierSelect.getSelectedId();
    modifierSelect.clear(juce::dontSendNotification);
    for(int i = 0; i < samples->size(); i++) {
        const juce::String label = samples->at(i)->getLabel();
        modifierSelect.addItem(label, i+1);
    }
    modifierSelect.setSelectedId(id, juce::dontSendNotification);
}

void Modifier::paint(juce::Graphics& g) {
    //g.setColour(juce::Colour(100, 100, 100));
    g.setColour(background);
    
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 20.f);
    //updateDropdown(); // this shouldn't really be called from the graphics thread
    
    // update position
    modifierPosition.setText(std::to_string(euclideanPosition), juce::dontSendNotification);
    
    // make sliders reflect true values
    modifierInterval.setValue(interval, juce::dontSendNotification);
    modifierMin.setValue(min, juce::dontSendNotification);
    modifierMax.setValue(max, juce::dontSendNotification);
    modifierStep.setValue(step, juce::dontSendNotification);
    
    modifierSelect.setSelectedId(sampleIndex+1, juce::dontSendNotification);
    modifierFunction.setSelectedId(functionIndex, juce::dontSendNotification);
    
    if(mode == MODE_RANDOM) {
        
        
        modifierChangeMode.setButtonText("Random");
        
        modifierMin.setVisible(true);
        modifierMinLabel.setVisible(true);
        modifierMinLabel.setText("Minimum value", juce::dontSendNotification);
        
        modifierMax.setVisible(true);
        modifierMaxLabel.setVisible(true);
        
        modifierStep.setVisible(true);
        modifierStepLabel.setVisible(true);
        modifierStepLabel.setText("Value step", juce::dontSendNotification);
        
        modifierIntervalLabel.setText("Interval (beats)", juce::dontSendNotification);
        modifierFunction.setVisible(true);
        modifierFunctionLabel.setVisible(true);
        modifierEquation.setVisible(false);
        modifierEquationLabel.setVisible(false);
        
        modifierPresetLabel.setVisible(false);
        modifierPresetMenu.setVisible(false);
        
    } else if(mode == MODE_EUCLIDEAN) {
        
        modifierChangeMode.setButtonText("Euclidean");
        
        modifierMin.setVisible(true);
        modifierMinLabel.setVisible(true);
        modifierMinLabel.setText("Hits per cycle", juce::dontSendNotification);
        
        modifierMax.setVisible(false);
        modifierMaxLabel.setVisible(false);
        
        modifierStep.setVisible(true);
        modifierStepLabel.setVisible(true);
        modifierStepLabel.setText("Rhythm pulse length (beats)", juce::dontSendNotification);
        
        modifierIntervalLabel.setText("Euclidean rhythm length", juce::dontSendNotification);
        
        modifierFunction.setVisible(false);
        modifierFunctionLabel.setVisible(false);
        
        modifierEquation.setVisible(false);
        modifierEquationLabel.setVisible(false);
        
        modifierPresetLabel.setVisible(true);
        modifierPresetMenu.setVisible(true);
        
    } else if(mode == MODE_EQUATION) {
       
        modifierChangeMode.setButtonText("Equation");
        
        modifierMin.setVisible(false);
        modifierMinLabel.setVisible(false);
        
        modifierMax.setVisible(false);
        modifierMaxLabel.setVisible(false);
        
        modifierStep.setVisible(false);
        modifierStepLabel.setVisible(false);

        modifierIntervalLabel.setText("Interval", juce::dontSendNotification);
        
        modifierFunction.setVisible(true);
        modifierFunctionLabel.setVisible(true);
        
        modifierEquation.setVisible(true);
        modifierEquationLabel.setVisible(true);
        
        modifierPresetLabel.setVisible(false);
        modifierPresetMenu.setVisible(false);
       
   }
    
    slidersChanged = false;
}


void Modifier::resized() {
    modifierSelectLabel.setBounds(15, 0, 120, 20);
    modifierSelect.setBounds(15, 20, 120, 20);
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
    
    modifierPosition.setBounds(700, 5, 50, 20);
    
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
                sample->setInterval(step); // play immediately?
            } else {
                // don't play
                background = juce::Colour(100, 100, 100);
                juce::MessageManager::callAsync ([this] { repaint(); });
                //std::cout << "0";
                sample->setInterval(interval);
            }
        }
    }
}

void Modifier::tickRandom(long roundedBeat, long prevBeat) {
    if((roundedBeat > prevBeat) && ((roundedBeat % roundedInterval) == 0)) {
        // every interval we do something?
        if(sampleIndex >= 0 & sampleIndex < samples->size()) {
           // valid sample selected
            Sample *sample = samples->at(sampleIndex);
            
            // update interval
            
            
            juce::Random r = juce::Random();
            // get value between min (1) and max (4), step (1)
            int iMin = (int)(min / step); // (1)
            int iMax = (int)(max / step); // (4)
            if(iMax > 0) {
                int n = r.nextInt(iMax) + iMin; // r.nextInt gives between 1-3
                double newVal = (double)n*step;
                
                if(functionIndex == MODIFIER_INTERVAL)
                    sample->setInterval(newVal);
                if(functionIndex == MODIFIER_DELAY)
                    sample->setDelay(newVal);
                if(functionIndex == MODIFIER_BPM)
                    sample->setBpm(newVal*10);
            } // otherwise user has entered parameters wrong and interval is too high or smth
        }
    }
}

void Modifier::tick(long roundedBeat, long prevBeat) {
    if(mode == MODE_EUCLIDEAN) {
        tickEuclidean(roundedBeat, prevBeat);
    } else if(mode == MODE_RANDOM) {
        tickRandom(roundedBeat, prevBeat);
    }
}

void Modifier::getParams() {
    std::cout << "GET PARAMS" << tmp++ << std::endl;
    
    interval = modifierInterval.getValue();
    min = modifierMin.getValue();
    max = modifierMax.getValue();
    step = modifierStep.getValue();
    sampleIndex = modifierSelect.getSelectedId() - 1;
    functionIndex = modifierFunction.getSelectedId();
    
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
    }
    modifierPresetMenu.setSelectedId(0);
}
