#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "Identifiers/ParamIDs.h"

class EONisyaAudioProcessor : public juce::AudioProcessor,
                              public juce::AudioProcessorValueTreeState::Listener
{
public:
    EONisyaAudioProcessor();
    ~EONisyaAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override { return "EON MOTIF6"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Steel Cathedral"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    const juce::AudioProcessorValueTreeState& getAPVTS() const { return apvts; }

    // Live performance meters shown by the panel
    int getActiveVoiceCount() const noexcept { return activeVoices; }
    float getPeakLevel() const noexcept { return outputPeak; }

    void parameterChanged (const juce::String& parameterID, float newValue) override;

private:
    struct Voice
    {
        double phase[4] = {};
        double phaseInc[4] = {};
        double ampEnv = 0.0;
        double fegEnv = 0.0;
        int fegStage = 0;   // 0 idle, 1 attack, 2 decay, 3 sustain, 4 release
        double filterState = 0.0;
        double filterState2 = 0.0;
        double level[4] = { 0.9, 0.45, 0.22, 0.18 };
        double detune[4] = { 0.0, -7.0, 5.0, 12.0 };
        bool active = false;
        double note = 0.0;
        int midiNote = 60;
        float velocity = 0.5f;
    };

    static constexpr int maxVoices = 128;
    Voice voices[maxVoices];
    double currentSampleRate = 44100.0;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<int> activeVoices { 0 };
    std::atomic<float> outputPeak { 0.0f };

    // Lightweight reverb + chorus send buses
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    bool reverbReady = false;
    double reverbMix = 0.25;

    juce::dsp::Chorus<float> chorus;
    bool chorusReady = false;
    void applyChorusSettings();

    juce::AudioBuffer<float> dryScratch;
    juce::AudioBuffer<float> fxScratch;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EONisyaAudioProcessor)
};
