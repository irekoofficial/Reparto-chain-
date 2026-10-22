#pragma once
// Editor LITE: interfaz estatica y compacta (sin animacion, sin temporizadores), 2 perillas por modulo.
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "KnobLook.h"
#include "Logo.h"
#include "GuyDraw.h"
#include "HavanaScene.h"

// Escena Lite: La Habana de dia (fondo dibujado UNA vez en una imagen) + un solo repartero (camiseta negra) + un carro.
// Tamano = 50% de la escena completa (550x220). 15 cuadros/s y solo se repinta la zona del repartero y la calle.
struct LiteScene : public juce::Component, private juce::Timer {
    static constexpr int kSW = 550, kSH = 220;
    std::function<float()> level;
    LiteScene() {
        setInterceptsMouseClicks(false, false);
        st.day = 1.f; st.fog = .12f; st.neon = .2f; st.car = .2f;
        juce::Image big(juce::Image::ARGB, 1100, 440, true);
        { HavanaScene hs; juce::Graphics g(big); hs.drawBackdrop(g, 1100.f, 4.f, st, false); }
        bg = juce::Image(juce::Image::ARGB, kSW, kSH, true);
        {
            juce::Graphics g(bg); g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
            g.drawImage(big, 0, 0, kSW, kSH, 0, 0, 1100, 440);
            // titulo y logo sobre el cielo (igual que la version completa, en pequeno)
            g.setFont(juce::FontOptions(32.f, juce::Font::bold | juce::Font::italic));
            g.setColour(juce::Colour(0x99000000)); g.drawText("REPARTO CHAIN", 17, 11, 330, 40, juce::Justification::left);
            IrekoLogo::gradText(g, "REPARTO CHAIN", juce::Font(juce::FontOptions(32.f, juce::Font::bold | juce::Font::italic)), 15, 9, 330, 40, juce::ColourGradient(juce::Colour(0xffffe27a), 0, 10, juce::Colour(0xffe29a00), 0, 44, false));
            g.setColour(RVLook::pink); g.fillRoundedRectangle(262.f, 20.f, 44.f, 18.f, 9.f);
            g.setColour(juce::Colours::white); g.setFont(juce::FontOptions(11.f, juce::Font::bold)); g.drawText("LITE", 262, 20, 44, 18, juce::Justification::centred);
            IrekoLogo::drawPng(g, 14.f, 54.f, 44, true);
        }
        helper.pauseTimer();
        startTimerHz(15);
    }
    ~LiteScene() override { stopTimer(); }
    void paint(juce::Graphics& g) override {
        juce::Path clip; clip.addRoundedRectangle(0.f, 0.f, (float)kSW, (float)kSH, 14.f);
        g.saveState(); g.reduceClipRegion(clip);
        g.drawImageAt(bg, 0, 0);
        g.addTransform(juce::AffineTransform::scale(.5f));          // dibujamos en coordenadas de la escena completa
        static const juce::Colour cols[2] = { juce::Colour(0xff1fb5b0), juce::Colour(0xffd8443c) };
        const float L = 1400.f, far = 440.f * .66f, curb = 440.f * .79f;
        { float x = std::fmod(time * 85.f, L) - 150.f; helper.drawCar(g, x, far + (curb - far) * .72f, .95f, 1, cols[0], st); }
        { float x = L - std::fmod(time * 60.f + 500.f, L) - 150.f; helper.drawCar(g, x, far + (curb - far) * .42f, .8f, -1, cols[1], st); }
        const float lv = level ? juce::jlimit(0.f, 1.f, level()) : 0.f;
        const float bob = std::abs(std::sin(time * (1.7f + lv * 1.5f) * 3.14159f)) * (3.f + 8.f * lv) + lv * 10.f;
        GuyDraw::person(g, 700.f, 424.f, 1.f, GuyDraw::blackShirt, time, 1, bob, -1.f, 1.f);
        g.restoreState();
        g.setColour(RVLook::gold.withAlpha(.5f)); g.drawRoundedRectangle(.5f, .5f, (float)kSW - 1.f, (float)kSH - 1.f, 14.f, 1.2f);
    }
private:
    HavanaScene helper;   // solo para dibujar el carro (su temporizador se detiene)
    SceneState st; juce::Image bg; float time = 0.f;
    void timerCallback() override { time += 1.f / 15.f; repaint(300, 74, 100, 146); repaint(0, 124, kSW, 58); }
};

class RVLiteEditor : public juce::AudioProcessorEditor {
public:
    explicit RVLiteEditor(RVProcessor&);
    ~RVLiteEditor() override;
    void resized() override;
private:
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    struct Knob { std::unique_ptr<juce::Slider> s; std::unique_ptr<juce::Label> l; std::unique_ptr<SA> a; int group = 0; };
    struct Layer : juce::Component { std::function<void(juce::Graphics&)> fn; Layer() { setInterceptsMouseClicks(false, false); setOpaque(true); } void paint(juce::Graphics& g) override { if (fn) fn(g); } };
    void setScalePct(int pct);

    RVProcessor& proc;
    RVLook look;
    struct Content : juce::Component {} content;
    Layer back;
    LiteScene guy;
    juce::ComboBox presetBox;
    juce::ToggleButton vocalBtn{ "CADENA VOZ" }, masterBtn{ "CADENA MASTER" };
    std::unique_ptr<BA> vocalAtt, masterAtt;
    std::vector<Knob> knobs;
    juce::TextButton sizeBtn[4];
    int scalePct = 100;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RVLiteEditor)
};
