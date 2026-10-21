#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "HavanaScene.h"
#include "KnobLook.h"

class RVEditor : public juce::AudioProcessorEditor {
public:
    explicit RVEditor(RVProcessor&);
    ~RVEditor() override;
    void resized() override;
private:
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    struct Knob { std::unique_ptr<juce::Slider> s; std::unique_ptr<juce::Label> l; std::unique_ptr<SA> a; int section = 0; };
    struct Group { juce::Rectangle<int> r; juce::String name; };
    struct Layer : juce::Component { std::function<void(juce::Graphics&)> fn; Layer() { setInterceptsMouseClicks(false, false); } void paint(juce::Graphics& g) override { if (fn) fn(g); } };

    SceneState readState() const;
    float raw(const char* id) const { return proc.apvts.getRawParameterValue(id)->load(); }
    void showTab(int t);
    void setScalePct(int pct);

    RVProcessor& proc;
    RVLook look;
    struct Content : juce::Component {} content;   // todo vive aqui dentro y se escala (declarado antes que sus hijos)
    HavanaScene scene;
    Layer panels;
    juce::ComboBox presetBox;
    juce::ToggleButton vocalBtn{ "CADENA VOZ" }, masterBtn{ "CADENA MASTER" };
    juce::TextButton tabVoz{ "VOZ" }, tabMaster{ "MASTER" };
    std::unique_ptr<BA> vocalAtt, masterAtt;
    std::vector<Knob> knobs;
    std::vector<Group> groups;
    int tab = 0, scalePct = 100;
    juce::TextButton sizeBtn[4];
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RVEditor)
};
