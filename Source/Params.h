#pragma once
// Lista unica de parametros (X-macro): id, nombre, min, max, defecto, skew, seccion
#include <string>
#include <cstring>

#define RV_PARAMS(X) \
 X(inGain,   "Input Gain dB",   -24.f, 24.f,  0.f,   1.f, 0) \
 X(hpf,      "Low Cut Hz",       20.f, 300.f, 80.f,  0.5f, 0) \
 X(dsFreq,   "De-Ess Freq Hz", 3000.f,10000.f,6500.f,0.6f, 1) \
 X(dsThresh, "De-Ess Thresh dB",-50.f, 0.f,  -30.f,  1.f, 1) \
 X(dsAmount, "De-Ess Max dB",     0.f, 12.f,  0.f,   1.f, 1) \
 X(cThresh,  "Comp Thresh dB",  -40.f, 0.f,  -18.f,  1.f, 2) \
 X(cRatio,   "Comp Ratio",        1.f, 10.f,  3.f,   0.6f,2) \
 X(cAttack,  "Comp Attack ms",   0.1f, 50.f,  8.f,   0.5f,2) \
 X(cRelease, "Comp Release ms",  10.f, 500.f,100.f,  0.5f,2) \
 X(cMakeup,  "Comp Makeup dB",    0.f, 18.f,  0.f,   1.f, 2) \
 X(eqLow,    "EQ Cuerpo 100Hz",  -9.f, 9.f,   0.f,   1.f, 3) \
 X(eqMud,    "EQ Barro 300Hz",   -9.f, 9.f,   0.f,   1.f, 3) \
 X(eqPres,   "EQ Presencia 3.5k",-9.f, 9.f,   0.f,   1.f, 3) \
 X(eqAir,    "EQ Aire 10k",      -9.f, 9.f,   0.f,   1.f, 3) \
 X(satDrive, "Saturacion Drive dB",0.f,24.f,  0.f,   1.f, 4) \
 X(satMix,   "Saturacion Mix",    0.f, 1.f,   0.f,   1.f, 4) \
 X(dlyTime,  "Delay Time ms",    40.f, 800.f,280.f,  0.6f,5) \
 X(dlyFb,    "Delay Feedback",    0.f, 0.9f,  0.3f,  1.f, 5) \
 X(dlyMix,   "Delay Mix",         0.f, 1.f,   0.f,   1.f, 5) \
 X(revSize,  "Reverb Size",       0.f, 1.f,   0.5f,  1.f, 5) \
 X(revDamp,  "Reverb Damp",       0.f, 1.f,   0.5f,  1.f, 5) \
 X(revPre,   "Reverb PreDelay ms",0.f, 150.f, 20.f,  1.f, 5) \
 X(revMix,   "Reverb Mix",        0.f, 1.f,   0.f,   1.f, 5) \
 X(vOut,     "Vocal Out dB",    -24.f, 12.f,  0.f,   1.f, 6) \
 X(mLow,     "Master Low 80Hz",  -6.f, 6.f,   0.f,   1.f, 7) \
 X(mHigh,    "Master High 9k",   -6.f, 6.f,   0.f,   1.f, 7) \
 X(mEq1,     "M-EQ 60Hz",        -6.f, 6.f,   0.f,   1.f, 8) \
 X(mEq2,     "M-EQ 175Hz",       -6.f, 6.f,   0.f,   1.f, 8) \
 X(mEq3,     "M-EQ 520Hz",       -6.f, 6.f,   0.f,   1.f, 8) \
 X(mEq4,     "M-EQ 1.5kHz",      -6.f, 6.f,   0.f,   1.f, 8) \
 X(mEq5,     "M-EQ 3.9kHz",      -6.f, 6.f,   0.f,   1.f, 8) \
 X(mEq6,     "M-EQ 9.8kHz",      -6.f, 6.f,   0.f,   1.f, 8) \
 X(mGlueThr, "Glue Thresh dB",  -30.f, 0.f,  -12.f,  1.f, 7) \
 X(mGlueRat, "Glue Ratio",        1.f, 4.f,   1.5f,  1.f, 7) \
 X(mWidth,   "Ancho Estereo",     0.f, 2.f,   1.f,   1.f, 7) \
 X(mBassMono,"Bajos Mono Hz",     0.f, 300.f,100.f,  1.f, 7) \
 X(mDrive,   "Master Drive dB",   0.f, 18.f,  0.f,   1.f, 7) \
 X(mClip,    "Soft Clip Mix",     0.f, 1.f,   0.f,   1.f, 7) \
 X(mCeiling, "Techo dB",         -3.f, 0.f,  -0.3f,  1.f, 7) \
 X(mOut,     "Master Out dB",   -12.f, 6.f,   0.f,   1.f, 7)

#define RV_TOGGLES(X) \
 X(vocalOn,  "Cadena Vocal", 1.f) \
 X(masterOn, "Cadena Master", 0.f)

static const char* const RV_SECTIONS[] = {
 "ENTRADA","DE-ESSER","COMPRESOR","EQ","SATURACION","ESPACIO (ECO / REVERB)","SALIDA VOCAL","MASTER","MASTER EQ (6 BANDAS)" };

struct Params {
#define X(id,n,mn,mx,df,sk,sec) float id = df;
 RV_PARAMS(X)
#undef X
#define X(id,n,df) float id = df;
 RV_TOGGLES(X)
#undef X
 bool set(const std::string& name, float v) {
#define X(id,n,mn,mx,df,sk,sec) if (name == #id) { id = v; return true; }
  RV_PARAMS(X)
#undef X
#define X(id,n,df) if (name == #id) { id = v; return true; }
  RV_TOGGLES(X)
#undef X
  return false;
 }
};
