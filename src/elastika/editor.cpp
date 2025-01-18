/*
 * Sapphire Plugins
 *
 * Bringing the magical world of CosineKitty's sapphire plugins for rack to your DAW
 *
 * Copyright 2024-2025, Don Cross, Paul Walker and Various authors, as described in the github
 * transaction log.
 *
 * The source code and license are at https://github.com/baconpaul/sapphire-plugins
 */

#include "editor.h"
#include "configuration.h"
#include "shared/graphics_resources.h"

#include <clap/plugin.h>

namespace sapphire_plugins::elastika
{

ElastikaEditor::ElastikaEditor(audioToUIQueue_t &atou, uiToAudioQueue_T &utoa,
                               std::function<void()> flushOperator)
{

    auto bg = shared::getSvgForPath("libs/sapphire/res/elastika.svg");
    if (bg.has_value())
    {
        auto bgx = juce::XmlDocument::parse(*bg);
        if (bgx)
            background = juce::Drawable::createFromSVG(*bgx);
    }
    setSize(300, 600);
    resized();
}

void ElastikaEditor::paint(juce::Graphics &g)
{
    if (background)
        background->drawAt(g, 0, 0, 1.0);
}

void ElastikaEditor::resized()
{
    if (background)
    {
        background->setTransformToFit(getLocalBounds().toFloat(), juce::RectanglePlacement());
    }
}

} // namespace sapphire_plugins::elastika