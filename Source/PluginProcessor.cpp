#include "PluginProcessor.h"

#include <algorithm>

// ── Helpers ─────────────────────────────────────────────────────────────
static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;

    auto add = [] (AudioProcessorValueTreeState::ParameterLayout& layout,
                   const String& id, const String& name, float min, float max, float def,
                   const String& label = {}, float step = 0.01f)
    {
        layout.add (std::make_unique<AudioParameterFloat> (id, name,
                                                           NormalisableRange<float> (min, max, step),
                                                           def, label));
    };

    auto addInt = [] (AudioProcessorValueTreeState::ParameterLayout& layout,
                      const String& id, const String& name, int min, int max, int def,
                      const String& label = {})
    {
        layout.add (std::make_unique<AudioParameterInt> (id, name, min, max, def, label));
    };

    AudioProcessorValueTreeState::ParameterLayout layout;

    addInt   (layout, ParamIDs::page, "Page", 0, 2, 0);

    // Assignable knobs (set 2 — filter / EG)
    add      (layout, ParamIDs::cutoff,   "Cutoff",    20.0f,  20000.0f, 5400.0f,  "Hz");
    add      (layout, ParamIDs::reso,     "Resonance", 0.0f,   1.0f,    0.32f);
    add      (layout, ParamIDs::attack,   "Attack",    0.001f, 5.0f,    0.08f,   "s");
    add      (layout, ParamIDs::release,  "Release",   0.005f, 8.0f,    0.62f,   "s");
    add      (layout, ParamIDs::assign1,  "Assign 1",  0.0f,   1.0f,    0.25f);
    add      (layout, ParamIDs::assign2,  "Assign 2",  0.0f,   1.0f,    0.5f);
    add      (layout, ParamIDs::reverb,   "Reverb",    0.0f,   1.0f,    0.25f);
    add      (layout, ParamIDs::chorus,   "Chorus",    0.0f,   1.0f,    0.12f);

    // Voice edit knobs
    add      (layout, ParamIDs::drive,     "Drive",     0.0f, 1.0f, 0.35f);
    add      (layout, ParamIDs::keyFollow, "Key Follow",0.0f, 1.0f, 0.6f);
    add      (layout, ParamIDs::fegAtk,    "FEG Attack",0.001f, 5.0f, 0.05f, "s");
    add      (layout, ParamIDs::fegDcy,    "FEG Decay", 0.005f, 8.0f, 1.2f,  "s");
    add      (layout, ParamIDs::fegSus,    "FEG Sustain",0.0f, 1.0f, 0.6f);
    add      (layout, ParamIDs::fegRel,    "FEG Release",0.005f, 8.0f, 0.9f, "s");

    // Mixing knobs
    add      (layout, ParamIDs::volume, "Volume", -60.0f, 6.0f, -3.0f, "dB");
    add      (layout, ParamIDs::pan,    "Pan",    -1.0f,  1.0f, 0.0f);
    add      (layout, ParamIDs::revSend, "Reverb Send", 0.0f, 1.0f, 0.22f);
    add      (layout, ParamIDs::choSend, "Chorus Send", 0.0f, 1.0f, 0.1f);

    // Status / transport
    add      (layout, ParamIDs::tempo,      "Tempo",      40.0f, 300.0f, 112.0f, "BPM", 0.1f);
    addInt   (layout, ParamIDs::masterLevel,"Master Level",0, 127, 108);

    return layout;
}

// ── Construction ────────────────────────────────────────────────────────
EONisyaAudioProcessor::EONisyaAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "EONisyaParams", createParameterLayout())
{
    for (auto& v : voices)
        v = {};

    apvts.addParameterListener (ParamIDs::reverb, this);
    apvts.addParameterListener (ParamIDs::tempo, this);
    apvts.addParameterListener (ParamIDs::chorus, this);

    reverbParams.roomSize   = 0.6f;
    reverbParams.damping    = 0.5f;
    reverbParams.wetLevel   = 0.9f;
    reverbParams.dryLevel   = 0.3f;
    reverbParams.width      = 1.0f;
    reverb.setParameters (reverbParams);
}

EONisyaAudioProcessor::~EONisyaAudioProcessor()
{
    apvts.removeParameterListener (ParamIDs::reverb, this);
    apvts.removeParameterListener (ParamIDs::tempo, this);
    apvts.removeParameterListener (ParamIDs::chorus, this);
}

// ── Audio lifecycle ─────────────────────────────────────────────────────
void EONisyaAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    for (auto& v : voices)
    {
        v.active = false;
        v.ampEnv = 0.0;
    }

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = juce::jmax (1, samplesPerBlock);
    spec.numChannels = 2;
    reverb.prepare (spec);
    reverbReady = true;

    chorus.prepare (spec);
    applyChorusSettings();
    chorusReady = true;

    dryScratch.setSize (2, samplesPerBlock, false, true, true);
    fxScratch.setSize (2, samplesPerBlock, false, true, true);

    // Stereo dry + one mono reverb tap
    setLatencySamples (0);
}

void EONisyaAudioProcessor::releaseResources()
{
    reverbReady = false;
    chorusReady = false;
}

bool EONisyaAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled();
}

// ── Processing ──────────────────────────────────────────────────────────
void EONisyaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const auto numSamples = buffer.getNumSamples();
    const float sampleRate = static_cast<float> (currentSampleRate);

    auto& cutoffParam  = *apvts.getRawParameterValue (ParamIDs::cutoff);
    auto& resoParam    = *apvts.getRawParameterValue (ParamIDs::reso);
    auto& attackParam  = *apvts.getRawParameterValue (ParamIDs::attack);
    auto& releaseParam = *apvts.getRawParameterValue (ParamIDs::release);
    auto& driveParam   = *apvts.getRawParameterValue (ParamIDs::drive);
    auto& keyFollow    = *apvts.getRawParameterValue (ParamIDs::keyFollow);
    auto& fegAtkParam  = *apvts.getRawParameterValue (ParamIDs::fegAtk);
    auto& fegDcyParam  = *apvts.getRawParameterValue (ParamIDs::fegDcy);
    auto& fegSusParam  = *apvts.getRawParameterValue (ParamIDs::fegSus);
    auto& fegRelParam  = *apvts.getRawParameterValue (ParamIDs::fegRel);
    auto& volParam     = *apvts.getRawParameterValue (ParamIDs::volume);
    auto& panParam     = *apvts.getRawParameterValue (ParamIDs::pan);
    auto& revSendParam = *apvts.getRawParameterValue (ParamIDs::revSend);
    auto& choSendParam = *apvts.getRawParameterValue (ParamIDs::choSend);
    auto& masterParam  = *apvts.getRawParameterValue (ParamIDs::masterLevel);

    auto* left  = buffer.getWritePointer (0);
    auto* right = buffer.getWritePointer (1);
    buffer.clear();

    // Allocate / release voices from MIDI
    juce::MidiBuffer::Iterator midiIt (midiMessages);
    juce::MidiMessage msg;
    int midiPos = 0;
    while (midiIt.getNextEvent (msg, midiPos))
    {
        if (msg.isNoteOn())
        {
            Voice* slot = nullptr;
            for (auto& v : voices)
            {
                if (! v.active) { slot = &v; break; }
            }
            if (slot == nullptr)
            {
                // Steal the quietest voice
                auto* quietest = &voices[0];
                for (auto& v : voices)
                    if (v.ampEnv < quietest->ampEnv) quietest = &v;
                slot = quietest;
            }

            slot->active = true;
            slot->midiNote = msg.getNoteNumber();
            slot->velocity = static_cast<float> (msg.getVelocity()) / 127.0f;
            slot->note = juce::MidiMessage::getMidiNoteInHertz (slot->midiNote);
            slot->ampEnv = 0.0;
            slot->fegEnv = 0.0;
            slot->fegStage = 1;
            slot->filterState = 0.0;
            slot->filterState2 = 0.0;

            for (int i = 0; i < 4; ++i)
            {
                const double detuneFactor = std::pow (2.0, slot->detune[i] / 1200.0);
                slot->phase[i] = 0.0;
                slot->phaseInc[i] = slot->note * detuneFactor / currentSampleRate;
            }
        }
        else if (msg.isNoteOff())
        {
            for (auto& v : voices)
                if (v.midiNote == msg.getNoteNumber())
                {
                    v.active = false;
                    v.fegStage = 4;
                }
        }
    }

    const double baseCutoff = static_cast<double> (cutoffParam.load());
    const double svfDamping = 1.0 / (0.5 + static_cast<double> (resoParam.load()) * 8.0);
    const double ampAttack  = juce::jmax (0.0005, 1.0 - std::exp (-1.0 / (static_cast<double> (attackParam.load()) * sampleRate + 1.0)));
    const double ampRelease = juce::jmax (0.0002, 1.0 - std::exp (-1.0 / (static_cast<double> (releaseParam.load()) * sampleRate + 1.0)));
    const double fegAttack  = juce::jmax (0.0005, 1.0 - std::exp (-1.0 / (static_cast<double> (fegAtkParam.load()) * sampleRate + 1.0)));
    const double fegDecay  = juce::jmax (0.0002, 1.0 - std::exp (-1.0 / (static_cast<double> (fegDcyParam.load()) * sampleRate + 1.0)));
    const double fegSus    = static_cast<double> (fegSusParam.load());
    const double fegRel    = juce::jmax (0.0002, 1.0 - std::exp (-1.0 / (static_cast<double> (fegRelParam.load()) * sampleRate + 1.0)));
    const double keyFollowAmt = static_cast<double> (keyFollow.load());
    const double drive     = static_cast<double> (driveParam.load()) * 4.0 + 1.0;
    const double ampGain   = juce::Decibels::decibelsToGain (static_cast<double> (volParam.load()));
    const double panLeft   = juce::jmin (1.0, juce::jmax (0.0, 1.0 - static_cast<double> (panParam.load()))) * 0.707;
    const double panRight  = juce::jmin (1.0, juce::jmax (0.0, 1.0 + static_cast<double> (panParam.load()))) * 0.707;
    const double masterGain = static_cast<double> (masterParam.load()) / 127.0;

    int activeCount = 0;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        double outL = 0.0, outR = 0.0;

        for (auto& v : voices)
        {
            if (! v.active && v.ampEnv < 1e-4)
                continue;

            // Filter envelope (FEG): attack → decay → sustain → release
            switch (v.fegStage)
            {
                case 1:
                    v.fegEnv += (1.0 - v.fegEnv) * fegAttack;
                    if (v.fegEnv >= 0.999) { v.fegEnv = 1.0; v.fegStage = 2; }
                    break;
                case 2:
                    v.fegEnv += (fegSus - v.fegEnv) * fegDecay;
                    if (std::abs (v.fegEnv - fegSus) < 1e-3) { v.fegEnv = fegSus; v.fegStage = 3; }
                    break;
                case 4:
                    v.fegEnv -= v.fegEnv * fegRel;
                    if (v.fegEnv < 1e-4) { v.fegEnv = 0.0; v.fegStage = 0; }
                    break;
                default:
                    break;
            }

            double filterCut = baseCutoff * (1.0 + v.fegEnv * 2.0);
            if (keyFollowAmt > 0.0)
                filterCut *= std::pow (2.0, (v.midiNote - 60) / 12.0 * keyFollowAmt * 1.5);
            filterCut = juce::jmin (filterCut, sampleRate * 0.45);
            const double g = std::tan (juce::MathConstants<double>::pi * filterCut / sampleRate);

            double sampleValue = 0.0;
            for (int i = 0; i < 4; ++i)
            {
                double wave = std::sin (v.phase[i] * juce::MathConstants<double>::twoPi);
                wave += 0.35 * std::sin (2.0 * v.phase[i] * juce::MathConstants<double>::twoPi);
                wave = std::tanh (wave * 1.4);
                v.phase[i] += v.phaseInc[i];
                if (v.phase[i] >= 1.0) v.phase[i] -= 1.0;
                sampleValue += wave * v.level[i];
            }

            // Amp envelope: attack toward 1 while held, release on note-off
            if (v.active)
            {
                v.ampEnv = v.ampEnv * (1.0 - ampAttack) + 1.0 * ampAttack;
            }
            else
            {
                v.ampEnv -= v.ampEnv * ampRelease;
                if (v.ampEnv < 1e-4) v.ampEnv = 0.0;
            }

            const double env = v.ampEnv;

            // TPT state-variable filter (Zavalishin): two integrator states
            const double in = sampleValue * env;
            const double a1 = 1.0 / (1.0 + g * (g + svfDamping));
            const double a2 = g * a1;
            const double a3 = g * a2;
            const double v3 = in - v.filterState2;
            const double v1 = a1 * v.filterState + a2 * v3;
            const double v2 = v.filterState2 + a2 * v.filterState + a3 * v3;
            v.filterState  = 2.0 * v1 - v.filterState;
            v.filterState2 = 2.0 * v2 - v.filterState2;
            const double lp = v2;

            // Drive
            double out = std::tanh (lp * drive) * 0.85;
            out *= v.velocity * ampGain;

            outL += out * panLeft;
            outR += out * panRight;

            if (v.active || v.ampEnv > 1e-4)
                ++activeCount;
        }

        outL *= masterGain;
        outR *= masterGain;

        left[sample]  = static_cast<float> (outL);
        right[sample] = static_cast<float> (outR);
    }

    // FX send buses — parallel taps of the dry synth output
    if (reverbReady || chorusReady)
    {
        dryScratch.setSize (2, numSamples, false, false, true);
        fxScratch.setSize (2, numSamples, false, false, true);
        for (int c = 0; c < 2; ++c)
            dryScratch.copyFrom (c, 0, buffer, c, 0, numSamples);

        if (chorusReady)
        {
            const float send = choSendParam.load();
            if (send > 1e-4f)
            {
                for (int c = 0; c < 2; ++c)
                    fxScratch.copyFrom (c, 0, dryScratch, c, 0, numSamples);
                fxScratch.applyGain (send);
                juce::dsp::AudioBlock<float> wetBlock (fxScratch);
                chorus.process (juce::dsp::ProcessContextReplacing<float> (wetBlock));
                for (int c = 0; c < 2; ++c)
                    buffer.addFrom (c, 0, fxScratch, c, 0, numSamples);
            }
        }

        if (reverbReady)
        {
            const float send = revSendParam.load();
            if (send > 1e-4f)
            {
                for (int c = 0; c < 2; ++c)
                    fxScratch.copyFrom (c, 0, dryScratch, c, 0, numSamples);
                fxScratch.applyGain (send);
                juce::dsp::AudioBlock<float> wetBlock (fxScratch);
                reverb.process (juce::dsp::ProcessContextReplacing<float> (wetBlock));
                for (int c = 0; c < 2; ++c)
                    buffer.addFrom (c, 0, fxScratch, c, 0, numSamples);
            }
        }
    }

    activeVoices.store (activeCount);

    // Output meter (smoothed)
    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        peak = juce::jmax (peak, std::abs (left[i]), std::abs (right[i]));
    }
    const float smoothed = outputPeak.load() * 0.85f + peak * 0.15f;
    outputPeak.store (smoothed);
}

// ── Parameters ──────────────────────────────────────────────────────────
void EONisyaAudioProcessor::applyChorusSettings()
{
    const float depth = apvts.getRawParameterValue (ParamIDs::chorus)->load();
    chorus.setRate (0.4f + depth * 6.0f);
    chorus.setDepth (0.10f + depth * 0.45f);
    chorus.setCentreDelay (7.0f);
    chorus.setFeedback (0.0f);
    chorus.setMix (1.0f);
}

void EONisyaAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (parameterID == ParamIDs::chorus)
    {
        applyChorusSettings();
    }
    else if (parameterID == ParamIDs::reverb)
    {
        reverbMix = newValue;
        reverbParams.wetLevel = static_cast<float> (reverbMix);
        reverbParams.dryLevel = 1.0f - static_cast<float> (reverbMix) * 0.5f;
        reverb.setParameters (reverbParams);
    }
    else if (parameterID == ParamIDs::tempo)
    {
        // Stored for transport display; the sequencer is UI-side.
    }
}

// ── State ───────────────────────────────────────────────────────────────
void EONisyaAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void EONisyaAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// ── Entry point ─────────────────────────────────────────────────────────
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EONisyaAudioProcessor();
}
