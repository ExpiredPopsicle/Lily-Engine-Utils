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

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#if EXPOP_ENABLE_SDL2
#if EXPOP_ENABLE_GL

#include <SDL.h>
#include <GL/gl.h>

#include "graphicalconsole.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    class GraphicalConsole_SDLGLWrapper
    {
    public:

        GraphicalConsole_SDLGLWrapper();
        GraphicalConsole_SDLGLWrapper(
            GraphicalConsole *inConsole);
        ~GraphicalConsole_SDLGLWrapper();

        void render(SDL_Window *window);

        void initWindow(SDL_Window *window);
        void shutdownWindow(SDL_Window *window);

    private:

        struct PerWindowState
        {
            SDL_GLContext glContext;
            GLuint texture;
            uint32_t lastSeenBackBufferUpdate;
        };

        std::map<SDL_Window *, PerWindowState> statesPerWindow;
        GraphicalConsole *console;
    };
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    inline GraphicalConsole_SDLGLWrapper::GraphicalConsole_SDLGLWrapper()
    {
        console = getMainConsole();
    }

    inline GraphicalConsole_SDLGLWrapper::GraphicalConsole_SDLGLWrapper(
            GraphicalConsole *inConsole)
    {
        console = inConsole;
    }

    inline GraphicalConsole_SDLGLWrapper::~GraphicalConsole_SDLGLWrapper()
    {
        if(statesPerWindow.size()) {
            ExPop::out()
                << ExPop::graphicalConsoleGetRedErrorText()
                << ": Graphical console GL resources still remaining at shutdown time. Improper shutdown?"
                << std::endl;
        }
    }

    inline void GraphicalConsole_SDLGLWrapper::render(SDL_Window *window)
    {
        if(console->getActive()) {

            auto it = statesPerWindow.find(window);
            if(it != statesPerWindow.end()) {

                // Save old state and switch to the one we need to
                // render this.
                SDL_GLContext oldContext = SDL_GL_GetCurrentContext();
                SDL_Window *oldWindow = SDL_GL_GetCurrentWindow();
                PerWindowState &state = it->second;
                SDL_GL_MakeCurrent(window, state.glContext);

                // Determine correct console size.
                int winWidth = 0;
                int winHeight = 0;
                SDL_GetWindowSize(window, &winWidth, &winHeight);
                glViewport(0, 0, winWidth, winHeight);
                winHeight /= 2;

                // Update backbuffer to match.
                console->setBackbufferSize(winWidth, winHeight);
                console->updateBackbuffer();

                glBindTexture(GL_TEXTURE_2D, state.texture);

                // Re-upload the whole texture if we need to.
                if(console->getBackbufferUpdateCount() != state.lastSeenBackBufferUpdate) {
                    glTexImage2D(
                        GL_TEXTURE_2D,
                        0,
                        GL_RGBA,
                        console->getBackbuffer()->getWidth(),
                        console->getBackbuffer()->getHeight(),
                        0,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        &console->getBackbuffer()->getData(0, 0, 0));
                    state.lastSeenBackBufferUpdate = console->getBackbufferUpdateCount();
                }

                // GL immediate mode atrocity. FIXME: If we ever
                // switch to something modern, we should only have to
                // replace this part (and add some stuff in other
                // places), but not have to delete much else.
                glBegin(GL_QUADS);
                glTexCoord2f( 0.0f, 0.0f);
                glVertex2f(  -1.0f, 1.0f);
                glTexCoord2f( 1.0f, 0.0f);
                glVertex2f(   1.0f, 1.0f);
                glTexCoord2f( 1.0f, 1.0f);
                glVertex2f(   1.0f, 0.0f);
                glTexCoord2f( 0.0f, 1.0f);
                glVertex2f(  -1.0f, 0.0f);
                glEnd();

                // Switch back to the context that was active before.
                // Pretend nothing changed.
                SDL_GL_MakeCurrent(oldWindow, oldContext);
            }
        }
    }

    inline void GraphicalConsole_SDLGLWrapper::initWindow(SDL_Window *window)
    {
        if(statesPerWindow.find(window) == statesPerWindow.end()) {

            SDL_GLContext oldContext = SDL_GL_GetCurrentContext();
            SDL_Window *oldWindow = SDL_GL_GetCurrentWindow();

            PerWindowState state;

            // Save all GL attribs, so we can safely clobber and
            // restore them.

            // FIXME: SDL_GL_CONTEXT_RELEASE_BEHAVIOR Chosen because
            // it was the last in the SDL_GLattr enum.
            const size_t numSDLGLAttribs = SDL_GL_CONTEXT_RELEASE_BEHAVIOR + 1;

            // We might fail to read and set some of these attributes.
            // We'll hide the error from the hosting application
            // anyway.
            int savedGLAttribs[numSDLGLAttribs];
            std::string existingError = SDL_GetError();

            for(size_t i = 0; i < numSDLGLAttribs; i++) {
                SDL_GL_GetAttribute(SDL_GLattr(i), &savedGLAttribs[i]);
            }

            SDL_GL_ResetAttributes();

            // Ceate the context (this also makes it current).
            state.glContext = SDL_GL_CreateContext(window);
            state.texture = 0;
            state.lastSeenBackBufferUpdate = 0;

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);

            // Set up our one texture.
            glEnable(GL_TEXTURE_2D);
            glGenTextures(1, &state.texture);
            glBindTexture(GL_TEXTURE_2D, state.texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            statesPerWindow[window] = state;

            // Restore GL attributes and the context.
            for(size_t i = 0; i < numSDLGLAttribs; i++) {
                SDL_GL_SetAttribute(SDL_GLattr(i), savedGLAttribs[i]);
            }
            SDL_SetError("%s", existingError.c_str());
            SDL_GL_MakeCurrent(oldWindow, oldContext);
        }
    }

    inline void GraphicalConsole_SDLGLWrapper::shutdownWindow(SDL_Window *window)
    {
        auto it = statesPerWindow.find(window);
        if(it != statesPerWindow.end()) {

            PerWindowState &state = it->second;

            glDeleteTextures(1, &state.texture);
            SDL_GL_DeleteContext(state.glContext);

            statesPerWindow.erase(window);
        }
    }


    inline GraphicalConsole_SDLGLWrapper &getMainSDLConsoleGLWrapper()
    {
        static GraphicalConsole_SDLGLWrapper mainWrapper;
        return mainWrapper;
    }

    inline SDL_Window *ExPop_SDL_CreateWindow(
        const char *title,
        int x, int y,
        int w, int h,
        Uint32 flags)
    {
        SDL_Window *window = SDL_CreateWindow(title, x, y, w, h, flags);

        if((flags & SDL_WINDOW_OPENGL) && window) {
            getMainSDLConsoleGLWrapper().initWindow(window);
        }

        return window;
    }

    inline void ExPop_SDL_GL_SwapWindow(SDL_Window *window)
    {
        getMainSDLConsoleGLWrapper().render(window);
        SDL_GL_SwapWindow(window);
    }

    inline void ExPop_SDL_DestroyWindow(SDL_Window *window)
    {
        getMainSDLConsoleGLWrapper().shutdownWindow(window);
        SDL_DestroyWindow(window);
    }

  #if EXPOP_ENABLE_SDL2_OVERRIDES

    // Lazy mode redirects.

  #ifdef SDL_CreateWindow
  #undef SDL_CreateWindow
  #endif
  #define SDL_CreateWindow ExPop::ExPop_SDL_CreateWindow

  #ifdef SDL_GL_SwapWindow
  #undef SDL_GL_SwapWindow
  #endif
  #define SDL_GL_SwapWindow ExPop::ExPop_SDL_GL_SwapWindow

  #ifdef SDL_DestroyWindow
  #undef SDL_DestroyWindow
  #endif
  #define SDL_DestroyWindow ExPop::ExPop_SDL_DestroyWindow

    // TODO: SDL_CreateWindowFrom().

  #endif
}

#endif // EXPOP_ENABLE_GL
#endif // EXPOP_ENABLE_SDL2

