#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"
#include "Components.h"
#include "LCDPanel.h"

namespace UI
{
    // ── Perform page (1a) ────────────────────────────────────────────────
    class PerformPage : public juce::Component
    {
    public:
        PerformPage (juce::AudioProcessorValueTreeState& apvts)
        {
            // Assignable knobs
            auto* cutoff  = apvts.getParameter ("cutoff");
            auto* reso    = apvts.getParameter ("reso");
            auto* attack  = apvts.getParameter ("attack");
            auto* release = apvts.getParameter ("release");
            auto* a1      = apvts.getParameter ("assign1");
            auto* a2      = apvts.getParameter ("assign2");
            auto* rvb     = apvts.getParameter ("reverb");
            auto* cho     = apvts.getParameter ("chorus");

            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (cutoff))
                addKnob (*pf, "CUTOFF", -135.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (reso))
                addKnob (*pf, "RESO", -60.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (attack))
                addKnob (*pf, "ATTACK", -20.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (release))
                addKnob (*pf, "RELEASE", 45.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (a1))
                addKnob (*pf, "ASSIGN 1", -100.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (a2))
                addKnob (*pf, "ASSIGN 2", 15.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (rvb))
                addKnob (*pf, "REVERB", -45.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (cho))
                addKnob (*pf, "CHORUS", -80.0f);

            // XA control
            legatoBtn = std::make_unique<UI::TextButton> ("LEGATO", [] {}, true);
            keyOffBtn = std::make_unique<UI::TextButton> ("KEY OFF");
            cycleBtn  = std::make_unique<UI::TextButton> ("CYCLE");
            addAndMakeVisible (*legatoBtn);
            addAndMakeVisible (*keyOffBtn);
            addAndMakeVisible (*cycleBtn);

            // SF buttons
            static const char* sfNames[] = { "LAYER", "SPLIT", "ARP ON", "ZONE", "A. FUNC", "STORE" };
            for (int i = 0; i < 6; ++i)
            {
                sfButtons[i] = std::make_unique<UI::TextButton> (juce::String (sfNames[i]),
                                                                 [this, i] { setSfActive (i); },
                                                                 i == 2);
                addAndMakeVisible (*sfButtons[i]);
            }

            // LCD
            lcd = std::make_unique<LCDPanel>();
            lcd->setTitle ("PERFORM · USER 1 · A01", "Steel Cathedral",
                           "ENGINE", "AWM2 ×3 + VA ×1",
                           "CATEGORY", "Pad / Choir");
            addAndMakeVisible (*lcd);

            // F-buttons
            static const char* fNames[] = { "PLAY", "PART", "ARP", "EFFECT", "EQ", "ZONE", "JOB", "UTILITY" };
            for (int i = 0; i < 8; ++i)
            {
                fButtons[i] = std::make_unique<UI::TextButton> (juce::String (fNames[i]), [] {}, i == 0);
                addAndMakeVisible (*fButtons[i]);
            }

            // Keybed
            keybed = std::make_unique<UI::Keybed>();
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
            const float padX = 16.0f;
            const float padY = 14.0f;
            const float colW = 272.0f;

            // Left column
            auto left = b.removeFromLeft (colW);
            left.reduce (padX, padY);
            const float knobAreaH = 62.0f;
            int k = 0;
            for (auto& knob : knobs)
            {
                const int col = k % 4;
                const int row = k / 4;
                knob->setBounds (left.getX() + col * 66, left.getY() + row * 74, 40, 40);
                ++k;
            }

            // Label row under knobs
            const float labelsY = left.getY() + 40.0f;

            // XA control row
            const float xaY = left.getY() + 170.0f;
            legatoBtn->setBounds (left.getX(), xaY, 76, 26);
            keyOffBtn->setBounds (left.getX() + 82, xaY, 76, 26);
            cycleBtn->setBounds (left.getX() + 164, xaY, 76, 26);

            // Right column
            auto right = b.removeFromRight (252.0f);
            right.reduce (padX, padY);

            // Center (LCD)
            auto center = b;

            // SF buttons across the top of center
            const float sfW = (center.getWidth() - 30.0f) / 6.0f;
            for (int i = 0; i < 6; ++i)
                sfButtons[i]->setBounds (center.getX() + i * (sfW + 6.0f), center.getY(), sfW, 32);

            // LCD
            lcd->setBounds (center.getX(), center.getY() + 38.0f,
                            center.getWidth(), center.getHeight() - 38.0f - 40.0f);

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
        std::unique_ptr<UI::TextButton> legatoBtn, keyOffBtn, cycleBtn;
        std::unique_ptr<UI::TextButton> sfButtons[6];
        std::unique_ptr<LCDPanel> lcd;
        std::unique_ptr<UI::TextButton> fButtons[8];
        std::unique_ptr<UI::Keybed> keybed;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerformPage)
    };
}
