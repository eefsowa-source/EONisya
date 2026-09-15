#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

namespace UI
{
    // ── The dark LCD screen: title, header rows, and a body ──────────────
    // Draws the border, corner marks, and the top title block (like the
    // design's dark #1d2d3d screen with #416180 border).
    class LCDPanel : public juce::Component
    {
    public:
        LCDPanel() { setOpaque (false); }

        void setTitle (juce::String top, juce::String big, juce::String right1, juce::String right1v,
                       juce::String right2, juce::String right2v)
        {
            titleTop = std::move (top);
            titleBig = std::move (big);
            titleRight1 = std::move (right1);
            titleRight1v = std::move (right1v);
            titleRight2 = std::move (right2);
            titleRight2v = std::move (right2v);
            repaint();
        }

        void paint (juce::Graphics& g) override
        {
            const auto b = getLocalBounds().toFloat();

            g.setColour (Theme::lcdBg);
            g.fillRect (b);
            g.setColour (Theme::lcdBorder);
            g.drawRect (b, 1.0f);

            // Corner marks (like the design's 11px crosshair marks)
            const float m = 5.0f, s = 6.0f;
            g.setColour (Theme::lcdTitle);
            g.drawLine (b.getX() + m - s, b.getY() + m, b.getX() + m + s, b.getY() + m, 1.0f);
            g.drawLine (b.getX() + m, b.getY() + m - s, b.getX() + m, b.getY() + m + s, 1.0f);
            g.drawLine (b.getRight() - m - s, b.getY() + m, b.getRight() - m + s, b.getY() + m, 1.0f);
            g.drawLine (b.getRight() - m, b.getY() + m - s, b.getRight() - m, b.getY() + m + s, 1.0f);

            // Title block
            const float pad = 14.0f;
            auto titleArea = b.reduced (pad, 0.0f).withTop (b.getY() + 14.0f)
                                                  .withHeight (44.0f);

            g.setColour (Theme::lcdTitle);
            g.setFont (Theme::condensedFont (10.0f));
            g.drawText (titleTop, titleArea.withY (titleArea.getY() - 2.0f).withHeight (14.0f),
                        juce::Justification::centredLeft);

            g.setColour (Theme::lcdWhite);
            g.setFont (Theme::condensedFont (30.0f));
            g.drawText (titleBig, titleArea.withY (titleArea.getY() + 16.0f).withHeight (30.0f),
                        juce::Justification::centredLeft);

            // Right readouts
            const float rightW = 170.0f;
            auto rightArea = titleArea.withX (titleArea.getRight() - rightW);
            g.setColour (Theme::lcdTitle);
            g.setFont (Theme::condensedFont (9.0f));
            g.drawText (titleRight1, rightArea.withHeight (12.0f), juce::Justification::centredRight);
            g.setColour (Theme::lcdBright);
            g.setFont (Theme::condensedFont (14.0f));
            g.drawText (titleRight1v, rightArea.withY (rightArea.getY() + 14.0f).withHeight (16.0f),
                        juce::Justification::centredRight);

            g.setColour (Theme::lcdTitle);
            g.setFont (Theme::condensedFont (9.0f));
            g.drawText (titleRight2, rightArea.withY (rightArea.getY() + 30.0f).withHeight (12.0f),
                        juce::Justification::centredRight);
            g.setColour (Theme::lcdBright);
            g.setFont (Theme::condensedFont (14.0f));
            g.drawText (titleRight2v, rightArea.withY (rightArea.getY() + 44.0f).withHeight (16.0f),
                        juce::Justification::centredRight);

            // Bottom border under title
            g.setColour (Theme::lcdBorder);
            g.drawLine (b.getX() + pad, b.getY() + 62.0f, b.getRight() - pad, b.getY() + 62.0f, 1.0f);
        }

        void resized() override
        {
            // Child components (tables, curves) get laid out by subclasses.
        }

    private:
        juce::String titleTop, titleBig, titleRight1, titleRight1v, titleRight2, titleRight2v;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LCDPanel)
    };
}
