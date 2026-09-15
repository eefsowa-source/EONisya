#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"
#include "Components.h"
#include "LCDPanel.h"
#include "CurvePlot.h"

namespace UI
{
    // ── Voice edit page (1b) ─────────────────────────────────────────────
    class VoiceEditPage : public juce::Component
    {
    public:
        VoiceEditPage (juce::AudioProcessorValueTreeState& apvts)
        {
            auto* cutoff  = apvts.getParameter ("cutoff");
            auto* reso    = apvts.getParameter ("reso");
            auto* drive   = apvts.getParameter ("drive");
            auto* keyF    = apvts.getParameter ("keyFollow");
            auto* fegAtk  = apvts.getParameter ("fegAtk");
            auto* fegDcy  = apvts.getParameter ("fegDcy");
            auto* fegSus  = apvts.getParameter ("fegSus");
            auto* fegRel  = apvts.getParameter ("fegRel");

            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (cutoff))
                addKnob (*pf, "CUTOFF", 30.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (reso))
                addKnob (*pf, "RESO", -35.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (drive))
                addKnob (*pf, "DRIVE", -110.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (keyF))
                addKnob (*pf, "KEY FLW", 70.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (fegAtk))
                addKnob (*pf, "FEG ATK", -140.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (fegDcy))
                addKnob (*pf, "FEG DCY", 0.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (fegSus))
                addKnob (*pf, "FEG SUS", 55.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (fegRel))
                addKnob (*pf, "FEG REL", 120.0f);

            // Filter type buttons
            static const char* types[] = { "LPF24D", "LPF18", "BPF12", "HPF12", "BEF12", "DUAL LPF" };
            for (int i = 0; i < 6; ++i)
            {
                filterTypes[i] = std::make_unique<UI::TextButton> (juce::String (types[i]), [] {}, i == 0);
                addAndMakeVisible (*filterTypes[i]);
            }

            // Element on/off
            for (int i = 0; i < 4; ++i)
            {
                elementButtons[i] = std::make_unique<UI::TextButton> (juce::String (i + 1), [] {}, i == 1);
                addAndMakeVisible (*elementButtons[i]);
            }

            // SF buttons
            static const char* sfNames[] = { "OSC", "PITCH", "FILTER", "AMP", "LFO", "EQ" };
            for (int i = 0; i < 6; ++i)
            {
                sfButtons[i] = std::make_unique<UI::TextButton> (juce::String (sfNames[i]),
                                                                 [this, i] { setSfActive (i); },
                                                                 i == 0);
                addAndMakeVisible (*sfButtons[i]);
            }

            // LCD
            lcd = std::make_unique<LCDPanel>();
            lcd->setTitle ("VOICE · PRE 4 · D12", "Analog Saw Stack",
                           "MODE", "VA · 4 ELEM",
                           "XA", "CYCLE");
            addAndMakeVisible (*lcd);

            // Filter + amp curves
            filterCurve = std::make_unique<CurvePlot>();
            ampCurve = std::make_unique<CurvePlot>();
            addAndMakeVisible (*filterCurve);
            addAndMakeVisible (*ampCurve);

            // F-buttons
            static const char* fNames[] = { "COMMON", "ELEMENT", "EG", "LFO", "MOD", "EFFECT", "JOB", "UTILITY" };
            for (int i = 0; i < 8; ++i)
            {
                fButtons[i] = std::make_unique<UI::TextButton> (juce::String (fNames[i]), [] {}, i == 1);
                addAndMakeVisible (*fButtons[i]);
            }

            // Keybed
            keybed = std::make_unique<UI::Keybed>();
            keybed->setOctaveLabel ("C2 — C6");
            addAndMakeVisible (*keybed);
        }

        void paint (juce::Graphics& g) override
        {
            g.fillAll (Theme::panelBg);
            g.setColour (Theme::line);
            g.drawLine (0.0f, (float) getHeight() - 1.0f, (float) getWidth(), (float) getHeight() - 1.0f, 1.0f);
        }

        void resized() override
        {
            const auto b = getLocalBounds();
            const float padX = 16.0f, padY = 14.0f;
            const float colW = 272.0f;

            auto left = b.removeFromLeft (colW);
            left.reduce (padX, padY);

            // Knobs 2x4
            int k = 0;
            for (auto& knob : knobs)
            {
                const int col = k % 4;
                const int row = k / 4;
                knob->setBounds (left.getX() + col * 66, left.getY() + row * 74, 40, 40);
                ++k;
            }

            // Filter type grid (2x3)
            const float typeY = left.getY() + 178.0f;
            for (int i = 0; i < 6; ++i)
            {
                const int col = i % 2;
                const int row = i / 2;
                filterTypes[i]->setBounds (left.getX() + col * 128, typeY + row * 30, 124, 26);
            }

            // Element buttons
            const float elY = typeY + 100.0f;
            for (int i = 0; i < 4; ++i)
                elementButtons[i]->setBounds (left.getX() + i * 64, elY, 60, 28);

            auto right = b.removeFromRight (252.0f);
            right.reduce (padX, padY);

            auto center = b;

            // SF buttons
            const float sfW = (center.getWidth() - 30.0f) / 6.0f;
            for (int i = 0; i < 6; ++i)
                sfButtons[i]->setBounds (center.getX() + i * (sfW + 6.0f), center.getY(), sfW, 32);

            // LCD
            lcd->setBounds (center.getX(), center.getY() + 38.0f,
                            center.getWidth(), center.getHeight() - 38.0f - 40.0f);

            // Curves inside the LCD's two-pane area: we place them over the
            // LCD (right pane) — the LCD draws background, curves draw on top.
            auto lcdArea = lcd->getBounds();
            filterCurve->setBounds (lcdArea.getRight() - 210.0f, lcdArea.getY() + 110.0f, 190.0f, 120.0f);
            ampCurve->setBounds    (lcdArea.getRight() - 210.0f, lcdArea.getY() + 250.0f, 190.0f, 120.0f);

            // F-buttons
            const float fW = (center.getWidth() - 35.0f) / 8.0f;
            for (int i = 0; i < 8; ++i)
                fButtons[i]->setBounds (center.getX() + i * (fW + 5.0f),
                                        center.getBottom() - 36.0f, fW, 30);

            // Keybed
            const float keyH = 182.0f;
            keybed->setBounds (b.getX(), b.getBottom() - keyH, b.getWidth(), keyH);
        }

    private:
        void addKnob (juce::AudioParameterFloat& p, const juce::String& label, float startAngle)
        {
            auto knob = std::make_unique<UI::Knob> (p, label, startAngle);
            addAndMakeVisible (*knob);
            knobs.push_back (std::move (knob));
        }

        void setSfActive (int index)
        {
            for (int i = 0; i < 6; ++i)
                sfButtons[i]->setActive (i == index);
        }

        std::vector<std::unique_ptr<UI::Knob>> knobs;
        std::unique_ptr<UI::TextButton> filterTypes[6];
        std::unique_ptr<UI::TextButton> elementButtons[4];
        std::unique_ptr<UI::TextButton> sfButtons[6];
        std::unique_ptr<LCDPanel> lcd;
        std::unique_ptr<CurvePlot> filterCurve, ampCurve;
        std::unique_ptr<UI::TextButton> fButtons[8];
        std::unique_ptr<UI::Keybed> keybed;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceEditPage)
    };
}
