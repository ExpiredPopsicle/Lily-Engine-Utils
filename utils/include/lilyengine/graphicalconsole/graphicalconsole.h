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

// Drop-in graphical console. The backend for this can run without any
// dependencies outside of the utilities lib that it lives in, but
// hooking it up to an actual graphics system may be a bit of legwork.

// ----------------------------------------------------------------------
// Inclusion/linkage/etc

// This one file should #include all the necessary headers for the
// entire console, but bits of functionality are enabled with some
// #defines...

//   EXPOP_ENABLE_SDL2 - Set to 1 to enable and expose SDL2 code. This
//     encompasses both input handling (via an event wrapper) and
//     rendering with the SDL_Renderer or old SDL software rendering
//     system. (lilyutils-config parameter: --enable-sdl2)

//   EXPOP_ENABLE_SDL2_OVERRIDES - Set to 0 to disable hacky SDL2
//     overrides. By default, SDL2 functions like SDL_GL_SwapWindow
//     and SDL_PollEvent are overridden to let the console get a
//     first-chance at keyboard input events and to allow it to draw
//     on top of the frame that the hosting application rendered
//     without any special modifications to the application code.
//     (Overrides are done with #defines. It's pretty gross.)
//     (lilyutils-config parameter: --disable-sdl2hacks)

//   EXPOP_ENABLE_GL - Set to 1 to enable fixed function GL code. This
//     only works if EXPOP_ENABLE_SDL2 is also enabled, for context
//     setup, context switching, and window size checking. A second
//     rendering context on SDL2 windows will be created if the
//     console is used. (lilyutils-config parameter: --enable-gl)

// ----------------------------------------------------------------------
// Global CVars and commands.

// Global variables can be turned into CVars like this...
//
//   int variable;
//   EXPOP_CVAR(cvarname, "Documentation string.", variable);

// Global functions can be turned into console commands like this...
//
//   int someCommand(int value, const std::string &someOtherValue);
//   EXPOP_COMMAND(commandname, "Documentation string.", someCommand);

// These macros create variables that hook up the command in their
// constructors. So they should probably be next to the definition of
// the functions and NOT in the header, or you'll end up with
// duplicate-symbol errors.

// You can also hook up console vars and commands manually...
//
//   ExPop::getMainConsole().setCVar("cvarname", "Documentation string.", variable);
//   ExPop::getMainConsole().setCommand("commandname", "Documentation string.", someCommand);

// Variables need to be types that have overloaded ostream
// operator<<() and operator>>(), or they will be unable to be
// converted to/from strings, and you may have some very confusing
// error messages. This applies to types used in function parameters,
// too.

// See graphicalconsole_typedescriptions.h for examples of how to make
// string descriptions for different types (for usage strings and help
// documentation).

// setContextualAutocompleteGenerator() can be used to create
// context-aware autocomplete lists. For example, strings that need to
// represent filenames.

// ----------------------------------------------------------------------
// Output

// ExPop::out() will return an std::ostream that outputs to the
// console and also stdout. Use utf-8. A subset of VT100 colors are
// supported in the console, but not always on the host operating
// system (like Windows). Color codes are per-line.

// ----------------------------------------------------------------------
// Actually using the thing.

// In the running application, press ctrl+backquote to open the
// console. Press it again to close the console.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#ifndef EXPOP_ENABLE_SDL2
#define EXPOP_ENABLE_SDL2 0
#endif

#ifndef EXPOP_ENABLE_GL
#define EXPOP_ENABLE_GL 0
#endif

#if EXPOP_ENABLE_SDL2
#ifndef EXPOP_ENABLE_SDL2_OVERRIDES
#define EXPOP_ENABLE_SDL2_OVERRIDES 1
#endif
#endif

#include <iostream>
#include <list>
#include <mutex>
#include <thread>
#include <functional>

#include "../pixelimage/pixelimage.h"
#include "../pixelimage/pixelimage_tga.h"
#include "../image.h"
#include "graphicalconsole_fontimage.h"

#include "../filesystem.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    class GraphicalConsole
    {
    public:
        GraphicalConsole();
        ~GraphicalConsole();

        void updateBackbuffer(bool force = false);
        void addLine(const std::string &text);
        void setBackbufferSize(size_t width, size_t height);
        bool needsUpdate() const;
        const PixelImage<uint8_t> *getBackbuffer() const;

        /// Use this to determine if your render layer has seen the most
        /// recent snapshot of the backbuffer.
        uint32_t getBackbufferUpdateCount() const;

        // ----------------------------------------------------------------------
        // Rendering tweaks

        void setBackgroundAlpha(float inAlpha);
        float getBackgroundAlpha() const;
        void setPrompt(const std::string &inPrompt);
        std::string getPrompt() const;

        // ----------------------------------------------------------------------
        // Text editing events

        void editMoveCursor(int amount);
        void editMoveCursorWord(int amount);
        void editMoveToEdge(int direction);
        void editInsertText(const std::string &utf8Text);
        void editDeleteText(size_t amount);
        void editSubmitText();
        void editAutocomplete();
        void editMoveHistory(int direction);

        // ----------------------------------------------------------------------
        // Text buffer display events

        /// Pageup/pagedown key event.
        void displayMovePage(int amount);

        // ----------------------------------------------------------------------
        // Convenience stuff

        /// Output to the console like a normal ostream. Line buffered.
        /// Not thread safe.
        std::ostream out;

        // ----------------------------------------------------------------------
        // Some values just for the hosting application

        float getVisibility() const;
        void setVisibilty(float newVisibility);

        bool getActive() const;
        void setActive(bool active);

        // ----------------------------------------------------------------------
        // Commands and Cvars

        template<typename ReturnType, typename ... ParameterTypes>
        void setCommand(
            const std::string &name,
            const std::string &doc,
            std::function<ReturnType(ParameterTypes...)> callback,
            bool useDefaultMissingArgs = false);

        template<typename ReturnType, typename ... ParameterTypes>
        inline void setCommand(
            const std::string &name,
            const std::string &doc,
            ReturnType(*callback)(ParameterTypes...),
            bool useDefaultMissingArgs = false);

        void clearCommand(const std::string &name);

        template<typename T>
        void setCVar(
            const std::string &name,
            const std::string &doc,
            T *value);

        void clearCommandOrCVar(const std::string &name);

        void runCommand(std::basic_string<uint32_t> &commandLine);
        void runCommand(std::string &commandLine);

        // ----------------------------------------------------------------------
        // Contextual autocomplete interface

        template<typename T>
        void setContextualAutocompleteGenerator(
            const std::string &commandOrCvarName,
            size_t parameterNumber,
            std::function<std::vector<T>()> function);

        void clearContextualAutocompleteGenerator(
            const std::string &commandOrCvarName,
            size_t parameterNumber);

    private:

        // ----------------------------------------------------------------------
        // Contextual autocomplete

        class ContextualAutocompleter_Base
        {
        public:
            virtual ~ContextualAutocompleter_Base();
            virtual std::vector<std::string> getResultsAsStrings();
        };

        template<typename T>
        class ContextualAutocompleter : public ContextualAutocompleter_Base
        {
        public:
            virtual ~ContextualAutocompleter();
            std::function<std::vector<T>()> func;
            std::vector<std::string> getResultsAsStrings() override;
        };

        std::map<
            std::basic_string<uint32_t>,
            std::map<size_t, ContextualAutocompleter_Base *> > contextualAutocompleters;

        // ----------------------------------------------------------------------
        // Command and CVar stuff

        class CommandOrVariableEntry_Base
        {
        public:
            CommandOrVariableEntry_Base();
            virtual ~CommandOrVariableEntry_Base();
            virtual void execute(const std::vector<std::string> &arguments);
            virtual std::string getUsageString() const;

            std::string name;
            std::string docString;
            enum { CVAR, COMMAND } type;
            GraphicalConsole *parentConsole;
        };

        // Command entry. For stuff tied to actual functions. Need a
        // specific parameter count, or it'll automatically throw an error
        // when attempting to use it. No errors thrown on type decoding
        // errors.
        template<typename ReturnType, typename ... ParameterTypes>
        class CommandEntry : public CommandOrVariableEntry_Base
        {
        public:
            CommandEntry();
            virtual ~CommandEntry();
            void execute(const std::vector<std::string> &arguments) override;
            std::string getUsageString() const override;

            std::function<ReturnType(ParameterTypes...)> func;
            bool useDefaultMissingArgs;
        };

        // CVar entry. For stuff tied to specific values. Executing
        // without any value specified will simply show the current value.
        // Executing with a single parameter will set the value.
        template<typename T>
        class VariableEntry : public CommandOrVariableEntry_Base
        {
        public:
            VariableEntry();
            virtual ~VariableEntry();
            std::string getUsageString() const override;
            void execute(const std::vector<std::string> &arguments) override;

            T *value;
        };

        std::map<std::basic_string<uint32_t>, CommandOrVariableEntry_Base *> commandsAndCvars;

        // ----------------------------------------------------------------------
        // ostream related stuff.

        // Specialized streambuf to output to this GraphicalConsole in
        // a thread-safe manner. See "ExPop::out()" for most
        // convenient use.
        class GraphicalConsoleStreambuf : public std::streambuf
        {
        public:
            GraphicalConsoleStreambuf(GraphicalConsole *inParent);
            int overflow(int c) override;
        private:
          #if EXPOP_ENABLE_THREADS
            Threads::Mutex buffersMutex;
            std::map<Threads::ThreadId, std::string> buffersByThread;
          #else
            std::string singleThreadedBuffer;
          #endif
            GraphicalConsole *parent;
        };
        GraphicalConsoleStreambuf streamBufOut;

        // True if we should mirror console output to stdout. Defaults
        // to enabled.
        bool writeCout;

        // ----------------------------------------------------------------------
        // Graphics state.

        PixelImage<uint8_t> *outlinedFontImg;
        PixelImage<uint8_t> *gradImg;
        PixelImage<uint8_t> *gradientsByColor[8*3];
        PixelImage<uint8_t> *backBuffer;
        bool backBufferIsDirty;
        float bgAlpha;
        float visibility;
        uint32_t backbufferUpdateCount;

        // ----------------------------------------------------------------------
        // Text buffer state.

        static const size_t lineRingBufferSize = 1024; // Must be a power of two.
        std::string lineRingBuffer[lineRingBufferSize];
        size_t lineRingBufferIndex; // Must be unsigned - relies on integer underflow.
      #if EXPOP_ENABLE_THREADS
        Threads::Mutex lineRingBufferMutex;
      #endif

        std::basic_string<uint32_t> editLineBuffer;
        int editLineCursorLocation;
        int viewOffset;
        std::vector<std::basic_string<uint32_t> > commandHistory;
        int historyPosition;
        std::string prompt;

        void addHistory(const std::basic_string<uint32_t> &newLine);
        void showHelp(const std::string &name);
        std::vector<std::string> showHelpAutoCompleter();

        // ----------------------------------------------------------------------
        // Convenience stuff for the hosting application.

        bool active;
    };

    /// Add new versions of this function to create textual
    /// descriptions for different types to appear in the usage text
    /// for command help.
    template<typename T>
    inline std::string graphicalConsoleGetTypeDescription();

    /// Get a singleton GraphicalConsole instance. (Singletons are
    /// evil, but we sometimes have to make exceptions for things like
    /// debugging tools.)
    inline GraphicalConsole *getMainConsole();

    /// Check to see if the main console is still active (has not had
    /// its constructor called). This is to deal with some static
    /// destruction order shenanigans.
    inline bool &getMainConsoleAvailable();

    /// Get the main (singleton) console's output stream directly.
    /// Note that this is buried in the ExPop namespace, so it
    /// shouldn't be a problem to name it this.
    ///
    /// Do not use this from code that may be active in other threads
    /// threads after main() has completed, because it may call
    /// functions on the main console after the main console's
    /// destructor has been called.
    inline std::ostream &out();

    /// Convenience function for setting up contextual autocomplete
    /// for filenames.
    inline std::function<std::vector<std::string>()> graphicalConsoleFileTreeGeneratorGenerator(
        const std::string &path,
        const std::string &matchExtension = "",
        bool includeDirectories = false);

    /// Convenience macro to mark something as a console variable.
  #define EXPOP_CVAR(name, doc, ptr)                                        \
    ExPop::GraphicalConsoleCVarSetter<std::decay<decltype(*ptr)>::type >    \
    expopGlobalCVar_##name(#name, doc, ptr)

    /// Convenience macro to mark something as a console command.
  #define EXPOP_COMMAND(name, doc, cmd)                                     \
    ExPop::GraphicalConsoleCommandSetter<std::decay<decltype(cmd)>::type >    \
    expopGlobalCommand_##name(#name, doc, cmd)
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

#include "graphicalconsole_common.h"
#include "graphicalconsole_builtincommands.h"
#include "graphicalconsole_gfx.h"
#include "graphicalconsole_textediting.h"
#include "graphicalconsole_typedescriptions.h"
#include "graphicalconsole_stream.h"
#include "graphicalconsole_entries.h"
#include "graphicalconsole_autocomplete.h"
#include "graphicalconsole_globalcvars.h"
#include "graphicalconsole_sdlinput.h"
#include "graphicalconsole_sdlrenderer.h"
#include "graphicalconsole_sdlsoftware.h"
#include "graphicalconsole_sdlgl.h"

// ----------------------------------------------------------------------

namespace ExPop
{
    inline bool &getMainConsoleAvailable()
    {
        static bool mainConsoleAvailable = true;
        return mainConsoleAvailable;
    }

    inline GraphicalConsole *getMainConsole()
    {
        static GraphicalConsole mainConsole;
        return &mainConsole;
    }

    inline uint32_t GraphicalConsole::getBackbufferUpdateCount() const
    {
        return backbufferUpdateCount;
    }

    inline bool GraphicalConsole::needsUpdate() const
    {
        return backBufferIsDirty;
    }

    inline void GraphicalConsole::setBackbufferSize(size_t width, size_t height)
    {
        if(width != size_t(backBuffer->getWidth()) || height != size_t(backBuffer->getHeight())) {
            *backBuffer = PixelImage<uint8_t>(width, height, 4);
            backBufferIsDirty = true;
        }
    }

    inline GraphicalConsole::GraphicalConsole() :
        out(&streamBufOut),
        streamBufOut(this)
    {
        backBufferIsDirty = true;
        lineRingBufferIndex = 0;
        editLineCursorLocation = 0;
        viewOffset = 0;
        historyPosition = -1;
        visibility = 0.0f;
        bgAlpha = 0.75f;
        writeCout = true;
        active = false;
        backbufferUpdateCount = 0;
        prompt = "> ";

        // Load the font up and convert it something with outlines for
        // better legibility.

        // We actually compressed the font image with zlib here, so we
        // need to pull it out, copy it, decompress it, and then load
        // it up as a TGA.
        {
            PixelImage<uint8_t> *fontImg = loadBuiltinFont();

            Gfx::makeBlackTransparent(*fontImg);
            outlinedFontImg = Gfx::generateOutlinedFontMask(*fontImg);
            delete fontImg;
        }

        // Make our background texture thing big enough to hold any single
        // letter.
        size_t blankSize = std::max(outlinedFontImg->getWidth() / 16, outlinedFontImg->getHeight() / 16);

        // Generate a cool gradient for some text.
        gradImg = makeGradientImage(
            blankSize,
            ExPop::FVec3(1.0f, 0.5f, 0.0f),
            ExPop::FVec3(1.0f, 1.0f, 0.0f),
            ExPop::FVec3(1.0f, 0.5f, 0.0f)
            );

        // Generate gradient images for each text color we might use.
        // Outer loop is brightness levels. Inner loop is three-bit
        // number with bits corresponding to red, green, and blue.
        for(int n = 0; n < 3; n++) {

            for(int i = 0; i < 8; i++) {

                // Split inner loop counter into individual color
                // bits.
                float r = !!(i & 1);
                float g = !!(i & 2);
                float b = !!(i & 4);

                // Adjust brightness based on outer loop value.
                float brightnessScale = 0.5f + 0.5f * float(n);

                // For the brightest color, we're actually going to
                // push it a little towards white. It'll probably
                // appear "lighter" rather than "bright" this way, but
                // I want the full color value present in the "middle"
                // color.
                ExPop::FVec3 bloom(0.0f, 0.0f, 0.0f);
                if(n == 2) {
                    float bloomAmt = 0.5f;
                    bloom = ExPop::FVec3(bloomAmt, bloomAmt, bloomAmt);
                }

                // Make a 3-color gradient that's slightly brighter on
                // the top than the bottom, with the representative
                // color in the middle.
                gradientsByColor[i + n * 8] = makeGradientImage(
                    blankSize,
                    ExPop::FVec3(r, g, b) * 0.5f  * brightnessScale,
                    ExPop::FVec3(r, g, b) * 0.75f * brightnessScale + bloom,
                    ExPop::FVec3(r, g, b) * 0.25f * brightnessScale);
            }
        }

        // Make some default-sized back buffer. This can be modified
        // later.
        backBuffer = new PixelImage<uint8_t>(320, 480, 4); // FIXME: Artificially small for wrapping tests.

        // Built-in commands.
        setCommand(
            "help", "Show a list of commands and variables, with a short description of each.",
            std::function<void(const std::string &)>(
                std::bind(&GraphicalConsole::showHelp, this, std::placeholders::_1)), true);

        setContextualAutocompleteGenerator(
            "help", 0,
            std::function<std::vector<std::string>()>(
                std::bind(
                    &GraphicalConsole::showHelpAutoCompleter,
                    this)));
    }

    inline GraphicalConsole::~GraphicalConsole()
    {
        getMainConsoleAvailable() = false;

        delete outlinedFontImg;
        delete gradImg;
        delete backBuffer;
        for(size_t i = 0; i < sizeof(gradientsByColor) / sizeof(gradientsByColor[0]); i++) {
            delete gradientsByColor[i];
        }
        for(auto i = commandsAndCvars.begin(); i != commandsAndCvars.end(); i++) {
            delete i->second;
        }
        for(auto i = contextualAutocompleters.begin(); i != contextualAutocompleters.end(); i++) {
            for(auto k = i->second.begin(); k != i->second.end(); k++) {
                delete k->second;
            }
        }
    }

    inline void GraphicalConsole::addLine(const std::string &text)
    {
      #if EXPOP_ENABLE_THREADS
        lineRingBufferMutex.lock();
      #endif

        lineRingBuffer[lineRingBufferIndex] =
            stringUTF32ToCodepage437(stringUTF8ToUTF32(text));
        lineRingBufferIndex++;
        lineRingBufferIndex %= lineRingBufferSize;

        if(writeCout) {
            std::cout << text << "\e[0m" << std::endl;
        }

        backBufferIsDirty = true;

      #if EXPOP_ENABLE_THREADS
        lineRingBufferMutex.unlock();
      #endif
    }


    inline float GraphicalConsole::getVisibility() const
    {
        return visibility;
    }

    inline void GraphicalConsole::setVisibilty(float newVisibility)
    {
        if(newVisibility > 1.0f) newVisibility = 1.0f;
        if(newVisibility < 0.0f) newVisibility = 0.0f;
        visibility = newVisibility;
    }

    inline bool GraphicalConsole::getActive() const
    {
        return active;
    }

    inline void GraphicalConsole::setActive(bool active)
    {
        this->active = active;
    }

    inline void GraphicalConsole::setBackgroundAlpha(float inAlpha)
    {
        bgAlpha = inAlpha;
        backBufferIsDirty = true;
    }

    inline float GraphicalConsole::getBackgroundAlpha() const
    {
        return bgAlpha;
    }

    inline void GraphicalConsole::setPrompt(const std::string &inPrompt)
    {
        if(prompt != inPrompt) {
            prompt = inPrompt;
            backBufferIsDirty = true;
        }
    }

    inline std::string GraphicalConsole::getPrompt() const
    {
        return prompt;
    }

    inline void GraphicalConsole::runCommand(std::string &commandLine)
    {
        std::string utf8Line = commandLine;

        // Strip off first word for command/cvar name.
        std::basic_string<uint32_t> cmdName = ExPop::stringUTF8ToUTF32(
            graphicalConsoleReadParam(utf8Line));

        // Execute it.
        auto itr = commandsAndCvars.find(cmdName);
        if(itr != commandsAndCvars.end()) {

            std::vector<std::string> parts =
                graphicalConsoleSplitLine(utf8Line);

            itr->second->execute(parts);

        } else {
            out << graphicalConsoleGetRedErrorText()
                << ": Bad command or variable name: \""
                << ExPop::stringUTF32ToUTF8(cmdName) << "\"" << std::endl;
        }

        backBufferIsDirty = true;
    }

    inline void GraphicalConsole::runCommand(std::basic_string<uint32_t> &commandLine)
    {
        std::basic_string<uint32_t> line = commandLine;
        std::string utf8Line = ExPop::stringUTF32ToUTF8(line);
        runCommand(utf8Line);
    }

}

