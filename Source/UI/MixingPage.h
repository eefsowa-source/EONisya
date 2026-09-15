#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"
#include "Components.h"
#include "LCDPanel.h"

namespace UI
{
    // ── Mixing page (1c) ─────────────────────────────────────────────────
    class MixingPage : public juce::Component
    {
    public:
        MixingPage (juce::AudioProcessorValueTreeState& apvts)
        {
            auto* vol  = apvts.getParameter ("volume");
            auto* pan  = apvts.getParameter ("pan");
            auto* rev  = apvts.getParameter ("revSend");
            auto* cho  = apvts.getParameter ("choSend");
            auto* cutoff = apvts.getParameter ("cutoff");
            auto* reso = apvts.getParameter ("reso");
            auto* attack = apvts.getParameter ("attack");
            auto* release = apvts.getParameter ("release");

            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (vol))
                addKnob (*pf, "VOLUME", -125.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (pan))
                addKnob (*pf, "PAN", 0.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (rev))
                addKnob (*pf, "REV SND", -70.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (cho))
                addKnob (*pf, "CHO SND", -95.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (cutoff))
                addKnob (*pf, "CUTOFF", 25.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (reso))
                addKnob (*pf, "RESO", -50.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (attack))
                addKnob (*pf, "ATTACK", 90.0f);
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*> (release))
                addKnob (*pf, "RELEASE", 140.0f);

            // SF buttons
            static const char* sfNames[] = { "VOL / PAN", "VOICE", "SEND", "TUNE", "ARP", "OUT SEL" };
            for (int i = 0; i < 6; ++i)
            {
                sfButtons[i] = std::make_unique<UI::TextButton> (juce::String (sfNames[i]),
                                                                 [this, i] { setSfActive (i); },
                                                                 i == 0);
                addAndMakeVisible (*sfButtons[i]);
            }

            // LCD
            lcd = std::make_unique<LCDPanel>();
            lcd->setTitle ("MIXING · SONG 04", "Night Drive Sketch",
                           "PARTS USED", "11 / 16",
                           "OUTPUT", "L&R + 8 BUS");
            addAndMakeVisible (*lcd);

            // F-buttons
            static const char* fNames[] = { "MIXER", "VOICE", "SEND", "EFFECT", "EQ", "ARP", "JOB", "UTILITY" };
            for (int i = 0; i < 8; ++i)
            {
                fButtons[i] = std::make_unique<UI::TextButton> (juce::String (fNames[i]), [] {}, i == 0);
                addAndMakeVisible (*fButtons[i]);
            }

            // Track select 4x4
            for (int i = 0; i < 16; ++i)
            {
                trackButtons[i] = std::make_unique<UI::TextButton> (
                    juce::String (i + 1),
                    [this, i] { setTrackActive (i); },
                    i == 2);
                addAndMakeVisible (*trackButtons[i]);
            }

            // Mute / solo
            muteBtn = std::make_unique<UI::TextButton> ("MUTE");
            soloBtn = std::make_unique<UI::TextButton> ("SOLO", [] {}, true);
            addAndMakeVisible (*muteBtn);
            addAndMakeVisible (*soloBtn);

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
            const float padX = 16.0f, padY = 14.0f;
            const float colW = 272.0f;

            auto left = b.removeFromLeft (colW);
            left.reduce (padX, padY);

            int k = 0;
            for (auto& knob : knobs)
            {
                const int col = k % 4;
                const int row = k / 4;
                knob->setBounds (left.getX() + col * 66, left.getY() + row * 74, 40, 40);
                ++k;
            }

            // Insert assign list (below knobs)
            // (painted by paintInsertList)

            auto right = b.removeFromRight (252.0f);
            right.reduce (padX, padY);

            // Track select 4x4
            const float gridW = right.getWidth();
            const float cell = (gridW - 3 * 6.0f) / 4.0f;
            const float gridY = right.getY() + 30.0f;
            for (int i = 0; i < 16; ++i)
            {
                const int col = i % 4;
                const int row = i / 4;
                trackButtons[i]->setBounds (right.getX() + col * (cell + 6.0f),
                                            gridY + row * (cell + 6.0f),
                                            cell, cell);
            }

            muteBtn->setBounds (right.getX(), gridY + 4 * (cell + 6.0f), (gridW - 6.0f) / 2.0f, 28);
            soloBtn->setBounds (right.getX() + (gridW - 6.0f) / 2.0f + 6.0f, gridY + 4 * (cell + 6.0f),
                                (gridW - 6.0f) / 2.0f, 28);

            auto center = b;

            // SF buttons
            const float sfW = (center.getWidth() - 30.0f) / 6.0f;
            for (int i = 0; i < 6; ++i)
                sfButtons[i]->setBounds (center.getX() + i * (sfW + 6.0f), center.getY(), sfW, 32);

            lcd->setBounds (center.getX(), center.getY() + 38.0f,
                            center.getWidth(), center.getHeight() - 38.0f - 40.0f);

            const float fW = (center.getWidth() - 35.0f) / 8.0f;
            for (int i = 0; i < 8; ++i)
                fButtons[i]->setBounds (center.getX() + i * (fW + 5.0f),
                                        center.getBottom() - 36.0f, fW, 30);

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

        void setTrackActive (int index)
        {
            for (int i = 0; i < 16; ++i)
                trackButtons[i]->setActive (i == index);
        }

        std::vector<std::unique_ptr<UI::Knob>> knobs;
        std::unique_ptr<UI::TextButton> sfButtons[6];
        std::unique_ptr<LCDPanel> lcd;
        std::unique_ptr<UI::TextButton> fButtons[8];
        std::unique_ptr<UI::TextButton> trackButtons[16];
        std::unique_ptr<UI::TextButton> muteBtn, soloBtn;
        std::unique_ptr<UI::Keybed> keybed;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixingPage)
    };
}
