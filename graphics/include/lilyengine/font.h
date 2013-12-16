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

#include <lilyengine/utils.h>

namespace ExPop {

    namespace Gfx {

        class Image;
        class GLContext;
        class FontInternal;
        class FontPage;

        /// Single font in the Font system. Can use FreeType2 fonts
        /// for Unicode goodness, or bitmap ASCII-only fixed-width
        /// fonts.
        ///
        /// To use it, make a new Font with the FontManager's
        /// createFont() function. doStringAction() will generate
        /// rendering information (or whatever you ask from it) for a
        /// UTF-32 string. You can then use that directly, or just use
        /// the stupidRender() function if all you want is simple text
        /// on the screen.
        class Font {
        public:

            class StringActionOutput;
            class StringActionInput;

            Font(GLContext *glc);
            ~Font(void);

            /// Font types.
            enum FontType {
                FONTTYPE_TRUETYPE,
                FONTTYPE_BITMAP,
            };

            /// Flags for doStringAction.
            enum StringActionBits {
                STRINGACTION_NOBITS          = 0,
                STRINGACTION_WORDWRAP        = 1,  // Do word wrapping.
                STRINGACTION_VT100COLOR      = 2,  // Emulate VT100 color codes. With this, default color is grey. White for bold.
                STRINGACTION_BUILDRENDERINFO = 4,  // Create rendering info.
                STRINGACTION_DONTBUILDVBOS   = 8,  // Don't build VBOs. Keep stuff in system memory.
            };

            /// The main meat of the font system happens here. Do
            /// something with a string of text (like generate vertex
            /// buffers to render, determine cursor position,
            /// etc). This will probably end up issuing GL calls to
            /// build font textures and vertex buffer objects, so make
            /// sure your rendering context is all set up. Returns a
            /// new instance of StringActionOutput.
            StringActionOutput *doStringAction(
                const std::vector<unsigned int> &str,
                StringActionBits stringActionBits = STRINGACTION_NOBITS,
                StringActionInput *inputVals = NULL);

            // TODO: Remove this. (Or leave, but just for debugging and reference.)

            /// Just get the verticalAdvance variable.
            float getVerticalAdvance(void);

            /// Get a font page. Render it if it doesn't exist already.
            FontPage *getPage(unsigned int pageNum);

            // ----------------------------------------------------------------------
            // RenderInfo
            // ----------------------------------------------------------------------

            /// All the output info from a string action needed to
            /// render a font.
            class RenderInfo {
            public:

                /// Single vertex in a font rendering stream.
                class FontVertex {
                public:

                    FontVertex(
                        const FVec3 &position,
                        const FVec2 &uv,
                        const FVec3 &color,
                        float verticalPos,
                        float characterIndex);

                    FVec3 position;
                    FVec3 color;
                    FVec2 uv;

                    // Position up from the baseline (or negative for
                    // below it).
                    float verticalPos;

                    float characterIndex;

                private:
                };

                /// Collection of font rendering info for a single
                /// texture. A full rendering operation could have
                /// multiple textures if the string expands across
                /// multiple pages of characters.
                class FontVertexStream {
                public:

                    FontVertexStream(unsigned int glTextureNumber);
                    ~FontVertexStream(void);

                    std::vector<FontVertex> vertices;
                    std::vector<unsigned short> indices;

                    /// Actual number of characters rendered into
                    /// this. Not available storage space.
                    unsigned int numCharacters;

                    /// Fonts can be split across multiple textures
                    /// and each one of these FontVertexStream objects
                    /// corresponds to the render data for a single
                    /// texture. This is the GL texture number.
                    unsigned int glTextureNumber;

                    // All this info is created when building the
                    // vertex buffer object and index buffer object.

                    /// GL buffer straight from the array of
                    /// FontVertex data. Complete with whatever silly
                    /// alignment and padding that structure might
                    /// have inside of it.
                    unsigned int vbo;

                    /// GL Triangle indexes buffer.
                    unsigned int indexBuffer;

                    /// Number of indices in the GL buffer objects.
                    unsigned int numIndices;

                    /// Build a Vertex Buffer Object for this after
                    /// we're done putting together the
                    /// vertices. vertices and indices are cleared
                    /// from system memory when this is done.
                    void buildVBO(GLContext *glc);

                    /// Destroy the vertex buffer objects for
                    /// this. Make sure you do this before destroying
                    /// this object, otherwise it'll cause an assert.
                    void destroyVBO(GLContext *glc);

                private:
                };

                ~RenderInfo(void);

                /// Clean up any existing OpenGL buffers associated
                /// with this. (Do this before deleting the RenderInfo
                /// object.)
                void destroyStreams(GLContext *glc);

                /// Get a vertex stream for a given GL texture number.
                FontVertexStream *getVertexStreamForTexture(
                    unsigned int glTextureNumber);

                /// Add a vertex and automatically assign it to the
                /// correct vertex stream.
                unsigned short addVertex(
                    unsigned int glTextureNumber,
                    const FVec3 &position,
                    const FVec2 &uv,
                    const FVec3 &color,
                    float verticalPos,
                    float characterIndex);

                /// Add an index and automatically assign it to the
                /// correct vertex stream.
                void addIndex(
                    unsigned int glTextureNumber,
                    unsigned short index);

                /// Build all vertex buffer objects (and discard
                /// system memory versions).
                void buildVBOs(GLContext *glc);

                /// Collection of vertex streams. For ASCII-only
                /// stuff, it'll probably just be a single stream. Can
                /// be more if the font is divided into multiple pages
                /// (big character sets).
                std::vector<FontVertexStream*> streams;

            private:
            };

            // ----------------------------------------------------------------------
            // StringActionOutput
            // ----------------------------------------------------------------------

            /// Output from doStringAction. Includes render
            /// information and stuff.
            class StringActionOutput {
            public:

                StringActionOutput(void);
                ~StringActionOutput(void);

                /// If render info was generated, make sure this gets
                /// called before this is deleted. It needs data about
                /// GL to properly clean up buffers.
                void destroyRenderInfo(GLContext *glc);

                /// Font vertex buffers will go here if
                /// STRINGACTION_BUILDRENDERINFO is set in the
                /// action's stringActionBits.
                RenderInfo *renderInfo;

                /// Final rendered width. Does not depend on
                /// STRINGACTION_BUILDRENDERINFO to be calculated.
                float renderedWidth;

                /// Final rendered height. Does not depend on
                /// STRINGACTION_BUILDRENDERINFO to be calculated.
                float renderedHeight;

                // Cursor stuff...

                /// Cursor x position (left side of whatever character
                /// it's on).
                float cursorX;

                /// Cursor y position (bottom of whatever character
                /// it's on).
                float cursorY;

                /// Height of the cursor (probably just the height of
                /// a line, scaled correctly).
                float cursorHeight;

                /// The width of a cursor.
                float cursorWidth;

                /// Text location for the input mouse coordinates.
                unsigned int mouseCursorLocation;
            };

            // ----------------------------------------------------------------------
            // StringActionInput
            // ----------------------------------------------------------------------

            /// A collection of input parameters for
            /// doStringAction. Some fields used or not used depending
            /// on stringActionBits. All set to some default.
            class StringActionInput {
            public:

                StringActionInput(void);

                /// Only useful for STRINGACTION_WORDWRAP. Sets the
                /// point at which word wrapping will attempt to
                /// happen.
                float wordWrapWidth;

                /// Font glyphs and advance sizes will be scaled by
                /// this.
                float scale;

                /// This will go into the color channel of the vertex
                /// buffer by default. Things that can change the
                /// color include the VT100 color emulation.
                FVec3 defaultColor;

                /// How much to indent after a word-wrapped line.
                float wordWrapIndent;

                /// Position in the input. Will output into cursorX,
                /// cursorY in the output.
                unsigned int cursorPosition;

                float mouseCursorX;
                float mouseCursorY;

            };

        private:

            /// Determine if the next word should word wrap while in a
            /// string action.
            bool nextWordShouldWrap(
                const std::vector<unsigned int> &str,
                unsigned int currentIndex,
                float currentX,
                float width,
                float scale);

            /// How much the font will move vertially after each line.
            float verticalAdvance;

            /// A bunch of internal junk that I didn't want to expose
            /// in the header (like FreeType2 types and stuff).
            FontInternal *internal;

            /// The FontManager is allowed to set up stuff inside the
            /// Font when it is created.
            friend class FontManager;

            /// OpenGL function pointers.
            GLContext *glc;

        };

    }

}
