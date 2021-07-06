#include "PatternView.h"

PatternView::PatternView(std::vector<bool> *pattern, int *position) {
    setOpaque(true);
    
    this->pattern = pattern;
    this->position = position;
}

PatternView::~PatternView() {
    
}

void PatternView::setEditable(bool val) {
    editable = val;
}

void PatternView::paint(juce::Graphics& g) {

    g.fillAll(juce::Colour(100, 100, 100));
    
    if(pattern->size() >= 0) {
        int x = 0;
        int y = 0;
        
        for(int i = 0; i < pattern->size(); i++) {
            if(pattern->at(i)) g.setColour(juce::Colours::green);
            else g.setColour(juce::Colour(255, 100, 100));
            
            g.fillRoundedRectangle(x+1, y+1, 20-2, 20-2, 7.f);
            
            
            if(i == *position) {
                g.setColour(juce::Colours::black);
                g.drawRoundedRectangle(x+1, y+1, 20-2, 20-2, 7.f, 2.f);
            } else {
                g.setColour(juce::Colours::white);
                g.drawRoundedRectangle(x+1, y+1, 20-2, 20-2, 7.f, 1.f);
            }
            
            x+=20;
        }
    }
}

int PatternView::patternWidth() {
    return pattern->size() * 20;
}

void PatternView::mouseDown(const juce::MouseEvent &event) {
    if(editable) {
        // if allowed to modify i.e. if in custom mode
        
        // work out which element was clicked on
        int x = event.getMouseDownX();
        int component = x/20;
        
        // then toggle this and redraw
        pattern->at(component) = !pattern->at(component);
        repaint();
    }
}
