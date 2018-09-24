// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2018 Kiri Jolly
//     http://expiredpopsicle.com
//     expiredpopsicle@gmail.com
//
// ---------------------------------------------------------------------------
//
//   This software is provided 'as-is', without any express or implied
//   warranty. In no event will the authors be held liable for any
//   damages arising from the use of this software.
//
//   Permission is granted to anyone to use this software for any
//   purpose, including commercial applications, and to alter it and
//   redistribute it freely, subject to the following restrictions:
//
//   1. The origin of this software must not be misrepresented; you must
//      not claim that you wrote the original software. If you use this
//      software in a product, an acknowledgment in the product
//      documentation would be appreciated but is not required.
//
//   2. Altered source versions must be plainly marked as such, and must
//      not be misrepresented as being the original software.
//
//   3. This notice may not be removed or altered from any source
//      distribution.
//
// -------------------------- END HEADER -------------------------------------

// Console SDL support for software (non-SDL_Renderer) rendering.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#if EXPOP_ENABLE_SDL2

#pragma once

#include <SDL.h>

#include "graphicalconsole.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    class GraphicalConsole_SDLSoftwareWrapper
    {
    public:

        GraphicalConsole_SDLSoftwareWrapper();
        GraphicalConsole_SDLSoftwareWrapper(
            GraphicalConsole *inConsole);
        ~GraphicalConsole_SDLSoftwareWrapper();

        void render(SDL_Window *window);

    private:

        GraphicalConsole *console;
        SDL_Surface *textBackSurf;
     };
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    inline GraphicalConsole_SDLSoftwareWrapper::GraphicalConsole_SDLSoftwareWrapper()
    {
        console = getMainConsole();
        textBackSurf = nullptr;
    }

    inline GraphicalConsole_SDLSoftwareWrapper::GraphicalConsole_SDLSoftwareWrapper(
        GraphicalConsole *inConsole)
    {
        console = inConsole;
        textBackSurf = nullptr;
    }

    inline GraphicalConsole_SDLSoftwareWrapper::~GraphicalConsole_SDLSoftwareWrapper()
    {
        if(textBackSurf) {
            SDL_FreeSurface(textBackSurf);
        }
    }

    inline void GraphicalConsole_SDLSoftwareWrapper::render(SDL_Window *window)
    {
        if(console->getActive()) {

            int winWidth = 0;
            int winHeight = 0;
            SDL_GetWindowSize(window, &winWidth, &winHeight);
            winHeight /= 2;

            if(!textBackSurf ||
                textBackSurf->w != winWidth ||
                textBackSurf->h != winHeight)
            {
                if(textBackSurf) {
                    SDL_FreeSurface(textBackSurf);
                    textBackSurf = nullptr;
                }

                console->setBackbufferSize(winWidth, winHeight);
                textBackSurf = makeSDLSurfaceForImage(
                    *console->getBackbuffer(), true);
            }

            // Run an update if needed.
            if(console->needsUpdate()) {
                console->updateBackbuffer(true);
            }

            SDL_BlitSurface(
                textBackSurf,
                nullptr,
                SDL_GetWindowSurface(window),
                nullptr);
        }
    }

    inline GraphicalConsole_SDLSoftwareWrapper &getMainSDLConsoleSoftwareWrapper()
    {
        static GraphicalConsole_SDLSoftwareWrapper mainWrapper;
        return mainWrapper;
    }

    inline int ExPop_SDL_UpdateWindowSurface(SDL_Window *window)
    {
        getMainSDLConsoleSoftwareWrapper().render(window);
        return SDL_UpdateWindowSurface(window);
    }

  #if EXPOP_ENABLE_SDL2_OVERRIDES

  #ifdef SDL_UpdateWindowSurface
  #undef SDL_UpdateWindowSurface
  #endif
  #define SDL_UpdateWindowSurface ExPop::ExPop_SDL_UpdateWindowSurface

  #endif
}

#endif


