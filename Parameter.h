#ifndef Parameter_hpp
#define Parameter_hpp

#include <string>
#include <JuceHeader.h>

class Parameter : public juce::Component {
public:
    Parameter(std::string label, std::string tooltip, double min, double max, double step, double value);
    void setLabel(std::string label);
    void setMin(double min);
    void setMax(double max);
    void setStep(double step);
    void setValue(double value);
    void setTooltip(std::string tooltip);
    double getValue();
    void update();
    void updateAll();
    void resized() override;
private:
    std::string label;
    std::string tooltip;
    double min;
    double max;
    double step;
    double value;
    
    juce::Label componentLabel;
    juce::Slider componentSlider;
};

#endif
