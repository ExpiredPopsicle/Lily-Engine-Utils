#pragma once

namespace ExPop {

    const unsigned int DERP_MAX_TOKEN_LENGTH = 1024;
    const unsigned int DERP_MAX_STRING_LENGTH = 1024;
    const unsigned int DERP_MAX_TOKENS = 65536;

    // FIXME: We can probably take out the per-function call counter
    // now that we have a global call count thing.
    const unsigned int DERP_MAX_STACK_FRAMES = 16384;

    const unsigned int DERP_MAX_OBJECT_COUNT = 0xefffffff;

}

