#pragma once
// Motor de audio original (sin JUCE). Cadena vocal + cadena master.
#include "Params.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cstring>

namespace rv {

constexpr double kPi = 3.14159265358979323846;
inline float dbToLin(float d) { return std::pow(10.f, d / 20.f); }
inline float linToDb(float l) { return 20.f * std::log10(std::max(l, 1e-9f)); }
// Aproximaciones rapidas (error < 0.01 dB) para el modo Lite
inline float fastLog2(float x) {
    union { float f; uint32_t i; } u{ x };
    float e = (float)((int)((u.i >> 23) & 255) - 127);
    u.i = (u.i & 0x007FFFFFu) | 0x3F800000u; float m = u.f;
    return e + ((((-7.915816145e-02f * m + 6.288736178e-01f) * m + -2.081214161e+00f) * m + 4.028547941e+00f) * m + -2.496846057e+00f);
}
inline float fastExp2(float x) {
    x = std::max(x, -120.f); float fl = std::floor(x), f = x - fl;
    float p = 1.f + f * (0.69314718f + f * (0.24022651f + f * (0.05550411f + f * 0.00961813f)));
    union { uint32_t i; float f; } u{ (uint32_t)((int)fl + 127) << 23 }; return p * u.f;
}
inline float fastLinToDb(float l) { return 6.0205999f * fastLog2(std::max(l, 1e-9f)); }
inline float fastDbToLin(float d) { return fastExp2(d * 0.16609640f); }

struct Biquad {
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    void reset() { z1 = z2 = 0; }
    float process(float x) {
        double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return (float)y;
    }
    void setRaw(double B0, double B1, double B2, double A0, double A1, double A2) {
        b0 = B0 / A0; b1 = B1 / A0; b2 = B2 / A0; a1 = A1 / A0; a2 = A2 / A0;
    }
    void highpass(double sr, double f, double q = 0.7071) {
        double w = 2 * kPi * f / sr, c = std::cos(w), al = std::sin(w) / (2 * q);
        setRaw((1 + c) / 2, -(1 + c), (1 + c) / 2, 1 + al, -2 * c, 1 - al);
    }
    void peaking(double sr, double f, double dB, double q = 1.0) {
        double A = std::pow(10.0, dB / 40), w = 2 * kPi * f / sr, c = std::cos(w), al = std::sin(w) / (2 * q);
        setRaw(1 + al * A, -2 * c, 1 - al * A, 1 + al / A, -2 * c, 1 - al / A);
    }
    void lowShelf(double sr, double f, double dB) {
        double A = std::pow(10.0, dB / 40), w = 2 * kPi * f / sr, c = std::cos(w);
        double al = std::sin(w) / 2 * std::sqrt(2.0), t = 2 * std::sqrt(A) * al;
        setRaw(A * ((A + 1) - (A - 1) * c + t), 2 * A * ((A - 1) - (A + 1) * c), A * ((A + 1) - (A - 1) * c - t),
               (A + 1) + (A - 1) * c + t, -2 * ((A - 1) + (A + 1) * c), (A + 1) + (A - 1) * c - t);
    }
    void highShelf(double sr, double f, double dB) {
        double A = std::pow(10.0, dB / 40), w = 2 * kPi * f / sr, c = std::cos(w);
        double al = std::sin(w) / 2 * std::sqrt(2.0), t = 2 * std::sqrt(A) * al;
        setRaw(A * ((A + 1) + (A - 1) * c + t), -2 * A * ((A - 1) + (A + 1) * c), A * ((A + 1) + (A - 1) * c - t),
               (A + 1) - (A - 1) * c + t, 2 * ((A - 1) - (A + 1) * c), (A + 1) - (A - 1) * c - t);
    }
};

// Compresor feed-forward con rodilla suave, enlace estereo
struct Comp {
    float thr = -18, ratio = 3, knee = 6, makeup = 0, aC = 0, rC = 0, env = 0; bool fast = false;
    void set(double sr, float t, float r, float atkMs, float relMs, float mk) {
        thr = t; ratio = std::max(1.f, r); makeup = mk;
        aC = (float)std::exp(-1.0 / (std::max(atkMs, 0.01f) * 0.001 * sr));
        rC = (float)std::exp(-1.0 / (std::max(relMs, 1.f) * 0.001 * sr));
    }
    void reset() { env = 0; }
    float gain(float absIn) {
        float over = (fast ? fastLinToDb(absIn) : linToDb(absIn)) - thr, gr;
        if (2 * over < -knee) gr = 0;
        else if (2 * std::fabs(over) <= knee) { float t = over + knee / 2; gr = (1 / ratio - 1) * t * t / (2 * knee); }
        else gr = (1 / ratio - 1) * over;
        float c = gr < env ? aC : rC;
        env = c * env + (1 - c) * gr;
        return fast ? fastDbToLin(env + makeup) : dbToLin(env + makeup);
    }
};

struct DeEsser {
    Biquad band[2][2];
    float env = 0, aC = 0, rC = 0, thr = -30, maxRed = 0; bool fast = false;
    void set(double sr, float f, float t, float amt) {
        for (int c = 0; c < 2; ++c) for (int s = 0; s < 2; ++s) band[c][s].highpass(sr, f, 0.7071);
        thr = t; maxRed = amt;
        aC = (float)std::exp(-1.0 / (0.0005 * sr)); rC = (float)std::exp(-1.0 / (0.03 * sr));
    }
    void reset() { for (auto& c : band) for (auto& b : c) b.reset(); env = 0; }
    void process(float& l, float& r) {
        if (maxRed <= 0.01f) { band[0][0].process(l); band[0][1].process(l); band[1][0].process(r); band[1][1].process(r); return; }
        float bl = band[0][1].process(band[0][0].process(l));
        float br = band[1][1].process(band[1][0].process(r));
        float pk = std::max(std::fabs(bl), std::fabs(br));
        env = pk > env ? aC * env + (1 - aC) * pk : rC * env + (1 - rC) * pk;
        float over = (fast ? fastLinToDb(env) : linToDb(env)) - thr;
        float redDb = over > 0 ? std::min(maxRed, over * 0.75f) : 0.f;
        float g = fast ? fastDbToLin(-redDb) : dbToLin(-redDb);
        l = l - bl + bl * g; r = r - br + br * g;
    }
};

inline float satTanh(float x, float drive) { return std::tanh(x * drive) / drive; }

struct Delay {
    std::vector<float> bl, br; int w = 0, n = 0; float lpL = 0, lpR = 0, lpC = 0.3f;
    void prepare(double sr) { n = (int)(sr * 1.0) + 8; bl.assign(n, 0.f); br.assign(n, 0.f); w = 0; lpL = lpR = 0; lpC = (float)(1 - std::exp(-2 * kPi * 5000 / sr)); }
    void process(float& l, float& r, double sr, float ms, float fb, float mix) {
        if (mix <= 0.0005f) { return; }
        int d = std::clamp((int)(ms * 0.001 * sr), 1, n - 1);
        int rd = (w - d + n) % n;
        float wl = bl[rd], wr = br[rd];
        lpL += lpC * (wl - lpL); lpR += lpC * (wr - lpR);
        float mono = 0.5f * (l + r);
        bl[w] = mono + fb * lpR; br[w] = fb * lpL;   // ping-pong
        w = (w + 1) % n;
        l += mix * wl; r += mix * wr;
    }
};

struct Reverb {
    struct Comb { std::vector<float> b; int i = 0; float st = 0;
        float run(float x, float fb, float damp) { float o = b[i]; st = o * (1 - damp) + st * damp; b[i] = x + st * fb; if (++i >= (int)b.size()) i = 0; return o; } };
    struct AP { std::vector<float> b; int i = 0;
        float run(float x) { float bo = b[i]; float o = -x + bo; b[i] = x + bo * 0.5f; if (++i >= (int)b.size()) i = 0; return o; } };
    Comb cL[8], cR[8]; AP aL[4], aR[4]; int nC = 8, nA = 4; float lvl = 1.f;
    std::vector<float> pre; int pw = 0; Biquad hp;
    void prepare(double sr, bool lite = false) {
        nC = lite ? 4 : 8; nA = lite ? 2 : 4; lvl = lite ? 1.9f : 1.f;
        static const int ct[8] = { 1116,1188,1277,1356,1422,1491,1557,1617 };
        static const int at[4] = { 556,441,341,225 };
        double k = sr / 44100.0;
        for (int i = 0; i < 8; ++i) { cL[i].b.assign((int)(ct[i] * k), 0.f); cR[i].b.assign((int)((ct[i] + 23) * k), 0.f); cL[i].i = cR[i].i = 0; cL[i].st = cR[i].st = 0; }
        for (int i = 0; i < 4; ++i) { aL[i].b.assign((int)(at[i] * k), 0.f); aR[i].b.assign((int)((at[i] + 23) * k), 0.f); aL[i].i = aR[i].i = 0; }
        pre.assign((int)(sr * 0.2) + 4, 0.f); pw = 0; hp.reset(); hp.highpass(sr, 250.0);
    }
    void process(float& l, float& r, double sr, float size, float damp, float preMs, float mix) {
        if (mix <= 0.0005f) return;
        float in = hp.process(0.5f * (l + r));
        int n = (int)pre.size();
        int d = std::clamp((int)(preMs * 0.001 * sr), 0, n - 1);
        pre[pw] = in; float pin = pre[(pw - d + n) % n]; pw = (pw + 1) % n;
        float fb = 0.7f + size * 0.28f, x = pin * 0.015f, wl = 0, wr = 0;
        for (int i = 0; i < nC; ++i) { wl += cL[i].run(x, fb, damp * 0.4f); wr += cR[i].run(x, fb, damp * 0.4f); }
        for (int i = 0; i < nA; ++i) { wl = aL[i].run(wl); wr = aR[i].run(wr); }
        l += mix * wl * 3.f * lvl; r += mix * wr * 3.f * lvl;
    }
};

// Limitador con lookahead (pico muestra a muestra, techo garantizado)
struct Limiter {
    int la = 1; std::vector<float> dl, dr, gt, ma; int w = 0, mw = 0; float env = 1, rel = 0.999f, sum = 0, ceil = 1;
    void prepare(double sr, bool lite = false) {
        la = std::max(2, (int)((lite ? 0.001 : 0.002) * sr));
        dl.assign(la + 1, 0.f); dr.assign(la + 1, 0.f); gt.assign(la + 1, 1.f); ma.assign(la, 1.f);
        w = mw = 0; env = 1; sum = (float)la; rel = (float)std::exp(-1.0 / (0.08 * sr));
    }
    void process(float& l, float& r, float ceiling) {
        ceil = ceiling;
        int n = la + 1;
        dl[w] = l; dr[w] = r;
        float pk = std::max(std::fabs(l), std::fabs(r));
        gt[w] = pk > ceil ? ceil / pk : 1.f;
        float mn = 1.f; for (int i = 0; i < n; ++i) mn = std::min(mn, gt[i]);
        env = mn < env ? mn : rel * env + (1 - rel) * mn;
        sum += env - ma[mw]; ma[mw] = env; mw = (mw + 1) % la;
        float g = sum / la;
        int rd = (w + 1) % n;                        // muestra mas antigua = retraso de 'la'
        l = std::clamp(dl[rd] * g, -ceil, ceil); r = std::clamp(dr[rd] * g, -ceil, ceil);
        w = (w + 1) % n;
    }
};

struct Engine {
    Params p, last; double sr = 44100; bool lite = false, haveLast = false;
    Biquad hpf[2][2], eq[4][2], mLow[2], mHigh[2], mEq[6][2], bmono, rvHp; DeEsser de; Comp vc, glue; Delay dly; Reverb rev; Limiter lim;
    int latency() const { return lim.la; }
    void prepare(double rate) {
        sr = rate; de.reset(); dly.prepare(sr); rev.prepare(sr, lite); lim.prepare(sr, lite); haveLast = false; vc.reset(); glue.reset();
        for (auto& a : hpf) for (auto& b : a) b.reset(); for (auto& a : eq) for (auto& b : a) b.reset();
        for (auto& a : mEq) for (auto& b : a) b.reset(); for (auto& b : mLow) b.reset(); for (auto& b : mHigh) b.reset(); bmono.reset();
        update();
    }
    void update() {   // llamar al inicio de cada bloque
        if (lite) {       // modo Lite: solo recalcula filtros si algun parametro cambio
            if (haveLast && std::memcmp(&p, &last, sizeof(Params)) == 0) return;
            last = p; haveLast = true; vc.fast = glue.fast = de.fast = true;
        }
        for (int c = 0; c < 2; ++c) {
            for (int s = 0; s < 2; ++s) hpf[c][s].highpass(sr, p.hpf);
            eq[0][c].lowShelf(sr, 100, p.eqLow); eq[1][c].peaking(sr, 300, p.eqMud, 1.0);
            eq[2][c].peaking(sr, 3500, p.eqPres, 0.9); eq[3][c].highShelf(sr, 10000, p.eqAir);
            mLow[c].lowShelf(sr, 80, p.mLow); mHigh[c].highShelf(sr, 9000, p.mHigh);
            const float f[6] = { 60.f, 175.f, 520.f, 1500.f, 3900.f, 9800.f };
            const float g[6] = { p.mEq1, p.mEq2, p.mEq3, p.mEq4, p.mEq5, p.mEq6 };
            for (int k = 0; k < 6; ++k) mEq[k][c].peaking(sr, f[k], g[k], 0.8);
        }
        de.set(sr, p.dsFreq, p.dsThresh, p.dsAmount);
        vc.set(sr, p.cThresh, p.cRatio, p.cAttack, p.cRelease, p.cMakeup);
        glue.set(sr, p.mGlueThr, p.mGlueRat, 30.f, 150.f, 0.f);
        bmono.highpass(sr, std::max(20.f, p.mBassMono), 0.7071);
    }
    void process(float* L, float* R, int n) {
        const bool vOn = p.vocalOn > 0.5f, mOn = p.masterOn > 0.5f;
        const float inG = dbToLin(p.inGain), vOutG = dbToLin(p.vOut), mOutG = dbToLin(p.mOut), drv = dbToLin(p.mDrive);
        const float satD = dbToLin(p.satDrive), ceil = dbToLin(p.mCeiling);
        for (int i = 0; i < n; ++i) {
            float l = L[i], r = R[i];
            if (vOn) {
                l *= inG; r *= inG;
                l = hpf[0][1].process(hpf[0][0].process(l)); r = hpf[1][1].process(hpf[1][0].process(r));
                de.process(l, r);
                for (int b = 0; b < 4; ++b) { l = eq[b][0].process(l); r = eq[b][1].process(r); }
                float g = vc.gain(std::max(std::fabs(l), std::fabs(r))); l *= g; r *= g;
                if (p.satMix > 0.001f) {
                    l += p.satMix * (satTanh(l, satD) - l); r += p.satMix * (satTanh(r, satD) - r);
                }
                dly.process(l, r, sr, p.dlyTime, p.dlyFb, p.dlyMix);
                rev.process(l, r, sr, p.revSize, p.revDamp, p.revPre, p.revMix);
                l *= vOutG; r *= vOutG;
            }
            if (mOn) {
                l = mHigh[0].process(mLow[0].process(l)); r = mHigh[1].process(mLow[1].process(r));
                for (int k = 0; k < 6; ++k) { l = mEq[k][0].process(l); r = mEq[k][1].process(r); }
                float g = glue.gain(std::max(std::fabs(l), std::fabs(r))); l *= g; r *= g;
                float m = 0.5f * (l + r), s = 0.5f * (l - r);
                if (p.mBassMono > 20.5f) s = bmono.process(s);
                s *= p.mWidth; l = m + s; r = m - s;
                l *= drv; r *= drv;
                if (p.mClip > 0.001f) {
                    auto sc = [](float x) { float a = std::fabs(x); if (a < 0.7f) return x; float y = 0.7f + 0.3f * std::tanh((a - 0.7f) / 0.3f); return x < 0 ? -y : y; };
                    l += p.mClip * (sc(l) - l); r += p.mClip * (sc(r) - r);
                }
                lim.process(l, r, ceil);
                l *= mOutG; r *= mOutG;
            } else {
                lim.process(l, r, 1.0e6f);   // mantiene latencia constante
            }
            L[i] = l; R[i] = r;
        }
    }
};

} // namespace rv
