#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Dsp.h"
#include "Presets.h"

class RVProcessor : public juce::AudioProcessor {
public:
    RVProcessor();
    #ifdef RV_LITE
    const juce::String getName() const override { return "Reparto Chain Lite by Ireko"; }
#else
    const juce::String getName() const override { return "Reparto Chain by Ireko"; }
#endif
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& l) const override {
        return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
            && l.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
    }
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }
    int getNumPrograms() override { return (int)rvPresets().size(); }
    int getCurrentProgram() override { return current; }
    void setCurrentProgram(int i) override;
    const juce::String getProgramName(int i) override { return rvPresets()[(size_t)i].name; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> meterLevel{ 0.f };   // nivel de salida para animar la escena
    std::atomic<float> meter{ 0.f };
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
    rv::Engine engine;
    int current = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RVProcessor)
};
