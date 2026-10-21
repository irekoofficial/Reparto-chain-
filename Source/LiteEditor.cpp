#include "LiteEditor.h"

namespace {
constexpr int kW = 800, kH = 616, kCell = 104, kGapX = 14, kRowH = 118, kTop = 250, kGroupW = kCell * 2 + 8;
struct Def { const char* id; const char* label; };
struct Group { const char* name; Def k[2]; };
// Solo las 2 perillas principales de cada modulo (lo demas lo fija el preset)
const Group kGroups[9] = {
    { "ENTRADA",     { { "inGain",   "GANANCIA" },   { "hpf",      "LOW CUT" } } },
    { "DE-ESSER",    { { "dsAmount", "CANTIDAD" },   { "dsFreq",   "FRECUENCIA" } } },
    { "COMPRESOR",   { { "cThresh",  "UMBRAL" },     { "cRatio",   "RATIO" } } },
    { "EQ",          { { "eqLow",    "CUERPO" },     { "eqAir",    "AIRE" } } },
    { "SATURACION",  { { "satDrive", "DRIVE" },      { "satMix",   "MEZCLA" } } },
    { "MASTER",      { { "mDrive",   "FUERZA" },     { "mWidth",   "ANCHO" } } },
    { "DELAY",       { { "dlyTime",  "TIEMPO" },     { "dlyMix",   "MEZCLA" } } },
    { "REVERB",      { { "revSize",  "TAMANO" },     { "revMix",   "MEZCLA" } } },
    { "SALIDA",      { { "vOut",     "VOZ" },        { "mOut",     "MASTER" } } },
};
juce::Rectangle<int> groupRect(int g) {
    int col = g % 3, row = g / 3, total = 3 * kGroupW + 2 * kGapX, x0 = (kW - total) / 2;
    return { x0 + col * (kGroupW + kGapX), kTop + row * kRowH, kGroupW, kRowH - 6 };
}
}

RVLiteEditor::RVLiteEditor(RVProcessor& p) : AudioProcessorEditor(&p), proc(p) {
    setLookAndFeel(&look);
    addAndMakeVisible(content);
    content.addAndMakeVisible(back);
    content.addAndMakeVisible(guy);
    guy.level = [this] { return proc.meterLevel.load(); };
    back.setBufferedToImage(true);   // el fondo se dibuja una sola vez
    back.fn = [](juce::Graphics& g) {
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff1a0f2b), 0, 0, juce::Colour(0xff0a0b14), (float)kW, (float)kH, false));
        g.fillRect(0, 0, kW, kH);
        g.setColour(juce::Colour(0xcc100e1a)); g.fillRoundedRectangle(10.f, 236.f, (float)kW - 20.f, (float)kH - 246.f, 16.f);
        g.setColour(RVLook::gold.withAlpha(.35f)); g.drawRoundedRectangle(10.f, 236.f, (float)kW - 20.f, (float)kH - 246.f, 16.f, 1.5f);
        g.setColour(RVLook::gold); g.setFont(juce::FontOptions(13.f, juce::Font::bold)); g.drawText("MODO LITE", 574, 184, 216, 18, juce::Justification::left);
        g.setColour(juce::Colours::white.withAlpha(.55f)); g.setFont(juce::FontOptions(11.5f, juce::Font::plain)); g.drawText("Bajo consumo de CPU  -  VST3", 574, 202, 216, 18, juce::Justification::left);
        for (int i = 0; i < 9; ++i) {
            auto r = groupRect(i).toFloat();
            g.setColour(juce::Colour(0x66000000)); g.fillRoundedRectangle(r, 10.f);
            g.setColour(RVLook::gold.withAlpha(.25f)); g.drawRoundedRectangle(r, 10.f, 1.f);
            g.setColour(RVLook::gold); g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
            g.drawText(kGroups[i].name, (int)r.getX() + 10, (int)r.getY() + 3, (int)r.getWidth() - 20, 14, juce::Justification::left);
        }
    };
    for (int i = 0; i < proc.getNumPrograms(); ++i) presetBox.addItem(proc.getProgramName(i), i + 1);
    presetBox.setSelectedId(proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { proc.setCurrentProgram(presetBox.getSelectedId() - 1); };
    content.addAndMakeVisible(presetBox);
    for (auto* b : { &vocalBtn, &masterBtn }) content.addAndMakeVisible(b);
    vocalAtt = std::make_unique<BA>(proc.apvts, "vocalOn", vocalBtn);
    masterAtt = std::make_unique<BA>(proc.apvts, "masterOn", masterBtn);
    static const int pcts[4] = { 50, 75, 100, 125 };
    for (int i = 0; i < 4; ++i) {
        auto& b = sizeBtn[i]; b.setButtonText(juce::String(pcts[i]) + "%"); b.setClickingTogglesState(true); b.setRadioGroupId(78);
        b.onClick = [this, i] { setScalePct(pcts[i]); }; content.addAndMakeVisible(b);
    }
    for (int g = 0; g < 9; ++g) for (int j = 0; j < 2; ++j) {
        Knob k; k.group = g; const auto& d = kGroups[g].k[j];
        auto* prm = proc.apvts.getParameter(d.id);
        const float span = prm ? prm->getNormalisableRange().end - prm->getNormalisableRange().start : 1.f;
        const int dp = span <= 2.5f ? 2 : (span <= 40.f ? 1 : 0);
        k.s = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
        k.s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, kCell - 6, 16);
        k.s->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        k.s->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        k.s->setColour(juce::Slider::textBoxTextColourId, RVLook::gold);
        k.s->setRotaryParameters(juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
        k.l = std::make_unique<juce::Label>(juce::String(), juce::String(d.label));
        k.l->setJustificationType(juce::Justification::centred); k.l->setFont(juce::FontOptions(10.5f, juce::Font::bold));
        k.l->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(.85f));
        k.a = std::make_unique<SA>(proc.apvts, d.id, *k.s);
        k.s->textFromValueFunction = [dp](double v) { return juce::String(v, dp); }; k.s->updateText();
        content.addAndMakeVisible(*k.s); content.addAndMakeVisible(*k.l);
        knobs.push_back(std::move(k));
    }
    int pct = (int)proc.apvts.state.getProperty("uiScale", 100);
    if (pct != 50 && pct != 75 && pct != 125) pct = 100;
    setScalePct(pct);
}

RVLiteEditor::~RVLiteEditor() { setLookAndFeel(nullptr); }

void RVLiteEditor::setScalePct(int pct) {
    static const int pcts[4] = { 50, 75, 100, 125 };
    scalePct = pct;
    proc.apvts.state.setProperty("uiScale", pct, nullptr);
    for (int i = 0; i < 4; ++i) sizeBtn[i].setToggleState(pcts[i] == pct, juce::dontSendNotification);
    setSize(juce::roundToInt(kW * pct / 100.f), juce::roundToInt(kH * pct / 100.f));
}

void RVLiteEditor::resized() {
    content.setTransform(juce::AffineTransform::scale((float)getWidth() / (float)kW));
    content.setBounds(0, 0, kW, kH);
    back.setBounds(0, 0, kW, kH);
    guy.setBounds(10, 8, LiteScene::kSW, LiteScene::kSH);
    presetBox.setBounds(574, 10, 216, 28);
    vocalBtn.setBounds(574, 46, 216, 26); masterBtn.setBounds(574, 78, 216, 26);
    for (int i = 0; i < 4; ++i) sizeBtn[i].setBounds(574 + (i % 2) * 110, 114 + (i / 2) * 32, 106, 26);
    for (int g = 0; g < 9; ++g) {
        auto r = groupRect(g);
        for (int j = 0; j < 2; ++j) {
            auto& k = knobs[(size_t)(g * 2 + j)];
            int kx = r.getX() + 4 + j * kCell;
            k.l->setBounds(kx, r.getY() + 16, kCell, 14);
            k.s->setBounds(kx + 4, r.getY() + 30, kCell - 8, 80);
        }
    }
}
