#ifndef Parameter_hpp
#define Parameter_hpp

#include <string>
#include <JuceHeader.h>
#include <vector>


class Parameter : public juce::Component, private juce::Timer {
public:
    Parameter(std::string label, std::string tooltip, double min, double max, double step, double value);
    void setLabel(std::string label);
    void setMin(double min);
    void setMax(double max);
    void setStep(double step);
    void setValue(double value);
    void setTooltip(std::string tooltip);
    bool isChanged(); // whether this parameter has changed since we last checked
    double getValue();
    double getMin();
    double getMax();
    double getStep();
    void update();
    void updateAll();
    void resized() override;
    bool isActive();
    void setActive(bool v);
    std::string getLabel();
    void highlight(int frameInterval = 20, int length = 8);
    void addModifier(int index);
    void removeModifier(int index);
private:
    void timerCallback() override;
    void mouseDown(const juce::MouseEvent &event) override;
    std::string label;
    std::string tooltip;
    double min;
    double max;
    double step;
    double value;
    bool active = true;
    bool changed = false;
    
    void paint (juce::Graphics& g) override;
    
    bool highlighted = false;
    int highlightPos = 0; // highlight animation position
    int highlightLen = 8; // number of frames of highlight animation
    
    class ParamLabel : public juce::Label {
    private:
        void mouseDown(const juce::MouseEvent &event) override {
            getParentComponent()->getParentComponent()->mouseDown(event); // it works (!)
        }
    };
    
    ParamLabel componentLabel;
    juce::Slider componentSlider;
    
    std::vector<int> modifiers; // list of modifier (indexes) acting on this parameter
};

#endif
