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

// Console input support for SDL2. Handles enabling/disabling text
// input and receiving events for the console in a way that *should*
// be invisible to the hosting application, under normal usage.
// (Contains some macro abuse to make this happen.)

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#if EXPOP_ENABLE_SDL2

#pragma once

#include <SDL.h>

// FIXME: Replace with actual path to needed parts. Probably string
// and image.
#include <lilyengine/utils.h>

#include "graphicalconsole.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    class GraphicalConsole_SDLInputWrapper
    {
    public:

        GraphicalConsole_SDLInputWrapper(
            GraphicalConsole *inConsole);
        GraphicalConsole_SDLInputWrapper();
        ~GraphicalConsole_SDLInputWrapper();

        /// Give the input wrapper a first-chance at input events.
        /// Returns true if the event is consumed (and should not be
        /// interpreted by the hosting application).
        bool handleEvent(SDL_Event &event);

        /// Use this instead of SDL_StartTextInput() and
        /// SDL_StopTextInput().
        void setTextInputState(bool active);

        bool getTextInputState() const;

    private:

        GraphicalConsole *console;
        bool shadowTextInputState;

        void makeTextInputStateConsistent();
    };

    /// SDL input function wrapper that will filter messages to the
    /// console system and hide them from the host application if
    /// they're consumed by the console. Use this instead of
    /// SDL_PollEvent for simple event handling on the main console.
    /// (SDL_PollEvent is later #defined to this, so redirection
    /// should be automatic.)
    inline int ExPop_SDL_PollEvent(SDL_Event *event);

    /// SDL function overrides for enabling and disabling text input
    /// so we can keep our shadow state and the actual text input
    /// state consistent.
    inline void ExPop_SDL_StartTextInput(void);
    inline void ExPop_SDL_StopTextInput(void);
    inline Uint8 ExPop_SDL_EventState(Uint32 type, int state);
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{

    inline GraphicalConsole_SDLInputWrapper &getMainSDLConsoleInputWrapper()
    {
        static GraphicalConsole_SDLInputWrapper mainWrapper;
        return mainWrapper;
    }

    inline int ExPop_SDL_PollEvent(SDL_Event *event)
    {
        SDL_Event tmpEvent = {0};
        int ret = 0;
        bool consoleConsumedEvent = false;

        do {

            ret = SDL_PollEvent(&tmpEvent);

            // No more events?
            if(!ret) {
                return ret;
            }

            // Console first-chance at event.
            consoleConsumedEvent =
                getMainSDLConsoleInputWrapper().handleEvent(tmpEvent);

        } while(consoleConsumedEvent);

        // Console didn't eat this event.
        *event = tmpEvent;
        return ret;
    }

    inline void ExPop_SDL_StartTextInput(void)
    {
        getMainSDLConsoleInputWrapper().setTextInputState(true);
    }

    inline void ExPop_SDL_StopTextInput(void)
    {
        getMainSDLConsoleInputWrapper().setTextInputState(false);
    }

    GraphicalConsole_SDLInputWrapper::GraphicalConsole_SDLInputWrapper()
    {
        console = getMainConsole();
        shadowTextInputState = SDL_IsTextInputActive();
    }

    GraphicalConsole_SDLInputWrapper::GraphicalConsole_SDLInputWrapper(
        GraphicalConsole *inConsole)
    {
        console = inConsole;

        // Restore text input state.
        if(shadowTextInputState != SDL_IsTextInputActive()) {
            if(shadowTextInputState) {
                SDL_StartTextInput();
            } else {
                SDL_StopTextInput();
            }
        }
    }

    GraphicalConsole_SDLInputWrapper::~GraphicalConsole_SDLInputWrapper()
    {
    }

    void GraphicalConsole_SDLInputWrapper::setTextInputState(bool active)
    {
        shadowTextInputState = active;
        makeTextInputStateConsistent();
    }

    bool GraphicalConsole_SDLInputWrapper::getTextInputState() const
    {
        return shadowTextInputState;
    }

    void GraphicalConsole_SDLInputWrapper::makeTextInputStateConsistent()
    {
        if(console->getActive()) {

            if(!SDL_IsTextInputActive()) {
                SDL_StartTextInput();
            }

        } else {

            if(SDL_IsTextInputActive() != shadowTextInputState) {

                if(shadowTextInputState) {
                    SDL_StartTextInput();
                } else {
                    SDL_StopTextInput();
                }

            }

        }
    }

    bool GraphicalConsole_SDLInputWrapper::handleEvent(SDL_Event &event)
    {
        // Always possible regardless of activeness: Backquote keydown,
        // which will make it active.
        if(event.type == SDL_KEYDOWN) {

            if(event.key.keysym.sym == SDLK_BACKQUOTE && (SDL_GetModState() & KMOD_CTRL)) {

                console->setActive(!console->getActive());
                makeTextInputStateConsistent();

                return true;
            }
        }

        if(console->getActive()) {

            if(event.type == SDL_TEXTINPUT) {
                console->editInsertText(event.text.text);
                return true;
            }

            if(event.type == SDL_KEYUP) {
                return true;
            }

            if(event.type == SDL_KEYDOWN) {

                switch(event.key.keysym.sym) {

                    case SDLK_LEFT:
                        if(SDL_GetModState() & KMOD_CTRL) {
                            console->editMoveCursorWord(-1);
                        } else {
                            console->editMoveCursor(-1);
                        }
                        return true;

                    case SDLK_RIGHT:
                        if(SDL_GetModState() & KMOD_CTRL) {
                            console->editMoveCursorWord(1);
                        } else {
                            console->editMoveCursor(1);
                        }
                        return true;

                    case SDLK_BACKSPACE:
                        console->editMoveCursor(-1);
                        console->editDeleteText(1);
                        return true;

                    case SDLK_DELETE:
                        console->editDeleteText(1);
                        return true;

                    case SDLK_HOME:
                        console->editMoveToEdge(-1);
                        return true;

                    case SDLK_END:
                        console->editMoveToEdge(1);
                        return true;

                    case SDLK_RETURN:
                        console->editSubmitText();
                        return true;

                    case SDLK_INSERT:
                        if(SDL_GetModState() & KMOD_SHIFT) {
                            console->editInsertText(SDL_GetClipboardText());
                        }
                        return true;

                    case SDLK_v:
                        if(SDL_GetModState() & KMOD_SHIFT) {
                            console->editInsertText(SDL_GetClipboardText());
                        }
                        return true;

                    case SDLK_PAGEUP:
                        console->displayMovePage(8);
                        return true;

                    case SDLK_PAGEDOWN:
                        console->displayMovePage(-8);
                        return true;

                    case SDLK_UP:
                        console->editMoveHistory(1);
                        return true;

                    case SDLK_DOWN:
                        console->editMoveHistory(-1);
                        return true;

                    case SDLK_TAB:
                        console->editAutocomplete();
                        return true;

                    default:
                        return true;
                }
            }
        }

        return false;
    }

    // SDL_StartTextInput() and SDL_StopTextInput() affect an internal
    // bit that it's possible to query and manipulate outside of those
    // functions. We need to wrap up that functionality too for the
    // sake of completeness. See SDL_EventState().
    inline Uint8 ExPop_SDL_EventState(Uint32 type, int state)
    {
        // Pass through non-textinput events.
        if(type == SDL_TEXTINPUT) {

            if(state == SDL_QUERY) {
                bool enabled = getMainSDLConsoleInputWrapper().getTextInputState();
                return enabled ? SDL_ENABLE : SDL_DISABLE;
            }

            if(state == SDL_IGNORE || state == SDL_DISABLE) {
                getMainSDLConsoleInputWrapper().setTextInputState(false);
            } else if(state == SDL_ENABLE) {
                getMainSDLConsoleInputWrapper().setTextInputState(true);
            }

        }

        return SDL_EventState(type, state);
    }

  #if EXPOP_ENABLE_SDL2_OVERRIDES

  #ifdef SDL_PollEvent
  #undef SDL_PollEvent
  #endif
  #define SDL_PollEvent(x) ExPop::ExPop_SDL_PollEvent(x)

  #ifdef SDL_StartTextInput
  #undef SDL_StartTextInput
  #endif
  #define SDL_StartTextInput() ExPop::ExPop_SDL_StartTextInput()

  #ifdef SDL_StopTextInput
  #undef SDL_StopTextInput
  #endif
  #define SDL_StopTextInput() ExPop::ExPop_SDL_StopTextInput()

  #ifdef SDL_EventState
  #undef SDL_EventState
  #endif
  #define SDL_EventState(x, y) ExPop::ExPop_SDL_EventState(x, y)

  #endif
}

#endif // EXPOP_ENABLE_SDL2

