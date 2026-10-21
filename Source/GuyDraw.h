#pragma once
// Dibujo del repartero (personaje ORIGINAL) para la version Lite. Copia fiel del de la escena completa.
#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>
namespace GuyDraw {
using C = juce::Colour;
struct Guy { float ox; C skin, cap, hood, pants, shoe; bool shades, beanie; };
// el de la camiseta (sudadera) negra y gorro naranja
inline const Guy blackShirt{ -.10f, C(0xff8d5524), C(0xfff26b1d), C(0xff17171c), C(0xff2a2d31), C(0xfff26b1d), false, true };
inline void person(juce::Graphics& g, float fx, float fy, float sc, const Guy& p, float t, int idx, float bob, float wp, float face) {
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
    (void)t;
}


}
