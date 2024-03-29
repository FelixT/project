#ifndef PatternView_hpp
#define PatternView_hpp

#include <JuceHeader.h>

class PatternView : public juce::Component {
public:
    PatternView(std::vector<bool> *pattern, int *position);
    ~PatternView();
    void setEditable(bool val);
    void paint(juce::Graphics& g) override;
    int patternWidth();
private:
    std::vector<bool> *pattern;
    int *position;
    void mouseDown(const juce::MouseEvent &event) override;
    bool editable = true;
};

#endif
