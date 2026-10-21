#pragma once
#include "Logo.h"
// Escena animada: La Habana de noche con reparteros (personajes ORIGINALES, dibujados por codigo).
// Todo se dibuja con vectores, sin imagenes externas: no usa fotos ni personajes de nadie.
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <cmath>

struct SceneState {
    float day = 0;      // 0 noche .. 1 dia      (Aire / brillo)
    float fog = 0;      // 0..1 bruma y nubes    (Reverb)
    float neon = .3f;   // 0..1 luces y neon     (Saturacion / Drive)
    float car = .2f;    // 0..1 trafico          (Compresor)
    float echo = 0;     // 0..1 sombras eco      (Delay)
    float bounce = .5f; // 0..1 rebote           (EQ cuerpo / graves)
    float spread = .5f; // 0..1 separacion       (Ancho estereo)
    float walk = 0;     // 0..1 caminan          (Ganancia / salida)
    float level = 0;    // 0..1 nivel de audio
};

class HavanaScene : public juce::Component, private juce::Timer {
public:
    using C = juce::Colour;
    std::function<SceneState()> provider;
    static constexpr float kSceneH = 440.f;

    HavanaScene() {
        setInterceptsMouseClicks(false, false);
        setOpaque(true);
        juce::Random r(7);
        for (int i = 0; i < 70; ++i) stars.push_back({ r.nextFloat(), r.nextFloat() * 0.5f, r.nextFloat() * 6.28f, 0.5f + r.nextFloat() });
        float x = -20;
        while (x < 1300) {
            Bld b; b.x = x; b.w = 44.f + r.nextFloat() * 52.f; b.h = 50.f + r.nextFloat() * 90.f;
            b.col = palette[r.nextInt(6)]; b.seed = r.nextFloat() * 1000.f; blds.push_back(b); x += b.w + 2.f;
        }
        startTimerHz(30);
    }

    void paint(juce::Graphics& g) override { drawScene(g, (float)getWidth(), time, cur); }

    void pauseTimer() { stopTimer(); }   // para usarla solo como "pincel" (version Lite)
    // Version Lite: fondo y utileria sueltos (sin personajes, sin titulo)
    void drawBackdrop(juce::Graphics& g, float W, float t, const SceneState& s, bool withCars) {
        const float S = kSceneH; g.fillAll(C(0xff07060f));
        sky(g, W, S, t, s.day); clouds(g, W, S, t, s); skyline(g, W, S, t, s); ground(g, W, S, t, s); lamps(g, W, S, t, s);
        if (withCars) cars(g, W, S, t, s);
        palms(g, W, S, t);
    }
    void drawCar(juce::Graphics& g, float x, float y, float sc, int dir, C col, const SceneState& s) { car(g, x, y, sc, dir, col, s); }

    // para pruebas / previsualizacion
    void drawScene(juce::Graphics& g, float W, float t, const SceneState& s) {
        const float S = kSceneH;
        const float day = s.day;
        g.fillAll(C(0xff07060f));
        g.saveState();
        g.reduceClipRegion(0, 0, (int)W, (int)S + 60);
        sky(g, W, S, t, day);
        clouds(g, W, S, t, s);
        skyline(g, W, S, t, s);
        ground(g, W, S, t, s);
        lamps(g, W, S, t, s);
        cars(g, W, S, t, s);
        palms(g, W, S, t);
        crew(g, W, S, t, s);
        wash(g, W, S, s);
        g.restoreState();
        // fundido hacia los paneles
        g.setGradientFill(juce::ColourGradient(C(0x0007060f), 0, S - 40, C(0xff0b0a14), 0, S + 40, false));
        g.fillRect(0.f, S - 40, W, 100.f);
        g.setColour(C(0xff0b0a14)); g.fillRect(0.f, S + 40, W, 2000.f);
        title(g, W, t, s);
    }

private:
    struct Star { float x, y, ph, sz; };
    struct Bld { float x, w, h, seed; C col; };
    std::vector<Star> stars; std::vector<Bld> blds;
    SceneState cur; float time = 0;
    const C palette[6] = { C(0xffe98fa6), C(0xff5fb8b0), C(0xffe3b15c), C(0xff8fa6e0), C(0xffd9776a), C(0xffa8cf86) };

    void timerCallback() override {
        time += 1.f / 30.f;
        SceneState t = provider ? provider() : SceneState();
        auto sm = [](float& a, float b, float k) { a += (b - a) * k; };
        sm(cur.day, t.day, .035f); sm(cur.fog, t.fog, .06f); sm(cur.neon, t.neon, .08f); sm(cur.car, t.car, .08f);
        sm(cur.echo, t.echo, .1f); sm(cur.bounce, t.bounce, .1f); sm(cur.spread, t.spread, .06f);
        sm(cur.walk, t.walk, .05f); sm(cur.level, t.level, .35f);
        repaint(0, 0, getWidth(), (int)kSceneH + 60);
    }

    static float hash(float a, float b) { float v = std::sin(a * 127.1f + b * 311.7f) * 43758.5453f; return v - std::floor(v); }
    static C mix(C a, C b, float k) { return a.interpolatedWith(b, juce::jlimit(0.f, 1.f, k)); }

    // ---------------- cielo ----------------
    void sky(juce::Graphics& g, float W, float S, float t, float day) {
        auto top = mix(C(0xff070820), C(0xff2f8ee0), day), mid = mix(C(0xff2a1247), C(0xff7cc4f2), day), hor = mix(C(0xff8a2f64), C(0xffffd9a0), day);
        juce::ColourGradient cg(top, 0, 0, hor, 0, S * .62f, false); cg.addColour(.55, mid);
        g.setGradientFill(cg); g.fillRect(0.f, 0.f, W, S);
        float starA = juce::jlimit(0.f, 1.f, 1.f - day * 1.6f);
        for (auto& st : stars) {
            float a = starA * (0.45f + 0.55f * std::sin(t * 1.7f * st.sz + st.ph)) * (1.f - st.y * 1.4f);
            if (a <= 0) continue;
            g.setColour(juce::Colours::white.withAlpha(a)); g.fillEllipse(st.x * W, st.y * S, st.sz * 1.6f, st.sz * 1.6f);
        }
        // luna
        float my = juce::jmap(day, 70.f, S * .55f), ma = juce::jlimit(0.f, 1.f, 1.f - day * 1.3f);
        if (ma > 0) {
            g.setGradientFill(juce::ColourGradient(C(0xffe8ecff).withAlpha(.35f * ma), W * .8f, my, C(0x00e8ecff), W * .8f + 90, my, true)); g.fillEllipse(W * .8f - 90, my - 90, 180, 180);
            g.setColour(C(0xfff4f1e6).withAlpha(ma)); g.fillEllipse(W * .8f - 24, my - 24, 48, 48);
            g.setColour(C(0xffd9d5c5).withAlpha(ma * .6f)); g.fillEllipse(W * .8f - 8, my - 10, 12, 12); g.fillEllipse(W * .8f + 6, my + 4, 8, 8);
        }
        // sol
        float sy = juce::jmap(day, S * .6f, 80.f), sa = juce::jlimit(0.f, 1.f, (day - .15f) * 2.f);
        if (sa > 0) {
            g.setGradientFill(juce::ColourGradient(C(0xffffe9a0).withAlpha(.6f * sa), W * .22f, sy, C(0x00ffe9a0), W * .22f + 150, sy, true)); g.fillEllipse(W * .22f - 150, sy - 150, 300, 300);
            g.setColour(C(0xffffd34d).withAlpha(sa)); g.fillEllipse(W * .22f - 34, sy - 34, 68, 68);
        }
    }

    void clouds(juce::Graphics& g, float W, float S, float t, const SceneState& s) {
        float a = .12f + s.fog * .5f; auto col = mix(C(0xff6a5a8c), C(0xfffafafa), s.day);
        for (int i = 0; i < 6; ++i) {
            float sp = 6.f + i * 3.f, x = std::fmod(t * sp + i * 260.f, W + 400.f) - 200.f, y = 40.f + i * 26.f + std::sin(t * .3f + i) * 4.f, k = 1.f + s.fog * .8f;
            g.setColour(col.withAlpha(a * (.5f + .1f * i)));
            for (int j = 0; j < 4; ++j) g.fillEllipse(x + j * 34 * k - 30, y + (j % 2 ? -8.f : 0.f), 80 * k, 28 * k);
        }
        g.setGradientFill(juce::ColourGradient(C(0x00ffffff), 0, S * .35f, mix(C(0xffb48cc8), C(0xffffffff), s.day).withAlpha(.55f * s.fog), 0, S * .72f, false));
        g.fillRect(0.f, S * .35f, W, S * .4f);
    }

    // ---------------- ciudad ----------------
    void skyline(juce::Graphics& g, float W, float S, float t, const SceneState& s) {
        const float base = S * .62f, nightK = 1.f - s.day;
        const float lit = nightK * (.35f + .65f * s.neon);
        for (auto& b : blds) {
            if (b.x > W) break;
            C c = mix(b.col.darker(.2f), C(0xff0c0a1c), .8f * nightK);
            g.setColour(c); g.fillRect(b.x, base - b.h, b.w, b.h + 4);
            g.setColour(c.darker(.3f)); g.fillRect(b.x, base - b.h - 4, b.w, 4.f);
            for (float wy = base - b.h + 8; wy < base - 10; wy += 15)
                for (float wx = b.x + 6; wx < b.x + b.w - 8; wx += 12) {
                    float h = hash(wx + b.seed, wy);
                    bool on = h < .55f;
                    float tw = .85f + .15f * std::sin(t * 2.f + h * 50.f);
                    g.setColour(on ? C(0xffffd27a).withAlpha(juce::jlimit(0.f, 1.f, lit * tw + .05f)) : mix(C(0xff1a1a2e), C(0xff9fc4e8), s.day).withAlpha(.9f));
                    g.fillRect(wx, wy, 6.f, 8.f);
                }
        }
        // capitolio (cupula generica de La Habana)
        float cx = W * .33f; C dc = mix(C(0xffd8cfc2), C(0xff1d1830), .75f * nightK);
        g.setColour(dc); g.fillRect(cx - 80, base - 46, 160.f, 50.f);
        g.setColour(dc.darker(.25f)); for (int i = 0; i < 9; ++i) g.fillRect(cx - 70 + i * 17.5f, base - 40, 6.f, 38.f);
        g.setColour(dc); g.fillRect(cx - 38, base - 70, 76.f, 26.f);
        juce::Path dome; dome.addCentredArc(cx, base - 70, 34, 52, 0, -1.5708f, 1.5708f, true); dome.lineTo(cx - 34, base - 70); dome.closeSubPath();
        juce::Path d2; d2.startNewSubPath(cx - 34, base - 70); d2.addCentredArc(cx, base - 70, 34, 56, 0, -1.5708f, 1.5708f, true); d2.closeSubPath();
        g.fillPath(d2); g.fillRect(cx - 3, base - 140, 6.f, 18.f);
        g.setColour(C(0xffffd27a).withAlpha(lit * .9f)); for (int i = 0; i < 7; ++i) g.fillRect(cx - 52 + i * 16.f, base - 38, 5.f, 12.f);
        // letrero de neon
        float sx = W * .72f, sy = base - 128;
        float flick = (std::sin(t * 23.f) > .93f) ? .35f : 1.f;
        float na = (.3f + .7f * s.neon) * flick * (.35f + .65f * nightK);
        g.setColour(C(0xff10101c)); g.fillRoundedRectangle(sx - 8, sy - 4, 124, 34, 6);
        for (int k = 3; k >= 0; --k) {
            g.setColour(C(0xff2ee6ff).withAlpha(na * (k == 0 ? 1.f : .12f))); g.setFont(juce::FontOptions(22.f, juce::Font::bold));
            for (int dx = -k; dx <= k; dx += juce::jmax(1, k)) for (int dy = -k; dy <= k; dy += juce::jmax(1, k)) g.drawText("HABANA", (int)sx + dx, (int)sy + dy, 108, 28, juce::Justification::centred);
        }
        // mar de fondo / bruma baja
        g.setGradientFill(juce::ColourGradient(C(0x00000000), 0, base - 30, mix(C(0xff2a1247), C(0xffffe6c0), s.day).withAlpha(.5f), 0, base + 6, false)); g.fillRect(0.f, base - 30, W, 36.f);
    }

    // ---------------- calle ----------------
    void ground(juce::Graphics& g, float W, float S, float t, const SceneState& s) {
        const float base = S * .62f, far = S * .66f, curb = S * .79f; (void)t;
        g.setColour(mix(C(0xff6b6577), C(0xff2a2634), 1.f - s.day * .6f)); g.fillRect(0.f, base, W, far - base);                  // acera lejana
        g.setGradientFill(juce::ColourGradient(mix(C(0xff201c2e), C(0xff55535f), s.day), 0, far, mix(C(0xff12101c), C(0xff3d3b46), s.day), 0, curb, false)); g.fillRect(0.f, far, W, curb - far); // asfalto
        g.setColour(C(0xffe8d36a).withAlpha(.7f)); for (float x = 10; x < W; x += 70) g.fillRect(x, (far + curb) * .5f, 36.f, 3.f);
        g.setGradientFill(juce::ColourGradient(mix(C(0xff4a4458), C(0xffa9a4b2), s.day), 0, curb, mix(C(0xff211d2d), C(0xff6d6a78), s.day), 0, S + 10, false)); g.fillRect(0.f, curb, W, S + 10 - curb); // acera
        g.setColour(mix(C(0xffb7b1c4), C(0xffeeeaf5), s.day)); g.fillRect(0.f, curb - 2, W, 4.f);
        // reflejos de neon en el asfalto mojado
        float ra = (1.f - s.day) * (.08f + .25f * s.neon);
        const C rc[3] = { C(0xffff3d9a), C(0xff2ee6ff), C(0xffffc83d) };
        for (int i = 0; i < 6; ++i) { float x = W * (.08f + i * .17f); g.setGradientFill(juce::ColourGradient(rc[i % 3].withAlpha(ra), x, far, rc[i % 3].withAlpha(0.f), x, curb, false)); g.fillRect(x - 10, far, 20.f, curb - far); }
    }

    void lamps(juce::Graphics& g, float W, float S, float t, const SceneState& s) {
        const float curb = S * .79f; float a = (1.f - s.day) * (.4f + .6f * s.neon);
        for (int i = 0; i < 4; ++i) {
            float x = W * (.12f + i * .26f) + (i % 2) * 20.f, top = curb - 190.f;
            g.setColour(C(0xff1c1a26)); g.fillRect(x - 3, top, 6.f, 190.f); g.fillRect(x - 3, top, 26.f, 5.f);
            float fl = (i == 2 && std::sin(t * 17.f) > .9f) ? .4f : 1.f;
            g.setGradientFill(juce::ColourGradient(C(0xffffc86a).withAlpha(.7f * a * fl), x + 22, top + 8, C(0x00ffc86a), x + 22, top + 120, true)); g.fillEllipse(x - 100, top - 90, 244.f, 244.f);
            g.setColour(C(0xffffe9b0).withAlpha(.3f + .7f * a * fl)); g.fillEllipse(x + 15, top + 3, 14.f, 8.f);
        }
    }

    void cars(juce::Graphics& g, float W, float S, float t, const SceneState& s) {
        const float far = S * .66f, curb = S * .79f; float spd = 25.f + s.car * 260.f;
        const C cols[3] = { C(0xff1fb5b0), C(0xffd8443c), C(0xfff0e2b8) };
        for (int i = 0; i < 3; ++i) {
            int dir = (i % 2) ? -1 : 1; float L = W + 300.f;
            float x = std::fmod(t * spd * (.8f + i * .15f) + i * 420.f, L); if (dir < 0) x = L - x; x -= 150.f;
            float y = far + (curb - far) * (i == 2 ? .38f : (i % 2 ? .42f : .72f)); float sc = (i % 2 ? .8f : .95f);
            car(g, x, y, sc, dir, cols[i], s);
        }
    }
    void car(juce::Graphics& g, float x, float y, float sc, int dir, C col, const SceneState& s) {
        g.saveState(); g.addTransform(juce::AffineTransform::scale(sc * dir, sc).translated(x, y));
        float nightK = 1.f - s.day;
        g.setColour(C(0x55000000)); g.fillEllipse(-4, -4, 128, 10);
        juce::Path body; body.addRoundedRectangle(0.f, -24.f, 124.f, 22.f, 8.f);
        juce::Path cab; cab.startNewSubPath(24, -24); cab.lineTo(38, -46); cab.lineTo(88, -46); cab.lineTo(104, -24); cab.closeSubPath();
        g.setColour(col); g.fillPath(body); g.fillPath(cab.createPathWithRoundedCorners(5.f));
        g.setColour(C(0xff9fd8ee).withAlpha(.85f)); juce::Path win; win.startNewSubPath(32, -26); win.lineTo(42, -42); win.lineTo(84, -42); win.lineTo(96, -26); win.closeSubPath(); g.fillPath(win);
        g.setColour(juce::Colours::white.withAlpha(.6f)); g.fillRect(4.f, -12.f, 116.f, 2.f);
        for (float wx : { 28.f, 98.f }) { g.setColour(C(0xff111111)); g.fillEllipse(wx - 10, -12, 20, 20); g.setColour(C(0xffc9c9d4)); g.fillEllipse(wx - 5, -7, 10, 10); }
        if (nightK > .05f) { // faros
            g.setGradientFill(juce::ColourGradient(C(0xffffeeb0).withAlpha(.55f * nightK), 124, -14, C(0x00ffeeb0), 190, -14, false)); g.fillRect(124.f, -26.f, 70.f, 24.f);
            g.setColour(C(0xffffffd0).withAlpha(nightK)); g.fillEllipse(118, -20, 8, 8);
            g.setColour(C(0xffff3030).withAlpha(nightK)); g.fillEllipse(-2, -20, 6, 8);
        }
        g.restoreState();
    }

    void palms(juce::Graphics& g, float W, float S, float t) {
        const float curb = S * .79f; const float xs[3] = { .04f, .5f, .96f };
        for (int i = 0; i < 3; ++i) {
            float x = W * xs[i], y = curb + (i == 1 ? 0.f : 30.f), h = (i == 1 ? 150.f : 230.f), sw = std::sin(t * .9f + i) * 6.f;
            juce::Path tr; tr.startNewSubPath(x - 7, y); tr.quadraticTo(x - 10 + sw * .4f, y - h * .5f, x + sw, y - h); tr.lineTo(x + sw + 5, y - h); tr.quadraticTo(x - 2 + sw * .4f, y - h * .5f, x + 7, y); tr.closeSubPath();
            g.setColour(C(0xff3b2a24)); g.fillPath(tr);
            for (int k = 0; k < 7; ++k) {
                float a = -2.9f + k * .5f + std::sin(t * 1.3f + k + i) * .06f, L = (i == 1 ? 70.f : 105.f);
                juce::Path f; float tx = x + sw, ty = y - h;
                f.startNewSubPath(tx, ty); f.quadraticTo(tx + std::cos(a) * L * .6f, ty + std::sin(a) * L * .6f - 22, tx + std::cos(a) * L, ty + std::sin(a) * L + 18);
                f.quadraticTo(tx + std::cos(a) * L * .55f, ty + std::sin(a) * L * .5f - 4, tx, ty);
                g.setColour(C(0xff1c6b45).darker(k % 2 ? .2f : 0.f)); g.fillPath(f);
            }
        }
    }

    // ---------------- reparteros (originales) ----------------
    struct Guy { float ox; C skin, cap, hood, pants, shoe; bool shades, beanie; };
    void crew(juce::Graphics& g, float W, float S, float t, const SceneState& s) {
        static const Guy guys[4] = {
            { -.30f, C(0xffc68642), C(0xfff2c200), C(0xffe0b000), C(0xff1d4e7a), C(0xfff2c200), false, false },
            { -.10f, C(0xff8d5524), C(0xfff26b1d), C(0xff17171c), C(0xff2a2d31), C(0xfff26b1d), false, true },
            {  .10f, C(0xffe0ac69), C(0xff2243c0), C(0xff263c96), C(0xffc8b38a), C(0xff2243c0), true,  false },
            {  .30f, C(0xff5c3a21), C(0xfff2f2f2), C(0xffe86a92), C(0xff9bb7d4), C(0xffe8374a), true,  false } };
        const float fy = S - 16.f; const float bpm = 1.6f + s.level * 1.2f;
        float spreadK = .55f + s.spread * .95f;
        for (int pass = 2; pass >= 0; --pass) {          // eco (delay): copias fantasma detras
            float a = pass == 0 ? 1.f : (.22f / pass) * s.echo * 3.f; if (pass > 0 && a < .02f) continue;
            for (int i = 0; i < 4; ++i) {
                float walkAmp = s.walk * 70.f, wp = t * (2.f + s.walk * 5.f) + i * 1.3f;
                float dir = std::cos(t * .45f + i) >= 0 ? 1.f : -1.f;
                float x = W * .5f + guys[i].ox * W * spreadK * 1.05f + (s.walk > .04f ? std::sin(t * .45f + i) * walkAmp : 0.f) - pass * 34.f * dir;
                float bob = (std::abs(std::sin(t * bpm * 3.14159f + i * .7f)) * (2.f + s.bounce * 9.f) + s.level * 10.f);
                if (a < 1.f) g.beginTransparencyLayer(a);
                person(g, x, fy, 1.f, guys[i], t, i, bob, s.walk > .04f ? wp : -1.f, s.walk > .04f ? dir : (i < 2 ? 1.f : -1.f), s);
                if (a < 1.f) g.endTransparencyLayer();
            }
        }
    }

    void person(juce::Graphics& g, float fx, float fy, float sc, const Guy& p, float t, int idx, float bob, float wp, float face, const SceneState& s) {
        g.saveState(); g.addTransform(juce::AffineTransform::scale(sc * face, sc).translated(fx, fy));
        const C ol(0xff0d0b14);
        g.setColour(C(0x66000000)); g.fillEllipse(-62, -8, 124, 16);
        auto outline = [&](const juce::Path& pth, C c) { g.setColour(c); g.fillPath(pth); g.setColour(ol); g.strokePath(pth, juce::PathStrokeType(2.f)); };
        const float hipY = -84.f - bob * .0f; bool walking = wp >= 0.f;
        g.addTransform(juce::AffineTransform::translation(0, -bob));
        // piernas anchas (pantalon baggy) y tenis
        for (int sd = -1; sd <= 1; sd += 2) {
            float sw = walking ? std::sin(wp + (sd > 0 ? 3.14159f : 0.f)) : 0.f, lift = walking ? juce::jmax(0.f, sw) * 9.f : 0.f, lx = sd * 19.f + sw * 12.f;
            juce::Path leg; leg.startNewSubPath(lx - 15, hipY); leg.lineTo(lx + 15, hipY); leg.lineTo(lx + 24, -18 - lift); leg.lineTo(lx - 24, -18 - lift); leg.closeSubPath();
            outline(leg.createPathWithRoundedCorners(6.f), p.pants);
            juce::Path shoe; shoe.addRoundedRectangle(lx - 24, -19 - lift + bob * 0, 52.f, 19.f, 8.f); outline(shoe, p.shoe);
            g.setColour(juce::Colours::white.withAlpha(.9f)); g.fillRoundedRectangle(lx - 24, -8 - lift, 52.f, 8.f, 4.f);
        }
        juce::Path waist; waist.addRoundedRectangle(-40.f, hipY - 10, 80.f, 18.f, 6.f); outline(waist, p.pants.darker(.15f));
        // torso (sudadera ancha)
        juce::Path torso; torso.startNewSubPath(-38, -150); torso.lineTo(38, -150); torso.lineTo(50, -76); torso.lineTo(-50, -76); torso.closeSubPath();
        outline(torso.createPathWithRoundedCorners(14.f), p.hood);
        g.setColour(p.hood.darker(.35f)); g.fillRoundedRectangle(-24, -160, 48, 20, 10);
        g.setColour(p.hood.darker(.25f)); g.fillRoundedRectangle(-30, -92, 60, 18, 6);   // bolsillo
        // brazos
        float beat = std::sin(t * 6.f + idx);
        for (int sd = -1; sd <= 1; sd += 2) {
            bool raised = (sd > 0) && (idx % 2 == 0);
            float ang = raised ? -(2.0f + beat * .12f) : (sd * .14f) + (walking ? std::sin(wp + (sd > 0 ? 0.f : 3.14f)) * .35f : 0.f);
            g.saveState(); g.addTransform(juce::AffineTransform::rotation(ang * (sd > 0 ? 1.f : -1.f) * (raised ? -1.f : 1.f) + (raised ? 3.14159f : 0.f) * 0.f, sd * 40.f, -144.f));
            juce::Path arm; arm.addRoundedRectangle(sd * 40.f - 11.f, -148.f, 22.f, 66.f, 10.f);
            if (raised) { g.restoreState(); g.saveState(); g.addTransform(juce::AffineTransform::rotation(-2.45f + beat * .1f, 40.f, -144.f)); arm.clear(); arm.addRoundedRectangle(40.f - 11.f, -148.f, 22.f, 66.f, 10.f); }
            outline(arm, p.hood);
            g.setColour(p.skin); g.fillEllipse(sd * 40.f - 9.f, -90.f, 18.f, 18.f); g.setColour(ol); g.drawEllipse(sd * 40.f - 9.f, -90.f, 18.f, 18.f, 1.5f);
            g.restoreState();
        }
        // cadena dorada con dije
        float glint = .5f + .5f * std::sin(t * 3.f + idx * 2.f);
        juce::Path ch; ch.startNewSubPath(-22, -148); ch.quadraticTo(0, -108, 22, -148);
        g.setColour(C(0xffffd34d)); g.strokePath(ch, juce::PathStrokeType(3.f)); g.setColour(juce::Colours::white.withAlpha(.3f + .5f * glint)); g.strokePath(ch, juce::PathStrokeType(1.f));
        g.setColour(C(0xffffd34d)); g.fillRoundedRectangle(-8, -122, 16, 16, 5); g.setColour(ol); g.drawRoundedRectangle(-8, -122, 16, 16, 5, 1.5f);
        g.setColour(juce::Colours::white.withAlpha(.4f + .5f * glint)); g.fillEllipse(-4, -118, 4, 4);
        // cabeza
        juce::Path head; head.addRoundedRectangle(-33.f, -214.f, 66.f, 62.f, 24.f); outline(head, p.skin);
        g.setColour(p.skin.darker(.2f)); g.fillEllipse(-38, -190, 10, 16); g.fillEllipse(28, -190, 10, 16);
        g.setColour(juce::Colours::white); g.setColour(ol);
        if (p.shades) { g.fillRoundedRectangle(-26, -192, 52, 18, 6); g.setColour(juce::Colours::white.withAlpha(.35f)); g.fillRect(-20.f, -189.f, 12.f, 3.f); }
        else { float bl = (std::fmod(t + idx * 1.7f, 3.4f) < .12f) ? 1.f : 6.f; g.fillEllipse(-19, -190, 10, bl * 2.f); g.fillEllipse(9, -190, 10, bl * 2.f); }
        g.setColour(ol); juce::Path sm; sm.addCentredArc(0, -170, 14, 9, 0, 1.4f, 4.9f, true); sm.clear(); sm.startNewSubPath(-12, -166); sm.quadraticTo(0, -154, 12, -166); g.strokePath(sm, juce::PathStrokeType(2.5f));
        // gorra / gorro
        if (p.beanie) {
            juce::Path b; b.addRoundedRectangle(-35.f, -234.f, 70.f, 40.f, 16.f); outline(b, p.cap);
            g.setColour(p.cap.darker(.25f)); g.fillRoundedRectangle(-35, -208, 70, 14, 6);
        } else {
            juce::Path dome; dome.startNewSubPath(-35, -198); dome.quadraticTo(-35, -236, 0, -236); dome.quadraticTo(35, -236, 35, -198); dome.closeSubPath();
            outline(dome, p.cap);
            juce::Path brim; brim.addRoundedRectangle(-8.f, -204.f, 60.f, 9.f, 4.5f); outline(brim, p.cap.darker(.15f));
        }
        g.restoreState();
        (void)s; (void)t;
    }

    void wash(juce::Graphics& g, float W, float S, const SceneState& s) {
        float a = (1.f - s.day) * (.05f + .22f * s.neon);
        g.setGradientFill(juce::ColourGradient(C(0xffff3d9a).withAlpha(a), 0, S * .4f, C(0x00ff3d9a), W * .35f, S * .4f, false)); g.fillRect(0.f, 0.f, W * .4f, S);
        g.setGradientFill(juce::ColourGradient(C(0xff2ee6ff).withAlpha(a), W, S * .4f, C(0x002ee6ff), W * .65f, S * .4f, false)); g.fillRect(W * .6f, 0.f, W * .4f, S);
        g.setGradientFill(juce::ColourGradient(C(0x00000000), W * .5f, S * .5f, C(0x66000000), 0, 0, true)); g.fillRect(0.f, 0.f, W, S);
    }

    void title(juce::Graphics& g, float W, float t, const SceneState& s) {
        (void)W; juce::String a("REPARTO CHAIN");
        g.setFont(juce::FontOptions(40.f, juce::Font::bold | juce::Font::italic));
        for (int k = 3; k >= 1; --k) { g.setColour(C(0xffff3d9a).withAlpha(.10f * (.4f + s.neon))); for (int d = -k; d <= k; d += k) g.drawText(a, 26 + d, 14 + d, 420, 48, juce::Justification::left); }
        g.setGradientFill(juce::ColourGradient(C(0xffffe27a), 0, 14, C(0xffe29a00), 0, 60, false)); g.drawText(a, 26, 14, 420, 48, juce::Justification::left);
        (void)s; (void)t;
        IrekoLogo::drawPng(g, 26.f, 62.f, 36);
    }
};
