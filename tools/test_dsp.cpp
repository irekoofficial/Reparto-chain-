// Prueba offline: lee f32le estereo 44.1k, aplica un preset, escribe f32le.
#include "../Source/Dsp.h"
#include "../Source/Presets.h"
#include <cstdio>
#include <cstdlib>
int main(int argc, char** argv) {
    if (argc < 5) { std::fprintf(stderr, "uso: test in.raw out.raw preset_idx pre_gain_db\n"); return 1; }
    FILE* f = std::fopen(argv[1], "rb"); std::vector<float> d; float t[4096];
    size_t k; while ((k = std::fread(t, 4, 4096, f)) > 0) d.insert(d.end(), t, t + k); std::fclose(f);
    int n = (int)(d.size() / 2); std::vector<float> L(n), R(n);
    float g = rv::dbToLin((float)std::atof(argv[4]));
    for (int i = 0; i < n; ++i) { L[i] = d[2 * i] * g; R[i] = d[2 * i + 1] * g; }
    rv::Engine e; rvApplyPreset(e.p, std::atoi(argv[3]));
    for (int a = 5; a < argc; ++a) { std::string s = argv[a]; auto q = s.find('='); if (q != std::string::npos) e.p.set(s.substr(0, q), (float)std::atof(s.substr(q + 1).c_str())); }
    e.prepare(44100);
    for (int i = 0; i < n; i += 512) { e.update(); e.process(&L[i], &R[i], std::min(512, n - i)); }
    std::vector<float> o(d.size()); int lat = e.latency();
    for (int i = 0; i < n; ++i) { int j = i + lat < n ? i + lat : n - 1; o[2 * i] = L[j]; o[2 * i + 1] = R[j]; }
    f = std::fopen(argv[2], "wb"); std::fwrite(o.data(), 4, o.size(), f); std::fclose(f);
}
