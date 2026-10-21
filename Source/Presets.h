#pragma once
#include <vector>
#include <utility>
#include "Params.h"
#include <string>

struct PresetDef { const char* name; std::vector<std::pair<std::string, float>> v; };

// Los valores no listados vuelven al defecto del parametro.
inline const std::vector<PresetDef>& rvPresets() {
    static const std::vector<PresetDef> P = {
        { "Init (todo neutro)", { {"vocalOn",1},{"masterOn",0} } },

        { "Reparto - Voz Principal", {
            {"vocalOn",1},{"masterOn",0},
            {"hpf",90},{"dsFreq",6500},{"dsThresh",-30},{"dsAmount",6},
            {"cThresh",-20},{"cRatio",4},{"cAttack",5},{"cRelease",80},{"cMakeup",4},
            {"eqLow",1.5},{"eqMud",-2.5},{"eqPres",3},{"eqAir",3},
            {"satDrive",6},{"satMix",0.35},
            {"dlyTime",300},{"dlyFb",0.25},{"dlyMix",0.08},
            {"revSize",0.4},{"revDamp",0.5},{"revPre",25},{"revMix",0.08} } },

        { "Reparto - Agresivo (Trap)", {
            {"vocalOn",1},{"masterOn",0},
            {"hpf",110},{"dsFreq",7000},{"dsThresh",-34},{"dsAmount",8},
            {"cThresh",-26},{"cRatio",6},{"cAttack",2},{"cRelease",60},{"cMakeup",7},
            {"eqLow",2},{"eqMud",-3.5},{"eqPres",4.5},{"eqAir",4},
            {"satDrive",12},{"satMix",0.55},
            {"dlyTime",230},{"dlyFb",0.3},{"dlyMix",0.12},
            {"revSize",0.3},{"revDamp",0.6},{"revPre",15},{"revMix",0.05} } },

        { "Reparto - Coros / Doble", {
            {"vocalOn",1},{"masterOn",0},
            {"hpf",150},{"dsFreq",6500},{"dsThresh",-32},{"dsAmount",6},
            {"cThresh",-22},{"cRatio",4},{"cAttack",8},{"cRelease",100},{"cMakeup",3},
            {"eqLow",-2},{"eqMud",-3},{"eqPres",2},{"eqAir",2},
            {"satDrive",4},{"satMix",0.2},
            {"dlyTime",360},{"dlyFb",0.4},{"dlyMix",0.2},
            {"revSize",0.65},{"revDamp",0.55},{"revPre",35},{"revMix",0.22},
            {"vOut",-3} } },

        { "Voz Cantada Limpia", {
            {"vocalOn",1},{"masterOn",0},
            {"hpf",80},{"dsFreq",6000},{"dsThresh",-32},{"dsAmount",5},
            {"cThresh",-18},{"cRatio",2.5},{"cAttack",12},{"cRelease",150},{"cMakeup",2},
            {"eqLow",1},{"eqMud",-2},{"eqPres",2},{"eqAir",3.5},
            {"satDrive",3},{"satMix",0.15},
            {"dlyMix",0.06},{"revSize",0.6},{"revDamp",0.45},{"revPre",30},{"revMix",0.15} } },

        // Masters calibrados con el analisis de cada tema de referencia (volumen, EQ de 6 bandas,
        // ancho estereo, bajos en mono, compresion y clipper). Un preset por referencia.
        { "Master estilo Honda", { {"mEq1",-1.1},{"mEq2",-1.5},{"mEq3",-0.1},{"mEq4",1.1},{"mEq5",1.1},{"mEq6",0.5},{"mWidth",1.02},{"mBassMono",107},{"mClip",0.6},{"mGlueRat",1.6},{"mGlueThr",-14},{"mCeiling",-0.3},{"vocalOn",0},{"masterOn",1},{"mDrive",11.1} } },

        { "Master estilo Befocus 1", { {"mEq1",0.6},{"mEq2",-0.4},{"mEq3",0.5},{"mEq4",0.5},{"mEq5",0.2},{"mEq6",-1.4},{"mWidth",1.21},{"mBassMono",118},{"mClip",0.25},{"mGlueRat",1.5},{"mGlueThr",-14},{"mCeiling",-0.3},{"vocalOn",0},{"masterOn",1},{"mDrive",7.5} } },

        { "Master estilo Befocus 2", { {"mEq1",0},{"mEq2",0.6},{"mEq3",0.1},{"mEq4",-0},{"mEq5",-0.6},{"mEq6",-0.1},{"mWidth",1.3},{"mBassMono",63},{"mClip",0.41},{"mGlueRat",1.5},{"mGlueThr",-14},{"mCeiling",-0.3},{"vocalOn",0},{"masterOn",1},{"mDrive",10} } },

        { "Master estilo Dichavate", { {"mEq1",0.8},{"mEq2",1.5},{"mEq3",0.8},{"mEq4",-0.7},{"mEq5",-0.5},{"mEq6",-1.8},{"mWidth",1.11},{"mBassMono",76},{"mClip",0.52},{"mGlueRat",1.8},{"mGlueThr",-14},{"mCeiling",-0.3},{"vocalOn",0},{"masterOn",1},{"mDrive",9.8} } },

        { "Master estilo El Bandolero", { {"mEq1",1.9},{"mEq2",-0.4},{"mEq3",-1.3},{"mEq4",-1.2},{"mEq5",0.3},{"mEq6",0.7},{"mWidth",0.94},{"mBassMono",127},{"mClip",0.3},{"mGlueRat",1.6},{"mGlueThr",-14},{"mCeiling",-0.3},{"vocalOn",0},{"masterOn",1},{"mDrive",8.7} } },

        { "Master estilo Mauro El Codigo Secreto", { {"mEq1",0.9},{"mEq2",0.8},{"mEq3",0.5},{"mEq4",0.3},{"mEq5",-1.8},{"mEq6",-0.8},{"mWidth",1.21},{"mBassMono",46},{"mClip",0.26},{"mGlueRat",1.8},{"mGlueThr",-14},{"mCeiling",-0.3},{"vocalOn",0},{"masterOn",1},{"mDrive",6.8} } },

        { "Master estilo Jefe White", { {"mEq1",-4},{"mEq2",-1.3},{"mEq3",0.4},{"mEq4",1.2},{"mEq5",2},{"mEq6",3.6},{"mWidth",0.76},{"mBassMono",134},{"mClip",0.28},{"mGlueRat",1.7},{"mGlueThr",-14},{"mCeiling",-0.3},{"vocalOn",0},{"masterOn",1},{"mDrive",9.6} } },

        { "Master estilo Dave Produce", { {"mEq1",2.7},{"mEq2",0.7},{"mEq3",-0.8},{"mEq4",-1.2},{"mEq5",-0.7},{"mEq6",-0.7},{"mWidth",0.75},{"mBassMono",160},{"mClip",0.29},{"mGlueRat",2.2},{"mGlueThr",-14},{"mCeiling",-0.3},{"vocalOn",0},{"masterOn",1},{"mDrive",9.5} } },

        { "Master - Suave (-9 LUFS aprox)", {
            {"vocalOn",0},{"masterOn",1},
            {"mLow",0.5},{"mHigh",0.5},
            {"mGlueThr",-12},{"mGlueRat",1.6},
            {"mWidth",1.05},{"mBassMono",100},
            {"mDrive",4},{"mClip",0.3},{"mCeiling",-0.5},{"mOut",0} } },

        { "Voz Reparto + Master estilo Honda", {
            {"vocalOn",1},{"masterOn",1},
            {"hpf",90},{"dsFreq",6500},{"dsThresh",-30},{"dsAmount",6},
            {"cThresh",-20},{"cRatio",4},{"cAttack",5},{"cRelease",80},{"cMakeup",4},
            {"eqLow",1.5},{"eqMud",-2.5},{"eqPres",3},{"eqAir",3},
            {"satDrive",6},{"satMix",0.35},
            {"dlyTime",300},{"dlyFb",0.25},{"dlyMix",0.08},
            {"revSize",0.4},{"revDamp",0.5},{"revPre",25},{"revMix",0.08},
            {"masterOn",1},{"mEq1",-1.1},{"mEq2",-1.5},{"mEq3",-0.1},{"mEq4",1.1},{"mEq5",1.1},{"mEq6",0.5},{"mWidth",1.02},{"mBassMono",107},{"mClip",0.6},{"mGlueRat",1.6},{"mGlueThr",-14},{"mCeiling",-0.3},{"mDrive",11.1} } },
    };
    return P;
}

inline void rvApplyPreset(Params& p, int idx) {
    p = Params();
    const auto& P = rvPresets();
    if (idx < 0 || idx >= (int)P.size()) return;
    for (auto& kv : P[idx].v) p.set(kv.first, kv.second);
}
