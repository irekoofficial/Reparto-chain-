#pragma once
// Marca Ireko: onda de audio con la "i" naranja + "by ireko"
#include <juce_gui_basics/juce_gui_basics.h>
#include <map>
#include "LogoData.h"
namespace IrekoLogo {
    static inline const juce::Colour orange{ 0xffff4d2b }, wave{ 0xff757375 }, cream{ 0xfff2f0ea }, grey{ 0xffb8b5b0 };

    // Dibuja solo la onda; (x,y) esquina sup. izq., w = ancho total
    inline void drawMark(juce::Graphics& g, float x, float y, float w, float alpha = 1.f) {
        const float u = w / 1152.f;                 // unidades del logo original (1600px)
        const float bw = 96.f * u, step = 132.f * u, cy = 276.f * u;   // cy: centro vertical de las barras grises
        static const float hs[9] = { 80, 180, 300, 440, 0, 440, 300, 180, 80 };
        for (int i = 0; i < 9; ++i) {
            float bx = x + i * step;
            if (i == 4) {
                g.setColour(orange.withAlpha(alpha));
                g.fillRoundedRectangle(bx, y + 132.f * u, bw, 420.f * u, bw * .5f);   // cuerpo de la i
                g.fillEllipse(bx, y, bw, bw);                                         // punto de la i
            } else {
                g.setColour(wave.withAlpha(alpha));
                g.fillRoundedRectangle(bx, y + cy - hs[i] * u * .5f + 24.f * u, bw, hs[i] * u, bw * .5f);
            }
        }
    }
    inline float markHeight(float w) { return 552.f * w / 1152.f; }

    // "by ireko" con la i naranja; baseline-ish en (x,y) arriba-izq
    inline void drawWordmark(juce::Graphics& g, float x, float y, float size, float alpha = 1.f) {
        juce::Font small(juce::FontOptions(size * .78f, juce::Font::plain));
        juce::Font big(juce::FontOptions(size, juce::Font::bold));
        juce::GlyphArrangement ga;
        float cx = x;
        auto put = [&](const juce::String& s, const juce::Font& f, juce::Colour c) {
            juce::GlyphArrangement a; a.addLineOfText(f, s, cx, y + size);
            g.setColour(c.withAlpha(alpha)); a.draw(g);
            cx += a.getBoundingBox(0, -1, true).getWidth() + (s == "by " ? 2.f : 0.f);
        };
        put("by ", small, grey);
        put("i", big, orange);
        put("reko", big, cream);
    }

    // Logo oficial (PNG transparente) con una sombra suave para que se lea sobre cualquier fondo
    inline void drawPng(juce::Graphics& g, float x, float y, int h, bool forLightBg = false) {
        static std::map<int, juce::Image> cache;
        const int key = h + (forLightBg ? 100000 : 0);
        auto it = cache.find(key);
        if (it == cache.end()) {
            juce::Image src = forLightBg ? juce::ImageFileFormat::loadFrom(kIrekoLogoLightPng, (size_t)kIrekoLogoLightPngSize) : juce::ImageFileFormat::loadFrom(kIrekoLogoPng, (size_t)kIrekoLogoPngSize);
            const int w = juce::roundToInt(src.getWidth() * (float)h / (float)src.getHeight());
            juce::Image scaled = src.rescaled(w, h, juce::Graphics::highResamplingQuality);
            const int pad = 6; juce::Image out(juce::Image::ARGB, w + 2 * pad, h + 2 * pad, true);
            { juce::Graphics og(out);
              og.saveState(); og.addTransform(juce::AffineTransform::translation((float)pad, (float)pad));
              juce::DropShadow(forLightBg ? juce::Colour(0x99ffffff) : juce::Colour(0xb0000000), 4, { 0, 1 }).drawForImage(og, scaled); og.restoreState();
              og.drawImageAt(scaled, pad, pad); }
            it = cache.emplace(key, out).first;
        }
        g.drawImageAt(it->second, juce::roundToInt(x) - 6, juce::roundToInt(y) - 6);
    }
}
