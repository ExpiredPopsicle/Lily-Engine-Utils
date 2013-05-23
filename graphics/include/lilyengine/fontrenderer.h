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

// We can't forward-declare Font::StringActionOutput, so we'll
// actually include this.
#include "font.h"

namespace ExPop {

    namespace Gfx {

        class GLContext;

        /// A simple basic font renderer with a simple built-in
        /// shader, for the common use of just putting text on the
        /// screen.
        class FontRendererBasic {
        public:

            FontRendererBasic(GLContext *glc);
            virtual ~FontRendererBasic(void);

            /// Render text. Takes the output from a string action
            /// (StringActionOutput) where the
            /// STRINGACTION_BUILDRENDERINFO was set and
            /// STRINGACTION_DONTBUILDVBOS was not set.
            ///
            /// Coordinates x and y are in pixels. scrWidth and
            /// scrHeight are the screen width and height. They can be
            /// set to zero, in which case the system will determine
            /// it based off the GL viewport.
            ///
            /// Any OpenGL state touched by this will be left at the
            /// OpenGL default! Does not preserve stuff.
            ///
            /// Coordinates should be snapped to integers if you're
            /// really trying to draw in screen space, otherwise it
            /// will not be aligned to pixels correctly.
            ///
            /// Color defaults to grey (0.5, 0.5, 0.5, 1.0) so to make
            /// it white, you'll have to set the colorScale to 2.0,
            /// 2.0, 2.0. 1.0.
            void render(
                Font::StringActionOutput *output,
                float x, float y,
                const FVec4 &colorScale = FVec4(1.0, 1.0, 1.0, 1.0),
                float scrWidth = 0, float scrHeight = 0);

            /// Render a UTF-8 string to the screen with the current
            /// font. This is the most basic, idiot-proof way to
            /// render text. Has all the same gotchas as render().
            void renderTextDirect(
                Font *font,
                const std::string &text,
                float x, float y,
                const FVec4 &colorScale = FVec4(1.0, 1.0, 1.0, 1.0),
                bool wordWrap = true);

        private:

            unsigned int vertShader;
            unsigned int fragShader;
            unsigned int shaderProgram;

            int positionHandle;
            int uvHandle;
            int colorHandle;
            int matrixHandle;
            int vposHandle;
            int samplerHandle;

            int colorScaleHandle;

            GLContext *glc;

        };
    }
}

