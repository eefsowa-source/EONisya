#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

namespace UI
{
    // ── Curve / envelope plot (like the design's filter curve + amp EG) ──
    class CurvePlot : public juce::Component
    {
    public:
        CurvePlot() { setOpaque (false); }

        void setPoints (std::vector<juce::Point<float>> pts) { points = std::move (pts); repaint(); }

        void paint (juce::Graphics& g) override
        {
            const auto b = getLocalBounds().toFloat();

            // Grid lines
            g.setColour (Theme::lcdTitle.withAlpha (0.35f));
            for (int i = 1; i < 4; ++i)
            {
                const float x = b.getX() + b.getWidth() * i / 4.0f;
                g.drawLine (x, b.getY(), x, b.getBottom(), 1.0f);
            }
            g.drawLine (b.getX(), b.getBottom() - 1.0f, b.getRight(), b.getBottom() - 1.0f, 1.0f);

            if (points.size() < 2)
                return;

            // Path
            juce::Path path;
            const float padX = b.getWidth() * 0.02f;
            const float padY = b.getHeight() * 0.06f;
            for (size_t i = 0; i < points.size(); ++i)
            {
                const float x = b.getX() + padX + (b.getWidth() - padX * 2.0f) * points[i].x;
                const float y = b.getY() + padY + (b.getHeight() - padY * 2.0f) * (1.0f - points[i].y);
                if (i == 0) path.startNewSubPath (x, y);
                else path.lineTo (x, y);
            }

            g.setColour (Theme::accentMid);
            g.strokePath (path, juce::PathStrokeType (1.5f));
        }

    private:
        std::vector<juce::Point<float>> points;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CurvePlot)
    };
}
