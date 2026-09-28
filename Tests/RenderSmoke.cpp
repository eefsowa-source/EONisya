// Offline render smoke test: instantiate the plugin, feed MIDI notes,
// and verify the output is finite, non-silent, and decays on release.

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_events/juce_events.h>
#include <cmath>
#include <cstdio>

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

int main (int argc, char* argv[])
{
    juce::MessageManager::getInstance();

    const juce::File outFile (argc > 1 ? juce::String (argv[1])
                                      : juce::File::getCurrentWorkingDirectory()
                                            .getChildFile ("render_smoke.wav"));

    const double sampleRate = 44100.0;
    const int blockSize = 512;
    const int heldBlocks = 172;   // ~2.0 s with the note held
    const int tailBlocks = 172;   // ~2.0 s after note-off

    std::unique_ptr<juce::AudioProcessor> proc (createPluginFilter());
    proc->setNonRealtime (true);
    proc->setRateAndBufferSizeDetails (sampleRate, blockSize);
    proc->prepareToPlay (sampleRate, blockSize);

    juce::AudioBuffer<float> buffer (2, blockSize);
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> rendered (2, (heldBlocks + tailBlocks) * blockSize);
    rendered.clear();

    const int tailMeasureStart = heldBlocks + tailBlocks - 43;  // last ~0.5 s

    bool allFinite = true;
    double heldPeak = 0.0, tailPeak = 0.0;

    for (int b = 0; b < heldBlocks + tailBlocks; ++b)
    {
        midi.clear();
        buffer.clear();

        if (b == 0)
            midi.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0);
        if (b == heldBlocks)
            midi.addEvent (juce::MidiMessage::noteOff (1, 60, 0.0f), 0);

        proc->processBlock (buffer, midi);

        rendered.copyFrom (0, b * blockSize, buffer, 0, 0, blockSize);
        rendered.copyFrom (1, b * blockSize, buffer, 1, 0, blockSize);

        for (int c = 0; c < 2; ++c)
        {
            const auto* d = buffer.getReadPointer (c);
            for (int i = 0; i < blockSize; ++i)
            {
                if (! std::isfinite (d[i]))
                    allFinite = false;
                const double a = std::abs ((double) d[i]);
                if (b < heldBlocks)               heldPeak = juce::jmax (heldPeak, a);
                else if (b >= tailMeasureStart)   tailPeak = juce::jmax (tailPeak, a);
            }
        }
    }

    proc->releaseResources();

    if (outFile.create().wasOk())
    {
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::OutputStream> os = outFile.createOutputStream();
        if (os != nullptr)
        {
            const auto options = juce::AudioFormatWriterOptions()
                                     .withSampleRate (sampleRate)
                                     .withNumChannels (2)
                                     .withBitsPerSample (16);
            std::unique_ptr<juce::AudioFormatWriter> writer (
                wav.createWriterFor (os, options));
            if (writer != nullptr)
                writer->writeFromAudioSampleBuffer (rendered, 0, rendered.getNumSamples());
        }
    }

    std::printf ("held peak: %.4f   tail peak: %.4f   output: %s\n",
                 heldPeak, tailPeak, outFile.getFullPathName().toRawUTF8());

    int failures = 0;
    if (! allFinite)                     { std::puts ("FAIL: non-finite samples"); ++failures; }
    if (! (heldPeak > 1e-3))             { std::puts ("FAIL: held note is silent"); ++failures; }
    if (! (tailPeak < heldPeak * 0.3))   { std::puts ("FAIL: no decay after note-off"); ++failures; }

    if (failures == 0)
        std::puts ("PASS");

    juce::DeletedAtShutdown::deleteAll();
    juce::MessageManager::deleteInstance();
    return failures == 0 ? 0 : 1;
}
