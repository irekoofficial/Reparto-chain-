// Genera capturas reales de la interfaz en macOS (CoreGraphics) para revisar como se ve.
#include "PluginProcessor.h"
#ifdef RV_LITE
 #include "LiteEditor.h"
#else
 #include "PluginEditor.h"
#endif
int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI init;
    RVProcessor p; p.setCurrentProgram(1); p.meterLevel.store(0.45f);
    std::unique_ptr<juce::AudioProcessorEditor> ed(p.createEditor());
    juce::MessageManager::getInstance()->runDispatchLoopUntil(600);
    auto img = ed->createComponentSnapshot(ed->getLocalBounds(), true, 2.0f);
    juce::File f(argc > 1 ? argv[1] : "preview.png"); f.deleteFile(); juce::FileOutputStream o(f);
    juce::PNGImageFormat().writeImageToStream(img, o);
    printf("ok %dx%d\n", ed->getWidth(), ed->getHeight());
    return 0;
}
