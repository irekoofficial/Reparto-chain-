#include "PluginProcessor.h"
#ifdef RV_LITE
#include "LiteEditor.h"
#else
#include "PluginEditor.h"
#endif

static juce::NormalisableRange<float> makeRange(float mn, float mx, float sk) {
    juce::NormalisableRange<float> r(mn, mx);
    if (sk != 1.f) r.setSkewForCentre(mn + (mx - mn) * 0.25f);
    return r;
}

juce::AudioProcessorValueTreeState::ParameterLayout RVProcessor::makeLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> v;
#define X(id,n,mn,mx,df,sk,sec) v.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{#id, 1}, n, makeRange(mn, mx, sk), df));
    RV_PARAMS(X)
#undef X
#define X(id,n,df) v.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{#id, 1}, n, df > 0.5f));
    RV_TOGGLES(X)
#undef X
    return { v.begin(), v.end() };
}

RVProcessor::RVProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", makeLayout()) {
#ifdef RV_LITE
    engine.lite = true;   // motor ligero: menos CPU
#endif
}

void RVProcessor::prepareToPlay(double sr, int) {
    engine.prepare(sr);
    setLatencySamples(engine.latency());
}

void RVProcessor::processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;
    if (buf.getNumChannels() < 2) return;
#define X(id,n,mn,mx,df,sk,sec) engine.p.id = apvts.getRawParameterValue(#id)->load();
    RV_PARAMS(X)
#undef X
#define X(id,n,df) engine.p.id = apvts.getRawParameterValue(#id)->load();
    RV_TOGGLES(X)
#undef X
    engine.update();
    engine.process(buf.getWritePointer(0), buf.getWritePointer(1), buf.getNumSamples());
    meterLevel.store(juce::jmax(buf.getMagnitude(0, 0, buf.getNumSamples()), meterLevel.load() * 0.9f));
    float pk = juce::jmax(buf.getMagnitude(0, 0, buf.getNumSamples()), buf.getMagnitude(1, 0, buf.getNumSamples()));
    meter.store(juce::jmax(pk, meter.load() * 0.85f));
}

void RVProcessor::setCurrentProgram(int idx) {
    if (idx < 0 || idx >= getNumPrograms()) return;
    current = idx;
    Params p; rvApplyPreset(p, idx);
    auto set = [&](const char* id, float v) {
        if (auto* prm = apvts.getParameter(id)) prm->setValueNotifyingHost(prm->convertTo0to1(v));
    };
#define X(id,n,mn,mx,df,sk,sec) set(#id, p.id);
    RV_PARAMS(X)
#undef X
#define X(id,n,df) set(#id, p.id);
    RV_TOGGLES(X)
#undef X
}

void RVProcessor::getStateInformation(juce::MemoryBlock& dest) {
    auto state = apvts.copyState();
    state.setProperty("program", current, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary(*xml, dest);
}

void RVProcessor::setStateInformation(const void* data, int size) {
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(apvts.state.getType())) {
            auto t = juce::ValueTree::fromXml(*xml);
            current = (int)t.getProperty("program", 0);
            apvts.replaceState(t);
        }
}

#ifdef RV_LITE
juce::AudioProcessorEditor* RVProcessor::createEditor() { return new RVLiteEditor(*this); }
#else
juce::AudioProcessorEditor* RVProcessor::createEditor() { return new RVEditor(*this); }
#endif

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RVProcessor(); }
