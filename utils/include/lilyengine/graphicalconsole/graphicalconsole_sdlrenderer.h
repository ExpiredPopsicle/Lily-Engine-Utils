// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2016 Clifford Jolly
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

// Console SDL_Renderer support.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#if EXPOP_ENABLE_SDL2

#pragma once

#include <SDL.h>

#include "graphicalconsole.h"

#include <chrono>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    class GraphicalConsole_SDLRendererWrapper
    {
    public:

        GraphicalConsole_SDLRendererWrapper(
            GraphicalConsole *inConsole);
        GraphicalConsole_SDLRendererWrapper();
        ~GraphicalConsole_SDLRendererWrapper();

        void render(SDL_Renderer *renderer);
        void initTextureForRenderer(SDL_Renderer *renderer);
        void deleteTextureForRenderer(SDL_Renderer *renderer);

    private:

        void commonInit();

        GraphicalConsole *console;
        SDL_Surface *textBackSurf;
        std::map<SDL_Renderer *, SDL_Texture *> texturesByRenderer;
        std::chrono::time_point<std::chrono::high_resolution_clock> lastUpdateTime;
        uint32_t lastSeenBackBufferUpdate;
    };
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    inline GraphicalConsole_SDLRendererWrapper::GraphicalConsole_SDLRendererWrapper()
    {
        console = getMainConsole();
        commonInit();
    }

    inline GraphicalConsole_SDLRendererWrapper::GraphicalConsole_SDLRendererWrapper(
        GraphicalConsole *inConsole)
    {
        console = inConsole;
        commonInit();
    }

    inline GraphicalConsole_SDLRendererWrapper::~GraphicalConsole_SDLRendererWrapper()
    {
        if(texturesByRenderer.size()) {
            ExPop::out()
                << ExPop::graphicalConsoleGetRedErrorText()
                << ": Graphical console texture(s) still remaining at shutdown time. Improper shutdown?"
                << std::endl;
        }

        SDL_FreeSurface(textBackSurf);
    }

    inline SDL_Surface *makeSDLSurfaceForImage(
        const PixelImage<uint8_t> &img,
        bool alpha = false)
    {
        return SDL_CreateRGBSurfaceFrom(
            (void*)&img.getData(0, 0, 0),
            img.getWidth(), img.getHeight(),
            img.getChannelCount() * 8,
            img.getWidth() * img.getChannelCount(),

            0x000000ff,
            0x0000ff00,
            0x00ff0000,

            alpha ?  0xff000000 : 0x0);
    }

    inline void GraphicalConsole_SDLRendererWrapper::commonInit()
    {
        assert(console);
        lastSeenBackBufferUpdate = 0;
        textBackSurf = makeSDLSurfaceForImage(
            *console->getBackbuffer(), true);
        lastUpdateTime = std::chrono::high_resolution_clock::now();
    }

    inline void GraphicalConsole_SDLRendererWrapper::initTextureForRenderer(
        SDL_Renderer *renderer)
    {
        if(texturesByRenderer[renderer]) {
            return;
        }

        SDL_Texture *tex = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            console->getBackbuffer()->getWidth(),
            console->getBackbuffer()->getHeight());

        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

        texturesByRenderer[renderer] = tex;
    }

    inline void GraphicalConsole_SDLRendererWrapper::deleteTextureForRenderer(
        SDL_Renderer *renderer)
    {
        auto it = texturesByRenderer.find(renderer);
        if(it != texturesByRenderer.end()) {
            SDL_DestroyTexture(it->second);
            texturesByRenderer.erase(renderer);
        }
    }

    inline void GraphicalConsole_SDLRendererWrapper::render(SDL_Renderer *renderer)
    {
        initTextureForRenderer(renderer);
        SDL_Texture *&tex = texturesByRenderer[renderer];

        // Animate in and out. Don't really care about accurate timesteps
        // here.

        std::chrono::time_point<std::chrono::high_resolution_clock> updateTime =
            std::chrono::high_resolution_clock::now();

        float timeDelta = float(std::chrono::duration_cast<std::chrono::microseconds>(
                updateTime - lastUpdateTime).count()) / 1000000.0f;

        float consoleSpeed = 4.0f;
        if(console->getActive()) {
            console->setVisibilty(console->getVisibility() + consoleSpeed * timeDelta);
        } else {
            console->setVisibilty(console->getVisibility() - consoleSpeed * timeDelta);
        }

        lastUpdateTime = updateTime;

        // Now handle stuff we only care about if the console really is visible.

        if(console->getVisibility()) {

            // Get half the windowsize.
            int winWidth = 0;
            int winHeight = 0;
            SDL_GetRendererOutputSize(renderer, &winWidth, &winHeight);
            winHeight /= 2;

            const PixelImage<uint8_t> *backBuffer = console->getBackbuffer();

            // Resize backbuffer if needed.
            if(int(backBuffer->getWidth()) != winWidth || int(backBuffer->getHeight()) != winHeight ||
                textBackSurf->w != winWidth || textBackSurf->h != winHeight)
            {
                console->setBackbufferSize(winWidth, winHeight);

                SDL_DestroyTexture(tex);
                SDL_FreeSurface(textBackSurf);

                textBackSurf = makeSDLSurfaceForImage(
                    *console->getBackbuffer(), true);

                tex = SDL_CreateTexture(
                    renderer,
                    SDL_PIXELFORMAT_ABGR8888,
                    SDL_TEXTUREACCESS_STREAMING,
                    winWidth, winHeight);

                SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
            }

            // Run an update if needed.
            if(console->needsUpdate()) {
                console->updateBackbuffer(true);
            }

            // Update texture with our backbuffer data.
            if(console->getBackbufferUpdateCount() != lastSeenBackBufferUpdate) {

                void *px = nullptr;
                int pitch = 0;

                SDL_LockTexture(tex, nullptr, &px, &pitch);

                assert(
                    pitch == console->getBackbuffer()->getWidth() * sizeof(ExPop::Gfx::Pixel));

                memcpy(px,
                    &console->getBackbuffer()->getData(0, 0, 0),
                    console->getBackbuffer()->getWidth() *
                    console->getBackbuffer()->getHeight() *
                    console->getBackbuffer()->getChannelCount() *
                    sizeof(uint8_t));

                SDL_UnlockTexture(tex);

                lastSeenBackBufferUpdate = console->getBackbufferUpdateCount();
            }

            // Display console.
            SDL_Rect dst;
            dst.w = winWidth;
            dst.h = winHeight;
            dst.x = 0;
            dst.y = -winHeight * (1.0f - console->getVisibility()); // * 0.75;
            // SDL_SetTextureAlphaMod(tex, 255 * console.getVisibility());
            SDL_RenderCopy(renderer, tex, NULL, &dst);
        }
    }

    inline GraphicalConsole_SDLRendererWrapper &getMainSDLConsoleRendererWrapper()
    {
        static GraphicalConsole_SDLRendererWrapper mainWrapper;
        return mainWrapper;
    }

    inline SDL_Renderer *ExPop_SDL_CreateRenderer(
        SDL_Window *window,
        int index,
        Uint32 flags)
    {
        SDL_Renderer *ret = SDL_CreateRenderer(window, index, flags);
        return ret;
    }

    inline void ExPop_SDL_DestroyRenderer(
        SDL_Renderer *renderer)
    {
        getMainSDLConsoleRendererWrapper().deleteTextureForRenderer(renderer);
        SDL_DestroyRenderer(renderer);
    }

    inline void ExPop_SDL_RenderPresent(
        SDL_Renderer *renderer)
    {
        getMainSDLConsoleRendererWrapper().render(renderer);
        SDL_RenderPresent(renderer);
    }

  #if EXPOP_ENABLE_SDL2_OVERRIDES

    // Lazy redirections.

  #ifdef SDL_CreateRenderer
  #undef SDL_CreateRenderer
  #endif
  #define SDL_CreateRenderer ExPop::ExPop_SDL_CreateRenderer

  #ifdef SDL_DestroyRenderer
  #undef SDL_DestroyRenderer
  #endif
  #define SDL_DestroyRenderer ExPop::ExPop_SDL_DestroyRenderer

  #ifdef SDL_RenderPresent
  #undef SDL_RenderPresent
  #endif
  #define SDL_RenderPresent ExPop::ExPop_SDL_RenderPresent

    // TODO: SDL_CreateWindowAndRenderer().

  #endif
}

#endif // EXPOP_ENABLE_SDL2

