#pragma once
// Estilo visual: perillas metalicas con aro de neon, botones tipo pastilla, colores de la noche habanera.
#include <juce_gui_basics/juce_gui_basics.h>

struct RVLook : public juce::LookAndFeel_V4 {
    using C = juce::Colour;
    static inline const C gold{ 0xffffd34d }, pink{ 0xffff3d9a }, cyan{ 0xff2ee6ff }, ink{ 0xff0b0a14 };

    RVLook() {
        setColour(juce::ComboBox::backgroundColourId, C(0xcc14111f));
        setColour(juce::ComboBox::textColourId, gold);
        setColour(juce::ComboBox::outlineColourId, gold.withAlpha(.7f));
        setColour(juce::ComboBox::arrowColourId, gold);
        setColour(juce::PopupMenu::backgroundColourId, C(0xff14111f));
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, pink.withAlpha(.8f));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxTextColourId, gold);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::Label::textColourId, juce::Colours::white);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override {
        auto cx = x + w * .5f, cy = y + h * .5f, r = juce::jmin(w, h) * .5f - 3.f, rb = r - 8.f;
        const float a = a0 + pos * (a1 - a0);
        juce::Path track; track.addCentredArc(cx, cy, r, r, 0, a0, a1, true);
        g.setColour(C(0xff241f35)); g.strokePath(track, juce::PathStrokeType(4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        auto col = cyan.interpolatedWith(pink, pos);
        if (pos > .005f) {
            juce::Path v; v.addCentredArc(cx, cy, r, r, 0, a0, a, true);
            g.setColour(col.withAlpha(.25f)); g.strokePath(v, juce::PathStrokeType(9.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour(col); g.strokePath(v, juce::PathStrokeType(4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setColour(C(0x88000000)); g.fillEllipse(cx - rb + 1, cy - rb + 4, rb * 2, rb * 2);
        g.setGradientFill(juce::ColourGradient(C(0xff4a4660), cx - rb * .5f, cy - rb * .7f, C(0xff0f0d18), cx + rb * .6f, cy + rb, true));
        g.fillEllipse(cx - rb, cy - rb, rb * 2, rb * 2);
        g.setColour(gold.withAlpha(.85f)); g.drawEllipse(cx - rb, cy - rb, rb * 2, rb * 2, 2.f);
        for (int i = 0; i < 28; ++i) {                                  // ranuras tipo perilla
            float ang = i * juce::MathConstants<float>::twoPi / 28.f, s = std::sin(ang), c = -std::cos(ang);
            g.setColour(C(0x66000000)); g.drawLine(cx + s * (rb - 5), cy + c * (rb - 5), cx + s * (rb - 1.5f), cy + c * (rb - 1.5f), 1.5f);
        }
        g.setColour(C(0xff1b1828)); g.fillEllipse(cx - rb * .62f, cy - rb * .62f, rb * 1.24f, rb * 1.24f);
        g.setColour(gold.withAlpha(.25f)); g.drawEllipse(cx - rb * .62f, cy - rb * .62f, rb * 1.24f, rb * 1.24f, 1.f);
        float s = std::sin(a), c = -std::cos(a);
        g.setColour(gold); g.drawLine(cx + s * rb * .2f, cy + c * rb * .2f, cx + s * rb * .88f, cy + c * rb * .88f, 3.f);
        g.setColour(col); g.fillEllipse(cx + s * rb * .88f - 2.5f, cy + c * rb * .88f - 2.5f, 5.f, 5.f);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool hover, bool) override {
        auto r = b.getLocalBounds().toFloat().reduced(1.f); bool on = b.getToggleState();
        g.setColour(on ? gold : C(0xcc14111f)); g.fillRoundedRectangle(r, r.getHeight() * .5f);
        g.setColour(on ? gold.brighter(.3f) : gold.withAlpha(hover ? 1.f : .55f)); g.drawRoundedRectangle(r, r.getHeight() * .5f, 1.6f);
    }
    void drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) override {
        g.setFont(juce::FontOptions(14.f, juce::Font::bold)); g.setColour(b.getToggleState() ? ink : gold);
        g.drawText(b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
    }
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool hover, bool) override {
        auto r = b.getLocalBounds().toFloat().reduced(1.f); bool on = b.getToggleState();
        g.setColour(C(0xcc14111f)); g.fillRoundedRectangle(r, r.getHeight() * .5f);
        g.setColour((on ? cyan : gold).withAlpha(hover ? 1.f : .6f)); g.drawRoundedRectangle(r, r.getHeight() * .5f, 1.6f);
        float d = r.getHeight() - 10.f;
        if (on) { g.setColour(cyan.withAlpha(.3f)); g.fillEllipse(r.getX() + 2, r.getY() + 2, d + 6, d + 6); }
        g.setColour(on ? cyan : C(0xff3a3550)); g.fillEllipse(r.getX() + 5, r.getY() + 5, d, d);
        g.setFont(juce::FontOptions(13.f, juce::Font::bold)); g.setColour(on ? juce::Colours::white : juce::Colours::grey);
        g.drawText(b.getButtonText(), r.withTrimmedLeft(d + 12).toNearestInt(), juce::Justification::centredLeft);
    }
    void drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& cb) override {
        auto r = juce::Rectangle<float>(0, 0, (float)w, (float)h).reduced(1.f);
        g.setColour(C(0xcc14111f)); g.fillRoundedRectangle(r, h * .5f);
        g.setColour(gold.withAlpha(.8f)); g.drawRoundedRectangle(r, h * .5f, 1.6f);
        juce::Path p; p.addTriangle((float)w - 24, h * .42f, (float)w - 12, h * .42f, (float)w - 18, h * .62f); g.setColour(cb.findColour(juce::ComboBox::arrowColourId)); g.fillPath(p);
    }
    juce::Font getComboBoxFont(juce::ComboBox&) override { return juce::Font(juce::FontOptions(14.f, juce::Font::bold)); }
    void positionComboBoxText(juce::ComboBox& cb, juce::Label& l) override { l.setBounds(14, 1, cb.getWidth() - 40, cb.getHeight() - 2); l.setFont(getComboBoxFont(cb)); }
};
