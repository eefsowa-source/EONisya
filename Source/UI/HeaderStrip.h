#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Components.h"
#include "Theme.h"

namespace UI
{
    // ── Header strip (logo, mode tabs, tempo/master/poly) ────────────────
    class HeaderStrip : public juce::Component
    {
    public:
        HeaderStrip (juce::String modeLabel,
                     std::function<void (int)> onMode,
                     int currentMode)
            : modeText (std::move (modeLabel))
        {
            for (int i = 0; i < 5; ++i)
            {
                tabs[i] = std::make_unique<UI::TextButton> (
                    juce::StringArray ("PERFORM", "VOICE", "MIXING", "SONG", "PATTERN")[i],
                    [this, i, onMode] { if (onMode) onMode (i); },
                    i == currentMode);
                addAndMakeVisible (*tabs[i]);
            }

            // Tempo + master meter readouts
            tempoValue = "112.0";
            masterValue = "108";
            polyValue = "34";
            repaint();
        }

        void setValues (juce::String tempo, juce::String master, juce::String poly)
        {
            tempoValue = std::move (tempo);
            masterValue = std::move (master);
            polyValue = std::move (poly);
            repaint();
        }

        void setMode (juce::String modeLabel, int mode)
        {
            modeText = std::move (modeLabel);
            for (int i = 0; i < 5; ++i)
                tabs[i]->setActive (i == mode);
            repaint();
        }

        void paint (juce::Graphics& g) override
        {
            const auto b = getLocalBounds().toFloat();
            g.fillAll (Theme::panelBg);
            g.setColour (Theme::line);
            g.drawLine (b.getX(), b.getBottom() - 1.0f, b.getRight(), b.getBottom() - 1.0f, 1.0f);

            // Logo
            juce::Rectangle<float> logo (b.getX() + 18.0f, b.getY() + 18.0f, 22.0f, 22.0f);
            g.setColour (Theme::accent);
            g.drawRect (logo, 1.0f);
            g.setColour (Theme::accentDark);
            g.setFont (Theme::condensedFont (12.0f));
            g.drawText ("E", logo, juce::Justification::centred);

            g.setColour (Theme::textBody);
            g.setFont (Theme::condensedFont (19.0f));
            g.drawText ("EON MOTIF6", juce::Rectangle<float> (b.getX() + 50.0f, b.getY() + 14.0f, 160.0f, 20.0f),
                        juce::Justification::centredLeft);
            g.setColour (Theme::textFaint);
            g.setFont (Theme::condensedFont (9.0f));
            g.drawText (modeText, juce::Rectangle<float> (b.getX() + 50.0f, b.getY() + 34.0f, 220.0f, 12.0f),
                        juce::Justification::centredLeft);

            // Right readouts
            const float rightX = b.getRight() - 18.0f;
            g.setColour (Theme::textFaint);
            g.setFont (Theme::condensedFont (9.0f));
            g.drawText ("TEMPO", juce::Rectangle<float> (rightX - 320.0f, b.getY() + 16.0f, 90.0f, 12.0f),
                        juce::Justification::centredRight);
            g.setColour (Theme::textBody);
            g.setFont (Theme::condensedFont (17.0f));
            g.drawText (tempoValue, juce::Rectangle<float> (rightX - 320.0f, b.getY() + 30.0f, 90.0f, 18.0f),
                        juce::Justification::centredRight);

            g.setColour (Theme::textFaint);
            g.setFont (Theme::condensedFont (9.0f));
            g.drawText ("MASTER", juce::Rectangle<float> (rightX - 150.0f, b.getY() + 16.0f, 60.0f, 12.0f),
                        juce::Justification::centredRight);
            g.setColour (Theme::textBody);
            g.setFont (Theme::condensedFont (17.0f));
            g.drawText (masterValue, juce::Rectangle<float> (rightX - 150.0f, b.getY() + 30.0f, 60.0f, 18.0f),
                        juce::Justification::centredRight);

            // POLY
            g.setColour (Theme::textFaint);
            g.setFont (Theme::condensedFont (9.0f));
            g.drawText ("POLY", juce::Rectangle<float> (rightX - 80.0f, b.getY() + 16.0f, 60.0f, 12.0f),
                        juce::Justification::centredRight);
            g.setColour (Theme::textBody);
            g.setFont (Theme::condensedFont (17.0f));
            g.drawText (polyValue, juce::Rectangle<float> (rightX - 80.0f, b.getY() + 30.0f, 60.0f, 18.0f),
                        juce::Justification::centredRight);
        }

        void resized() override
        {
            const float tabY = (getHeight() - 28.0f) * 0.5f;
            const float tabW = 92.0f;
            for (int i = 0; i < 5; ++i)
                tabs[i]->setBounds (140 + i * tabW, tabY, tabW, 28.0f);
        }

    private:
        juce::String modeText;
        juce::String tempoValue, masterValue, polyValue;
        std::unique_ptr<UI::TextButton> tabs[5];

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderStrip)
    };
}
