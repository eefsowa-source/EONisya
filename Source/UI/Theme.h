#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace Theme
{
    // Hardware panel palette (from EON MOTIF6 Panel.dc.html)
    inline const juce::Colour panelBg      { 0xfff2f2f3 };
    inline const juce::Colour panelBorder  { juce::Colour (0xff1d1f20).withAlpha (0.30f) };
    inline const juce::Colour line         { juce::Colour (0xff1d1f20).withAlpha (0.16f) };
    inline const juce::Colour lineStrong   { juce::Colour (0xff1d1f20).withAlpha (0.24f) };
    inline const juce::Colour textBody     { 0xff1d1f20 };
    inline const juce::Colour textDim      { 0xff5d5d60 };
    inline const juce::Colour textFaint    { 0xff7a7a7d };
    inline const juce::Colour textGhost    { 0xff9c9ca0 };

    // EON accent blue
    inline const juce::Colour accent       { 0xff5980a6 };
    inline const juce::Colour accentDark   { 0xff416180 };
    inline const juce::Colour accentLight  { 0xffeef6ff };
    inline const juce::Colour accentMid    { 0xff94bce3 };

    // LCD (dark screen)
    inline const juce::Colour lcdBg        { 0xff1d2d3d };
    inline const juce::Colour lcdBorder    { 0xff416180 };
    inline const juce::Colour lcdTitle     { 0xff9ebbd8 };
    inline const juce::Colour lcdBright    { 0xffd6ebff };
    inline const juce::Colour lcdWhite     { 0xfff2f2f3 };
    inline const juce::Colour lcdLine      { juce::Colour (0xff416180).withAlpha (0.55f) };

    // Knob / wheel
    inline const juce::Colour knobEdge     { 0xff9c9ca0 };

    // Keybed
    inline const juce::Colour whiteKey     { 0xfffbfbfc };
    inline const juce::Colour blackKey     { 0xff2b2b2d };

    // Typography
    inline const juce::FontOptions bodyFont (float h)
    {
        return juce::FontOptions (h).withFallbacks ({ "Helvetica Neue", "Helvetica", "Arial", "System" });
    }
    inline const juce::FontOptions condensedFont (float h)
    {
        return juce::FontOptions (h).withFallbacks ({ "Arial Narrow", "Helvetica Neue", "Helvetica", "Arial", "System" });
    }
}
