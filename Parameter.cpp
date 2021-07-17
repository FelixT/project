#include "Parameter.h"

Parameter::Parameter(std::string label, std::string tooltip, double min, double max, double step, double value)
: label(label), tooltip(tooltip), min(min), max(max), step(step), value(value) {
    
    // TODO: make opaque
    
    componentLabel.setJustificationType(juce::Justification::left);
    componentLabel.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.f, juce::Font::plain));
    componentSlider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, juce::Colour(4, 40, 40));
    componentSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colour(110, 110, 110));
    componentSlider.setColour(juce::Slider::ColourIds::textBoxTextColourId, juce::Colour(225, 225, 225));
    
    updateAll();
    
    componentSlider.onValueChange = [this] {
        this->value = componentSlider.getValue();
        this->changed = true;
    };
    
    addAndMakeVisible(componentLabel);
    addAndMakeVisible(componentSlider);
}

// TODO: check things  get repainted if these change
void Parameter::setLabel(std::string label) {
    this->label = label;
}

void Parameter::setMin(double min) {
    this->min = min;
}

void Parameter::setMax(double max) {
    this->max = max;
}

void Parameter::setStep(double step) {
    this->step = step;
}

void Parameter::setValue(double value) {
    if(value >= min && value <= max) {
        this->value = value;
        changed = true;
    }
}

void Parameter::setTooltip(std::string tooltip) {
    this->tooltip = tooltip;
}

bool Parameter::isChanged() {
    bool c = changed;
    changed = false;
    return c;
}

void Parameter::setActive(bool v) {
    active = v;
    setVisible(v);
    setEnabled(v);
}

double Parameter::getValue() {
    return value;
}

std::string Parameter::getStrValue() {
    return std::to_string(value);
}

double Parameter::getMin() {
    return min;
}

double Parameter::getMax() {
    return max;
}

double Parameter::getStep() {
    return step;
}

std::string Parameter::getLabel() {
    return label;
}

void Parameter::highlight(int frameInterval, int length) {
    highlighted = true;
    highlightPos = 0;
    
    highlightLen = length;
    
    repaint();
    
    startTimer(20); // how long each 'frame' of the highlight animation lasts
}

void Parameter::paint (juce::Graphics& g) {
    // highlight animation
    
    if(highlighted) {
        g.setColour(juce::Colours::whitesmoke);
        g.setOpacity(((highlightLen-highlightPos)/(float)highlightLen)*0.75f);
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.f, 6.f);
    }
}

void Parameter::timerCallback() {
    if(highlighted) {
        if(highlightPos == highlightLen) {
            highlighted = false;
            stopTimer();
        }
        
        repaint();
        highlightPos++;
    }
}

void Parameter::update() {
    componentSlider.setValue(value, juce::dontSendNotification); // don't bother sending a notification to the slider's onValueChange
}

void Parameter::updateAll() {
    componentLabel.setTooltip(tooltip);
    componentSlider.setRange(min, max, step);
    componentSlider.setValue(value, juce::dontSendNotification);
    componentSlider.setTooltip(tooltip);
    componentLabel.setText(label + ":", juce::dontSendNotification);
}

void Parameter::resized() {
    int width = getLocalBounds().getWidth();
    int height = 34;
    
    componentLabel.setBounds(0, 0, width, 14);
    componentSlider.setBounds(0, 14, width, 20);
    
    int boxWidth = width/5;
    if(boxWidth > 100) boxWidth = 100;
    componentSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, boxWidth, 17);
}

bool Parameter::isActive() {
    return active;
}

void Parameter::mouseDown(const juce::MouseEvent &event) {
    if(event.mods.isRightButtonDown()) {
        // show right click menu, displaying list of modifiers which change the parameter's value        
        
        juce::PopupMenu m;
        m.addSectionHeader("Modified by:");
        
        for(int i = 0; i < modifiers.size(); i++) {
            m.addItem("Modifier " + std::to_string(modifiers.at(i)), true, false, ([this]{ /*do something: would be nice to highlight the modifier on click*/ }));
        }
        
        m.show();
    }
}

void Parameter::addModifier(int index) {
    // tells parameter to add a modifier to its right click menu
    
    modifiers.push_back(index);
}

void Parameter::removeModifier(int index) {
    // tells parameter to remove a modifier from its right click menu
    
    // first off we find it
    for(int i = 0; i < modifiers.size(); i++) {
        if(modifiers.at(i) == index) {
            modifiers.erase(modifiers.begin() + i);
        }
    }
}
