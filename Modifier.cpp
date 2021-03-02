//
//  Modifier.cpp
//  Proto - App
//
//  Created by Felix Thomas on 01/03/2021.
//

#include "Modifier.h"

Modifier::Modifier(std::vector<Sample*> *samplesPointer) {
    samples = samplesPointer;
    
    modifierFunction.addItem("Interval", 1);
    modifierFunction.addItem("Delay", 2);
    modifierFunction.addItem("BPM", 3);
    
    modifierInterval.setRange(1, 32, 1);
    modifierMin.setRange(0.25, 10, 0.25);
    modifierMax.setRange(0.25, 10, 0.25);
    modifierStep.setRange(0.25, 10, 0.25);
    
    addAndMakeVisible(modifierSelect);
    addAndMakeVisible(modifierFunction);
    addAndMakeVisible(modifierInterval);
    addAndMakeVisible(modifierMin);
    addAndMakeVisible(modifierMax);
    addAndMakeVisible(modifierStep);
}

Modifier::~Modifier() {
    
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
    
    modifierInterval.setBounds(200, 0, 120, 20);
    
    modifierMin.setBounds(200, 40, 120, 20);
    modifierMax.setBounds(350, 40, 120, 20);
    modifierStep.setBounds(550, 40, 120, 20);
    
}

long Modifier::pow10(float input, int power) {
    for(int i = 0; i < power; i++) {
        input*=10.f;
    }
    return (long)input;
}

void Modifier::tick(long roundedBeat, long prevBeat) {
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

void Modifier::updateParams(int precision) {
    roundedInterval = pow10((float)modifierInterval.getValue(), precision);
}
