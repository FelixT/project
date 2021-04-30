#include "Parameter.h"

Parameter::Parameter(std::string label, std::string tooltip, double min, double max, double step, double value)
: label(label), tooltip(tooltip), min(min), max(max), step(step), value(value) {
    
    componentLabel.setJustificationType(juce::Justification::right);
    
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


void Parameter::update() {
    componentSlider.setValue(value, juce::dontSendNotification); // don't bother sending a notification to the slider's onValueChange
}

void Parameter::updateAll() {
    componentLabel.setTooltip(tooltip);
    componentSlider.setRange(min, max, step);
    componentSlider.setValue(value, juce::dontSendNotification);
    componentSlider.setTooltip(tooltip);
    componentLabel.setText(label, juce::dontSendNotification);
}

void Parameter::resized() {
    int width = getLocalBounds().getWidth();
    int height = 40;
    
    componentLabel.setBounds(0, 0, width, 20);
    componentSlider.setBounds(0, 20, width, 20);
    
    componentSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, width/5, 20);
}

bool Parameter::isActive() {
    return active;
}
