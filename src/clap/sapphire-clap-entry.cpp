#include <clap/clap.h>
#include "sapphire-clap-entry-impl.h"

extern "C"
{

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif

    // clang-format off
    const CLAP_EXPORT struct clap_plugin_entry clap_entry = {
        CLAP_VERSION,
        sapphire_plugins::clap_init,
        sapphire_plugins::clap_deinit,
        sapphire_plugins::get_factory
    };
    // clang-format on
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}