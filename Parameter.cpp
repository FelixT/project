#include "Parameter.h"

Parameter::Parameter(std::string label, double min, double max, double step, double value)
: label(label), min(min), max(max), step(step), value(value) {
    
    componentLabel.setJustificationType(juce::Justification::right);
    componentSlider.setRange(min, max, step);
    componentSlider.setValue(value);
    componentLabel.setText(label, juce::dontSendNotification);
    addAndMakeVisible(componentLabel);
    addAndMakeVisible(componentSlider);
    
    componentSlider.onValueChange = [this] {
        this->value = componentSlider.getValue();
    };
    
}

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
    this->value = value;
}

void Parameter::updateGraphics() {
    componentSlider.setRange(min, max, step);
    componentSlider.setValue(value);
    componentLabel.setText(label, juce::dontSendNotification);
}

void Parameter::resized() {
    int width = getLocalBounds().getWidth();
    int height = 40;
    
    componentLabel.setBounds(0, 0, width, 20);
    componentSlider.setBounds(0, 20, width, 20);
    
    componentSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, width/5, 20);
}
