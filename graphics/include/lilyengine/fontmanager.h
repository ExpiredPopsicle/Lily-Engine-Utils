// ---------------------------------------------------------------------------
//
//   Lily Engine alpha
//
//   Copyright (c) 2012 Clifford Jolly
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

#pragma once

#include <string>
#include <map>

namespace ExPop {

    namespace Gfx {

        class GLContext;
        class FontManagerInternal;
        class FontRendererBasic;
        class Font;
        class Image;

        /// FontManager just keeps track of multiple fonts and loads
        /// them on demand.
        class FontManager {
        public:

            FontManager(GLContext *glc);
            ~FontManager(void);

            /// Add a font explicitly to the list of loaded fonts
            /// instead of loading it normally. Font system takes
            /// ownership of the buffer for the font file and will
            /// delete it when the Font goes away.
            Font *createFont(
                const std::string &name,
                char *buffer,
                int bufferLength);

            /// Create a font from an image. At the moment these are
            /// limited to pure ASCII (single-page) fonts with a fixed
            /// size per character. 16 rows of 16 characters, evenly
            /// spaced. Image size must be a power of two in each
            /// dimension. Font system takes ownership of the Image
            /// object and will delete it when the Font goes away.
            Font *createImageFont(
                const std::string &name,
                Image *img);

            /// Get a font by filename.
            Font *getFont(const std::string &name);

            /// Install (and possibly overwrite) a Font explicitly
            /// into the list of fonts. Use this if you used
            /// createFont() or createImageFont() directly.
            void setFont(const std::string &name, Font *font);

            /// Fastest way to just get text on the screen. Doesn't
            /// have the performance benefits granted by preserving
            /// StringActionOutput objects or UTF-32 converted strings
            /// between frames. Also generates a FontRendererBasic
            /// object, with all the shader compilation that goes with
            /// that, the first time it's used.
            void putTextOnScreen(
                const std::string &str,
                const std::string &fontName = "builtIn",
                float x = 0, float y = 0,
                float wordWrapWidth = -1);

        private:

            FontRendererBasic *builtInFontRenderer;
            GLContext *glc;
            FontManagerInternal *internal;
            std::map<std::string, Font*> loadedFonts;
        };

    }

}

