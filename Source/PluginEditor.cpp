#include "PluginEditor.h"

static constexpr int kW = 1100, kH = 840, kCell = 100, kGap = 16, kRowH = 112, kTop = 484;

RVEditor::RVEditor(RVProcessor& p) : AudioProcessorEditor(&p), proc(p) {
    setLookAndFeel(&look);
    addAndMakeVisible(content);
    content.addAndMakeVisible(scene);
    scene.provider = [this] { return readState(); };
    content.addAndMakeVisible(panels);
    panels.fn = [this](juce::Graphics& g) {
        const juce::Colour gold = RVLook::gold;
        g.setColour(juce::Colour(0xcc100e1a)); g.fillRoundedRectangle(10.f, 444.f, (float)kW - 20.f, (float)kH - 454.f, 16.f);
        g.setColour(gold.withAlpha(.35f)); g.drawRoundedRectangle(10.f, 444.f, (float)kW - 20.f, (float)kH - 454.f, 16.f, 1.5f);
        for (auto& gr : groups) {
            g.setColour(juce::Colour(0x66000000)); g.fillRoundedRectangle(gr.r.toFloat(), 10.f);
            g.setColour(gold.withAlpha(.25f)); g.drawRoundedRectangle(gr.r.toFloat(), 10.f, 1.f);
            g.setColour(gold); g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
            g.drawText(gr.name, gr.r.getX() + 10, gr.r.getY() + 3, gr.r.getWidth() - 20, 14, juce::Justification::left);
        }
    };
    for (int i = 0; i < proc.getNumPrograms(); ++i) presetBox.addItem(proc.getProgramName(i), i + 1);
    presetBox.setSelectedId(proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { proc.setCurrentProgram(presetBox.getSelectedId() - 1); };
    content.addAndMakeVisible(presetBox);
    for (auto* b : { &vocalBtn, &masterBtn }) content.addAndMakeVisible(b);
    vocalAtt = std::make_unique<BA>(proc.apvts, "vocalOn", vocalBtn);
    masterAtt = std::make_unique<BA>(proc.apvts, "masterOn", masterBtn);
    for (auto* b : { &tabVoz, &tabMaster }) { b->setClickingTogglesState(true); b->setRadioGroupId(77); content.addAndMakeVisible(b); }
    tabVoz.onClick = [this] { showTab(0); }; tabMaster.onClick = [this] { showTab(1); };
    tabVoz.setToggleState(true, juce::dontSendNotification);
    static const int pcts[4] = { 25, 50, 75, 100 };
    for (int i = 0; i < 4; ++i) {
        auto& b = sizeBtn[i]; b.setButtonText(juce::String(pcts[i]) + "%"); b.setClickingTogglesState(true); b.setRadioGroupId(78);
        b.onClick = [this, i] { setScalePct(pcts[i]); }; content.addAndMakeVisible(b);
    }
#define X(id,n,mn,mx,df,sk,sec) { Knob k; k.section = sec; k.s = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow); \
        k.s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, kCell - 6, 16); k.s->setNumDecimalPlacesToDisplay((mx - mn) <= 2.5f ? 2 : ((mx - mn) <= 40.f ? 1 : 0)); \
        k.s->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack); k.s->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack); k.s->setColour(juce::Slider::textBoxTextColourId, RVLook::gold); k.s->setRotaryParameters(juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true); \
        k.l = std::make_unique<juce::Label>(juce::String(), juce::String(n).toUpperCase()); k.l->setJustificationType(juce::Justification::centred); k.l->setFont(juce::FontOptions(10.f, juce::Font::bold)); \
        k.l->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(.85f)); \
        k.a = std::make_unique<SA>(proc.apvts, #id, *k.s); { const int dp = (mx - mn) <= 2.5f ? 2 : ((mx - mn) <= 40.f ? 1 : 0); k.s->textFromValueFunction = [dp](double v) { return juce::String(v, dp); }; k.s->updateText(); } content.addAndMakeVisible(*k.s); content.addAndMakeVisible(*k.l); knobs.push_back(std::move(k)); }
    RV_PARAMS(X)
#undef X
    int pct = (int)proc.apvts.state.getProperty("uiScale", 100);
    if (pct != 25 && pct != 50 && pct != 75) pct = 100;
    setScalePct(pct);
    showTab(0);
}

RVEditor::~RVEditor() { setLookAndFeel(nullptr); }

// Lo que haces con las perillas mueve la escena
SceneState RVEditor::readState() const {
    SceneState s;
    auto pos = [](float v) { return juce::jmax(0.f, v); };
    s.day    = juce::jlimit(0.f, 1.f, pos(raw("eqAir")) / 9.f + pos(raw("mHigh")) / 6.f);                 // mas aire/brillo -> amanece
    s.fog    = juce::jlimit(0.f, 1.f, raw("revMix") * (.6f + .4f * raw("revSize")));                        // reverb -> bruma y nubes
    s.neon   = juce::jlimit(0.f, 1.f, .25f + raw("satDrive") / 24.f * .35f + raw("satMix") * .3f + raw("mDrive") / 18.f * .3f + raw("mClip") * .3f); // saturacion -> neon
    s.car    = juce::jlimit(0.f, 1.f, (raw("cRatio") - 1.f) / 9.f * .7f + raw("cMakeup") / 18.f * .3f);   // compresion -> trafico
    s.echo   = juce::jlimit(0.f, 1.f, raw("dlyMix") + raw("dlyFb") * .3f * raw("dlyMix"));                 // delay -> sombras eco
    s.bounce = juce::jlimit(0.f, 1.f, (raw("eqLow") + 9.f) / 18.f);                                        // graves -> rebote
    s.spread = juce::jlimit(0.f, 1.f, raw("mWidth") / 2.f);                                                // ancho -> separacion
    s.walk   = juce::jlimit(0.f, 1.f, pos(raw("vOut")) / 12.f + pos(raw("inGain")) / 24.f + pos(raw("mOut")) / 6.f); // mas volumen -> caminan
    s.level  = juce::jlimit(0.f, 1.f, proc.meterLevel.load());
    return s;
}

void RVEditor::setScalePct(int pct) {
    static const int pcts[4] = { 25, 50, 75, 100 };
    scalePct = pct;
    proc.apvts.state.setProperty("uiScale", pct, nullptr);   // se recuerda al cerrar y abrir
    for (int i = 0; i < 4; ++i) sizeBtn[i].setToggleState(pcts[i] == pct, juce::dontSendNotification);
    setSize(juce::roundToInt(kW * pct / 100.f), juce::roundToInt(kH * pct / 100.f));
}

void RVEditor::showTab(int t) {
    tab = t;
    for (auto& k : knobs) { bool vis = (t == 0) ? k.section <= 6 : k.section >= 7; k.s->setVisible(vis); k.l->setVisible(vis); }
    resized();
    panels.repaint();
}

void RVEditor::resized() {
    content.setTransform(juce::AffineTransform::scale((float)getWidth() / (float)kW));
    content.setBounds(0, 0, kW, kH);
    scene.setBounds(0, 0, kW, kH);
    panels.setBounds(0, 0, kW, kH);
    presetBox.setBounds(kW - 350, 16, 330, 30);
    vocalBtn.setBounds(kW - 350, 54, 160, 28); masterBtn.setBounds(kW - 180, 54, 160, 28);
    for (int i = 0; i < 4; ++i) sizeBtn[i].setBounds(kW - 350 + i * 84, 90, 78, 24);
    tabVoz.setBounds(26, 452, 110, 26); tabMaster.setBounds(142, 452, 110, 26);
    // filas de secciones por pestana
    static const std::vector<std::vector<int>> rowsVoz = { { 0, 1, 2 }, { 3, 4, 6 }, { 5 } }, rowsMaster = { { 7 }, { 8 } };
    const auto& rows = tab == 0 ? rowsVoz : rowsMaster;
    groups.clear();
    int y = kTop;
    for (auto& row : rows) {
        int cnt = 0; for (int sec : row) for (auto& k : knobs) if (k.section == sec) ++cnt;
        int total = cnt * kCell + ((int)row.size() - 1) * kGap, x = (kW - total) / 2;
        for (int sec : row) {
            int c = 0;
            for (auto& k : knobs) if (k.section == sec) {
                int kx = x + c * kCell; ++c;
                k.l->setBounds(kx, y + 16, kCell, 14);
                k.s->setBounds(kx + 4, y + 30, kCell - 8, 82 - 2);
            }
            groups.push_back({ juce::Rectangle<int>(x - 4, y, c * kCell + 8, kRowH - 6), RV_SECTIONS[sec] });
            x += c * kCell + kGap;
        }
        y += kRowH;
    }
    panels.repaint();
}
