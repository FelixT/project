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
    
    modifierIntervalLabel.setText("Interval", juce::dontSendNotification);
    modifierInterval.setRange(1, 32, 1);
    modifierInterval.setValue(16);
    modifierMinLabel.setText("Minimum value", juce::dontSendNotification);
    modifierMin.setRange(0.25, 10, 0.25);
    modifierMin.setValue(2);
    modifierMaxLabel.setText("Maximum value", juce::dontSendNotification);
    modifierMax.setRange(0.25, 10, 0.25);
    modifierMax.setValue(4);
    modifierStepLabel.setText("Value step", juce::dontSendNotification);
    modifierStep.setRange(0.25, 10, 0.25);
    
    modifierEuclidean.setButtonText("Euclidean rhythms?");
    modifierUpdateEuclidean.setButtonText("Update euclidean params");
    modifierUpdateEuclidean.onClick = [this] {
        int length = (int)modifierInterval.getValue();
        int beats = (int)modifierMin.getValue();
        
        modifierEuclideanRhythm = genEuclideanRhythm(length, beats);
        std::cout << "terminated" << std::endl;
        
        // get current sample
        int sampleIndex = modifierSelect.getSelectedId() - 1;
        if(sampleIndex >= 0 & sampleIndex < samples->size()) {
           // valid sample selected
            Sample *sample = samples->at(sampleIndex);
            // this is v buggy...
            const juce::MessageManagerLock mmLock;
            sample->setInterval(modifierInterval.getValue());
        }
    };
    
    addAndMakeVisible(modifierSelect);
    addAndMakeVisible(modifierFunction);
    addAndMakeVisible(modifierInterval);
    addAndMakeVisible(modifierMin);
    addAndMakeVisible(modifierMax);
    addAndMakeVisible(modifierStep);
    addAndMakeVisible(modifierEuclidean);
    addAndMakeVisible(modifierIntervalLabel);
    addAndMakeVisible(modifierMaxLabel);
    addAndMakeVisible(modifierMinLabel);
    addAndMakeVisible(modifierStepLabel);
    addAndMakeVisible(modifierUpdateEuclidean);
}

Modifier::~Modifier() {
    
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
    std::cout << std::endl;
    for(int i = 0; i < output.size(); i++) {
        if(output.at(i)) std::cout << "1";
        else std::cout << "0";
    }
    std::cout << std::endl;
    
    return output;
}

void Modifier::updateDropdown() {
    modifierSelect.clear();
    for(int i = 0; i < samples->size(); i++) {
        const juce::String label = samples->at(i)->getLabel();
        modifierSelect.addItem(label, i+1);
    }
}

void Modifier::paint(juce::Graphics& g) {
    g.setColour(juce::Colour(100, 100, 100));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 20.f);
}


void Modifier::resized() {
    updateDropdown();
    
    modifierSelect.setBounds(0, 0, 120, 20);
    modifierFunction.setBounds(0, 40, 120, 20);
    
    modifierIntervalLabel.setBounds(200, 0, 150, 20);
    modifierInterval.setBounds(200, 20, 150, 20);
    
    modifierMinLabel.setBounds(200, 40, 150, 20);
    modifierMin.setBounds(200, 60, 150, 20);
    modifierMaxLabel.setBounds(350, 40, 150, 20);
    modifierMax.setBounds(350, 60, 150, 20);
    modifierStepLabel.setBounds(550, 40, 150, 20);
    modifierStep.setBounds(550, 60, 150, 20);
    
    modifierEuclidean.setBounds(550, 0, 120, 20);
    modifierUpdateEuclidean.setBounds(700, 0, 50, 40);
    
}

long Modifier::pow10(float input, int power) {
    for(int i = 0; i < power; i++) {
        input*=10.f;
    }
    return (long)input;
}

void Modifier::tick(long roundedBeat, long prevBeat) {
    if(modifierEuclidean.getToggleState() && modifierEuclideanRhythm.size() > 0) {
        
        if((roundedBeat > prevBeat) && ((roundedBeat % roundedStep) == 0)) {
            // every step, work out whether the sample should be playing
            
            euclideanPosition+=1;
            euclideanPosition%=modifierEuclideanRhythm.size();
            if(euclideanPosition == 0) std::cout << std::endl << "NEW" << std::endl;
            
            int sampleIndex = modifierSelect.getSelectedId() - 1;
            if(sampleIndex >= 0 & sampleIndex < samples->size()) {
               // valid sample selected
                Sample *sample = samples->at(sampleIndex);
            
                if(modifierEuclideanRhythm.at(euclideanPosition)) {
                    std::cout << "1";
                    // this is v buggy...
                    const juce::MessageManagerLock mmLock;
                    sample->setInterval(modifierStep.getValue()); // play immediately?
                } else {
                    std::cout << "0";
                    const juce::MessageManagerLock mmLock;
                    sample->setInterval(modifierInterval.getValue());
                    // dunno
                }
            }
        }
    } else {
        if((roundedBeat > prevBeat) && ((roundedBeat % roundedInterval) == 0)) {
            // every interval we do something?
            int sampleIndex = modifierSelect.getSelectedId() - 1;
            if(sampleIndex >= 0 & sampleIndex < samples->size()) {
               // valid sample selected
                Sample *sample = samples->at(sampleIndex);
                
                // update interval
                
                
                juce::Random r = juce::Random();
                // get value between min (1) and max (4), step (1)
                double step = modifierStep.getValue();
                int iMin = (int)(modifierMin.getValue() / step); // (1)
                int iMax = (int)(modifierMax.getValue() / step); // (4)
                if(iMax > 0) {
                    int n = r.nextInt(iMax) + iMin; // r.nextInt gives between 1-3
                    double newVal = (double)n*step;
                    
                    // this is v buggy...
                    const juce::MessageManagerLock mmLock;
                    if(modifierFunction.getSelectedId() == MODIFIER_INTERVAL)
                        sample->setInterval(newVal);
                    if(modifierFunction.getSelectedId() == MODIFIER_DELAY)
                        sample->setDelay(newVal);
                    if(modifierFunction.getSelectedId() == MODIFIER_BPM)
                        sample->setBpm(newVal*10);
                } // otherwise user has entered parameters wrong and interval is too high or smth
            }
        }
        
    }
}

void Modifier::updateParams(int precision) {
    roundedInterval = pow10((float)modifierInterval.getValue(), precision);
    roundedStep = pow10((float)modifierStep.getValue(), precision);
}
