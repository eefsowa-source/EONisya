#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

namespace UI
{
    // ── Small labelled knob (40px, marker at rotation) ────────────────────
    class Knob : public juce::Component
    {
    public:
        Knob (juce::AudioParameterFloat& param, juce::String label, float startAngle = -135.0f)
            : parameter (param), labelText (std::move (label)), startAngleDeg (startAngle)
        {
            setInterceptsMouseClicks (true, false);
            setMouseCursor (juce::MouseCursor::PointingHandCursor);
            param.addListener (this);
        }

        ~Knob() override { parameter.removeListener (this); }

        void setStartAngleDegrees (float deg) { startAngleDeg = deg; }

        void paint (juce::Graphics& g) override
        {
            const auto bounds = getLocalBounds().toFloat();

            // Knob body
            g.setColour (Theme::panelBg);
            g.fillEllipse (bounds);
            g.setColour (Theme::knobEdge);
            g.drawEllipse (bounds, 1.0f);

            // Inner ring
            const auto inner = bounds.reduced (bounds.getWidth() * 0.32f);
            g.setColour (Theme::lineStrong);
            g.drawEllipse (inner, 1.0f);

            // Marker
            const float norm = parameter.getNormalisableRange().convertTo0to1 (parameter.get());
            const float angle = juce::degreesToRadians (startAngleDeg + (norm - 0.5f) * 270.0f);
            const float cx = bounds.getCentreX();
            const float cy = bounds.getCentreY();
            const float r = bounds.getWidth() * 0.5f - 3.0f;

            g.setColour (Theme::accent);
            g.drawLine (juce::Line<float> (cx, cy,
                                           cx + std::cos (angle) * r,
                                           cy + std::sin (angle) * r), 2.0f);
            g.fillEllipse (cx - 1.5f, cy - 1.5f, 3.0f, 3.0f);
        }

        void resized() override {}

        void mouseDown (const juce::MouseEvent&) override { dragging = true; }
        void mouseUp (const juce::MouseEvent&) override { dragging = false; }

        void mouseDrag (const juce::MouseEvent& e) override
        {
            const auto start = juce::Point<float> (static_cast<float> (e.getMouseDownX()),
                                                   static_cast<float> (e.getMouseDownY()));
            const auto end = juce::Point<float> (static_cast<float> (e.getPosition().x),
                                                 static_cast<float> (e.getPosition().y));
            const auto delta = end.y - start.y;
            const auto range = parameter.getNormalisableRange();
            const float speed = range.convertTo0to1 (range.end) / 200.0f;
            parameter.setValueNotifyingHost (juce::jlimit (0.0f, 1.0f,
                                                           parameter.getValue() - delta * speed));
        }

        void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w) override
        {
            const auto range = parameter.getNormalisableRange();
            const float speed = range.convertTo0to1 (range.end) / 200.0f;
            parameter.setValueNotifyingHost (juce::jlimit (0.0f, 1.0f,
                                                           parameter.getValue() + w.deltaY * speed));
        }

        // For layout: label sits under the knob, painted by the container.
        const juce::String& getLabel() const noexcept { return labelText; }

        void parameterChanged (juce::AudioParameterFloat*, float) override { repaint(); }

    private:
        juce::AudioParameterFloat& parameter;
        juce::String labelText;
        float startAngleDeg = -135.0f;
        bool dragging = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Knob)
    };

    // ── Vertical fader (7px wide, like control sliders) ──────────────────
    class Fader : public juce::Component
    {
    public:
        explicit Fader (juce::AudioParameterFloat& param) : parameter (param)
        {
            setInterceptsMouseClicks (true, false);
            param.addListener (this);
        }

        ~Fader() override { parameter.removeListener (this); }

        void paint (juce::Graphics& g) override
        {
            const auto b = getLocalBounds().toFloat();
            const float norm = parameter.getNormalisableRange().convertTo0to1 (parameter.get());
            const float fillH = juce::jmax (0.0f, b.getHeight() * norm);

            g.setColour (Theme::lineStrong);
            g.drawRect (b, 1.0f);

            if (fillH > 0.0f)
            {
                g.setColour (Theme::accentMid);
                g.fillRect (b.withTop (b.getBottom() - fillH));
            }

            // Cap
            const float capY = b.getBottom() - fillH;
            g.setColour (Theme::accentDark);
            g.fillRect (b.expanded (4.0f, 0.0f).withTop (capY - 1.5f).withHeight (3.0f));
        }

        void mouseDrag (const juce::MouseEvent& e) override
        {
            const float norm = juce::jlimit (0.0f, 1.0f,
                                             1.0f - static_cast<float> (e.getPosition().y) / static_cast<float> (getHeight()));
            const auto range = parameter.getNormalisableRange();
            parameter.setValueNotifyingHost (range.convertFrom0to1 (norm));
        }

        void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w) override
        {
            const auto range = parameter.getNormalisableRange();
            parameter.setValueNotifyingHost (juce::jlimit (0.0f, 1.0f,
                                                           parameter.getValue() + w.deltaY * 0.01f));
        }

        void parameterChanged (juce::AudioParameterFloat*, float) override { repaint(); }

    private:
        juce::AudioParameterFloat& parameter;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Fader)
    };

    // ── Simple text button (bordered, condensed font) ────────────────────
    class TextButton : public juce::Component
    {
    public:
        TextButton (juce::String text, std::function<void()> onClick = {}, bool active = false)
            : label (std::move (text)), callback (std::move (onClick)), isActive (active)
        {
            setInterceptsMouseClicks (true, false);
            setMouseCursor (juce::MouseCursor::PointingHandCursor);
        }

        void setActive (bool active) { isActive = active; repaint(); }
        bool getActive() const noexcept { return isActive; }
        void setOnClick (std::function<void()> cb) { callback = std::move (cb); }

        void paint (juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat();

            if (isActive)
            {
                g.setColour (Theme::accent);
                g.fillRect (b);
                g.setColour (juce::Colours::white.withAlpha (0.9f));
            }
            else
            {
                g.setColour (Theme::lineStrong);
                g.drawRect (b, 1.0f);
                g.setColour (Theme::textDim);
            }

            g.setFont (Theme::condensedFont (juce::jmax (9.0f, getHeight() * 0.42f)));
            g.drawText (label, b, juce::Justification::centred);
        }

        void mouseDown (const juce::MouseEvent&) override
        {
            if (callback) callback();
        }

    private:
        juce::String label;
        std::function<void()> callback;
        bool isActive = false;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextButton)
    };

    // ── Keybed (28 white keys, black keys, pitch/mod wheels) ──────────────
    class Keybed : public juce::Component
    {
    public:
        Keybed()
        {
            setInterceptsMouseClicks (true, false);
        }

        void setOctaveLabel (juce::String label) { octaveLabel = std::move (label); repaint(); }

        void paint (juce::Graphics& g) override
        {
            const auto b = getLocalBounds().toFloat();
            const float keyH = b.getHeight() - 26.0f;
            const float keyW = 25.0f;
            const int whiteCount = 28;

            // White keys
            for (int i = 0; i < whiteCount; ++i)
            {
                juce::Rectangle<float> key (b.getX() + i * keyW, b.getY() + 8.0f, keyW - 1.0f, keyH);
                g.setColour (Theme::whiteKey);
                g.fillRect (key);
                g.setColour (Theme::lineStrong);
                g.drawRect (key, 1.0f);
            }

            // Black keys (pattern from the design: [T,T,F,T,T,T,F], last white is bare)
            static const bool blackPattern[] = { true, true, false, true, true, true, false };
            for (int i = 0; i < whiteCount; ++i)
            {
                if (i == whiteCount - 1) continue;
                if (! blackPattern[i % 7]) continue;

                juce::Rectangle<float> black (b.getX() + i * keyW + keyW - 9.0f, b.getY() + 8.0f,
                                              17.0f, keyH * 0.62f);
                g.setColour (Theme::blackKey);
                g.fillRect (black);
                g.setColour (juce::Colours::black.withAlpha (0.35f));
                g.drawRect (black, 1.0f);
            }

            // Octave label
            g.setColour (Theme::textFaint);
            g.setFont (Theme::condensedFont (10.0f));
            g.drawText ("OCTAVE", juce::Rectangle<float> (b.getX(), b.getY(), 70.0f, 14.0f),
                        juce::Justification::centredLeft);
            g.setColour (Theme::accentDark);
            g.setFont (Theme::condensedFont (12.0f));
            g.drawText (octaveLabel, juce::Rectangle<float> (b.getX() + 74.0f, b.getY(), 90.0f, 14.0f),
                        juce::Justification::centredLeft);
        }

    private:
        juce::String octaveLabel = "C1 — C5";
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Keybed)
    };
}
