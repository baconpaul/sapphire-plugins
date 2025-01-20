/*
 * Sapphire Plugins
 *
 * Bringing the magical world of CosineKitty's sapphire plugins for rack to your DAW
 *
 * Copyright 2024-2025, Don Cross, Paul Walker, Morgon Kanter and other authors, as
 * described in the github transaction log.
 *
 * This project is distributed under the Gnu General Public License, version 3.0 or later.
 * You can find the LICENSE file at the address below.
 *
 * The source code and license are at https://github.com/baconpaul/sapphire-plugins
 */

#ifndef SAPPHIRE_PLUGINS_SHARED_TOOLTIP_H
#define SAPPHIRE_PLUGINS_SHARED_TOOLTIP_H

#include "configuration.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace sapphire_plugins::shared
{
struct Tooltip : juce::Component
{
    std::string title;
    std::string value;

    void paint(juce::Graphics &g) override
    {
        g.fillAll(juce::Colours::black.withAlpha(0.6f));
        juce::Graphics::ScopedSaveState ss(g);
        auto at = juce::AffineTransform().scaled(0.2);
        auto sb = getLocalBounds().transformedBy(at.inverted());
        g.addTransform(at);
        g.setColour(juce::Colours::white);
        g.drawRect(sb, 1.f);
        g.setFont(juce::FontOptions(20.0f));
        g.drawText(title, sb.withHeight(sb.getHeight() / 2).reduced(4),
                   juce::Justification::centredLeft, true);
        g.drawText(value, sb.withTrimmedTop(sb.getHeight() / 2).reduced(4),
                   juce::Justification::centredLeft, true);
    }
};
} // namespace sapphire_plugins::shared

#endif // TOOLTIP_H
