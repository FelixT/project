//
//  Modifier.cpp
//  Proto - App
//
//  Created by Felix Thomas on 01/03/2021.
//

#include "Modifier.h"

Modifier::Modifier() {
    
}

Modifier::~Modifier() {
    
}

void Modifier::paint (juce::Graphics& g) {
    g.setColour(juce::Colour(100, 100, 100));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 20.f);
}


void Modifier::resized() {
    
}
