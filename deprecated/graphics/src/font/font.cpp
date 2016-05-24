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

#include <iomanip>
#include <cmath>
#include <iostream>
using namespace std;

#include <GL/gl.h>
#include <GL/glext.h>

// #include "config.h"
#if EXPOP_USE_FREETYPE2
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include <lilyengine/utils.h>
using namespace ExPop::Console;
using namespace ExPop::FileSystem;
using namespace ExPop;

#include <lilyengine/glcontext.h>
#include <lilyengine/font.h>
#include <lilyengine/fontmanager.h>
#include <lilyengine/fontrenderer.h>
#include <lilyengine/imagegl.h>

#define fontScale 16

#include <SDL/SDL.h>

namespace ExPop {

    namespace Gfx {

        // Built-in font.
        extern unsigned int builtInAsciiFont_width;
        extern unsigned int builtInAsciiFont_height;
        extern unsigned int builtInAsciiFont_length;
        extern char builtInAsciiFont_data[];

        // ----------------------------------------------------------------------
        // Internal classes within the font system.
        // ----------------------------------------------------------------------

        // Single glyph record.
        class FontCharacterRecord {
        public:

            FontCharacterRecord(void);

            // Bounds on the texture.
            float left, right, top, bottom;

            // Bottom-line of the text (things like 'g' go below it).
            float baseline;

            // Width of the character on screen.
            float width;
            float height;

            // How much to move forward after this. (On screen.)
            float advance;

            bool isValid;

        private:
        };

        // Single page of 256 glyphs.
        class FontPage {
        public:

            FontPage(void);

            ~FontPage(void);

            void destroyImageData(GLContext *glc);

            FontCharacterRecord characterRecords[256];
            Image *img;
            unsigned int glTextureNum;

        private:
        };

        // Internal data inside of a Font, so we don't have to expose
        // things like FreeType to the rest of the system.
        class FontInternal {
        public:

            Font::FontType type;
            std::map<unsigned int, FontPage*> pages;

            // Important stuff for FreeType fonts.
          #if EXPOP_USE_FREETYPE2
            FT_Face face;
          #endif
            char *fontFileBuffer;

            // Important stuff for bitmap fonts.
            Image *img;

        private:
        };

        // ----------------------------------------------------------------------
        // FontManager stuff.
        // ----------------------------------------------------------------------

        class FontManagerInternal {
        public:

          #if EXPOP_USE_FREETYPE2

            FT_Library ft2lib;

          #endif

        private:
        };

        FontManager::FontManager(GLContext *glc) {

            this->glc = glc;

            // This will get initialized as needed.
            builtInFontRenderer = NULL;

            // Init the internal data and the library.
            internal = new FontManagerInternal();

          #if EXPOP_USE_FREETYPE2
            FT_Init_FreeType(&(internal->ft2lib));
          #endif

            // Initialize the built-in font to use as a fallback.
            Image *builtInFontImg = load1BitImageFromBitmap(
                builtInAsciiFont_data, builtInAsciiFont_length,
                builtInAsciiFont_width, builtInAsciiFont_height);
            Font *builtInFont = createImageFont("builtIn", builtInFontImg);
            setFont("builtIn", builtInFont);
        }

        FontManager::~FontManager(void) {

            delete builtInFontRenderer;

            // Clean up all the fonts.
            for(std::map<std::string, Font*>::iterator i = loadedFonts.begin(); i != loadedFonts.end(); i++) {
                delete (*i).second;
            }

          #if EXPOP_USE_FREETYPE2

            // Shut down the library.
            FT_Done_FreeType(internal->ft2lib);

          #endif

            delete internal;
        }

      #if EXPOP_USE_FREETYPE2

        Font *FontManager::createFont(
            const std::string &name,
            char *buffer,
            int bufferLength) {

            out("info") << "Loading new font: " << name << endl;

            Font *newFont = new Font(glc);
            newFont->internal = new FontInternal;
            newFont->internal->fontFileBuffer = buffer;
            newFont->internal->type = Font::FONTTYPE_TRUETYPE;

            newFont->internal->img = NULL;

            if(FT_New_Memory_Face(
                   internal->ft2lib,
                   (const FT_Byte*)newFont->internal->fontFileBuffer,
                   bufferLength,
                   0, &newFont->internal->face)) {

                delete[] newFont->internal->fontFileBuffer;
                delete newFont;
                out("error") << "Couldn't load a font face for " << name << "." << endl;
                return NULL;
            }

            // Set the size.
            if(FT_Set_Char_Size(
                   newFont->internal->face,
                   0,              // 1/64th points
                   fontScale * 64, // 1/64th points
                   72, 72)) {

                // "Device" resolution in dpi. (?) (1/72 inch = 1 pt, so
                // 72 for this value = 1 pixel per point?)

                delete[] newFont->internal->fontFileBuffer;
                delete newFont;
                out("error") << "Couldn't set font size for " << name << "." << endl;
                return NULL;
            }

            // This is how far to advance down every time we word wrap.
            // FIXME: Figure out the right approach here.
            //newFont->verticalAdvance = (newFont->internal->face->height * fontScale) >> 10;
            newFont->verticalAdvance = fontScale;

            return newFont;
        }

      #else

        Font *FontManager::createFont(
            const std::string &name,
            char *buffer,
            int bufferLength) {

            out("error") << "Tried to load a TrueType font without support compiled in." << endl;

            return NULL;
        }

      #endif

        Font *FontManager::createImageFont(
            const std::string &name,
            Image *img) {

            assert(img);

            // We don't support non-2^n sized images for this.
            if(!img->isPow2Size()) {
                out("error") << "Tried to use an image for a font that was not a power of two size: " << name << endl;
                return NULL;
            }

            Font *newFont = new Font(glc);
            newFont->internal = new FontInternal;
            newFont->internal->img = img;
            newFont->internal->type = Font::FONTTYPE_BITMAP;

            newFont->internal->fontFileBuffer = NULL;

            // Font advances by one full vertical row.
            newFont->verticalAdvance = float(img->getHeight()) / 16.0;

            return newFont;
        }

        Font *FontManager::getFont(const std::string &name) {

            if(loadedFonts.count(name)) {

                // Font already exists.
                return loadedFonts[name];
            }

            // Doesn't exist. Load it and create it.
            int bufferLength = 0;

            // This buffer will either end up owned by the Font system or
            // deleted by the end of the function.
            char *buffer = loadFile(name, &bufferLength);
            if(!buffer) {
                return NULL;
            }

            Font *newFont = NULL;

            if(strEndsWith(".ttf", name)) {

                // Create normal TrueType font.
                newFont = createFont(name, buffer, bufferLength);

            } else if(strEndsWith(".tga", name) || strEndsWith(".png", name)) {

                // Create a bitmap font.
                Image *img = NULL;
                if(strEndsWith(".tga", name)) {
                    img = loadTGA(buffer, bufferLength);
                } else {
                  #if EXPOP_USE_PNG
                    img = loadPNG(buffer, bufferLength);
                  #endif
                }

                // This was used to build the image, but is no longer
                // needed.
                if(buffer) delete[] buffer;

                if(img) {
                    newFont = createImageFont(name, img);
                } else {
                    out("error") << "Could not read image for font: " << name << endl;
                }

            }

            // Register the new font if we succeeded.
            if(newFont) {
                loadedFonts[name] = newFont;
            } else {
                out("error") << "Something went horribly wrong loading font: " << name << endl;
            }

            return newFont;
        }

        void FontManager::setFont(const std::string &name, Font *font) {

            if(loadedFonts.count(name)) {

                assert(loadedFonts[name] != font);

                delete loadedFonts[name];
                loadedFonts.erase(name);
            }

            loadedFonts[name] = font;

        }

        void FontManager::putTextOnScreen(
            const std::string &str,
            const std::string &fontName,
            float x, float y,
            float wordWrapWidth,
            const FVec4 &color) {

            Font *font = getFont(fontName);

            assert(font); // TODO: Make this an exception or
                          // something?

            // Construct a new FontRendererBasic if necessary.
            if(!builtInFontRenderer) {
                builtInFontRenderer = new FontRendererBasic(glc);
            }

            // Convert to UTF-32
            vector<unsigned int> utf32Str;
            strUTF8ToUTF32(str, utf32Str);

            unsigned int bits = Font::STRINGACTION_BUILDRENDERINFO;
            Font::StringActionInput input;

            if(wordWrapWidth >= 0) {
                bits |= Font::STRINGACTION_WORDWRAP;
                input.wordWrapWidth = wordWrapWidth;
            }

            // TODO: Add word wrap?
            Font::StringActionOutput *sto = font->doStringAction(
                utf32Str, (Font::StringActionBits)bits, &input);

            builtInFontRenderer->render(sto, x, y, color);

            // Clean up.
            sto->destroyRenderInfo(glc);
            delete sto;
        }


        // ----------------------------------------------------------------------
        // Local utility functions
        // ----------------------------------------------------------------------

      #if EXPOP_USE_FREETYPE2

        // Clear an image to opaque black.
        static void clearImage(Image *img) {
            for(int x = 0; x < img->getWidth(); x++) {
                for(int y = 0; y < img->getHeight(); y++) {
                    Pixel *p = img->getPixel(x, y);
                    p->value = 0;
                    p->rgba.a = 0xff;
                }
            }
        }

        // // Save a debug image.
        // static void writeImage(Image *img, unsigned int pageNum) {

        //     ostringstream str;
        //     str << "out_" << setfill('0') << hex << pageNum << ".tga";

        //     int outTgaLen = 0;
        //     unsigned char *outTgaData = img->saveTGA(&outTgaLen);
        //     FileSystem::saveFile(str.str(), (char*)outTgaData, outTgaLen);
        //     delete[] outTgaData;
        // }

        // Render an entire font page, and setup UV info, advance info, and
        // baseline. This is only for TrueType fonts.
        static void renderFontPage(FT_Face face, unsigned int pageNum, FontPage *page, GLContext *glc) {

            unsigned int startSizeX = 256;
            unsigned int startSizeY = 256;

            Image *img = new Image(startSizeX, startSizeY);
            if(page->img) delete page->img;
            page->img = img;
            clearImage(img);

            if(page->glTextureNum) {
                glc->glDeleteTextures(1, &page->glTextureNum);
                page->glTextureNum = 0;
            }

            unsigned int glyphIndex = 0;
            unsigned int numGlyphsDumped = 0;
            unsigned int maxWidth = 0;
            unsigned int maxHeight = 0;

            unsigned int tallestThingThisRow = 0;
            unsigned int padding = 2;
            unsigned int currentXPos = padding;
            unsigned int currentYPos = padding;

            for(unsigned int charCode = pageNum << 8;
                charCode < (pageNum+1) << 8;
                charCode++) {

                unsigned int recordNum = charCode & 0xFF;

                glyphIndex = FT_Get_Char_Index(face, charCode);

                if(!glyphIndex) {
                    // Mark invalid.
                    page->characterRecords[recordNum].isValid = false;
                    continue;
                }

                // Assume this is valid until we find out it's not.
                page->characterRecords[recordNum].isValid = true;

                // Load a glyph.
                if(FT_Load_Glyph(
                       face,
                       glyphIndex,
                       FT_LOAD_DEFAULT)) {

                    // Skip this one.

                    // Mark it as invalid.
                    page->characterRecords[recordNum].isValid = false;

                    continue;
                }

                // Might need to convert it.
                if(face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {

                    if(FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
                        out("error") << "Could not render glyph" << endl;

                        // Mark this as invalid.
                        page->characterRecords[recordNum].isValid = false;

                        continue;
                    }
                }

                // Wrap to the next line if we have to.
                if(currentXPos + (face->glyph->advance.x >> 6) > (unsigned int)img->getWidth()) {

                    currentXPos = padding;
                    currentYPos += tallestThingThisRow;
                    currentYPos += padding;
                }

                while(currentYPos + face->glyph->bitmap.rows > (unsigned int)img->getHeight()) {

                    // We ran out of room in the image. Add more rows.
                    Image *newImage = new Image(img->getWidth(), img->getHeight() * 2);
                    clearImage(newImage);
                    memcpy(newImage->getPixel(0, 0), img->getPixel(0, 0), img->getWidth() * img->getHeight() * sizeof(Pixel));
                    delete img;
                    img = newImage;
                    page->img = img;
                    out("error") << "Had to expand the font image. This is slow. You should probably increase the size of the default image!" << endl;
                    out("error") << "Height is now: " << img->getHeight() << endl;
                }

                // And try to output it.
                maxWidth = MAX(maxWidth, (unsigned int)face->glyph->bitmap.pitch);
                maxHeight = MAX(maxHeight, (unsigned int)face->glyph->bitmap.rows);

                // Get rendered bitmap bounds.
                int maxx = face->glyph->bitmap.pitch;
                int maxy = face->glyph->bitmap.rows;

                page->characterRecords[recordNum].advance = (face->glyph->advance.x >> 6);

                page->characterRecords[recordNum].baseline = face->glyph->bitmap_top;

                for(int y = 0; y < maxy ; y++) {
                    for(int x = 0; x < maxx; x++) {

                        int imgX = currentXPos + x;
                        int imgY = currentYPos + y;

                        int val = 0;

                        Pixel *p = img->getPixel(imgX, imgY);

                        if(p) {

                            // I was using some of this stuff to debug
                            // character boundaries. So excuse the ugly
                            // block of commented out code.

                            // if(y == face->glyph->bitmap_top) {
                            //     p->rgba.r = 0;
                            // }

                            // if(x == 0 || y == 0 || x == maxx - 1 || y == maxy - 1) {
                            //     p->rgba.b = 0;
                            // }

                            // if(x == 0 || y == 0 || x == face->glyph->bitmap.width - 1 || y == face->glyph->bitmap.rows - 1) {
                            //     p->rgba.b = 0;
                            //     p->rgba.g = 0;
                            // }

                            if(x < face->glyph->bitmap.pitch && y < face->glyph->bitmap.rows) {
                                val = face->glyph->bitmap.buffer[x + y * face->glyph->bitmap.pitch];
                            }

                            // TODO: We might want to switch our fonts
                            // from black and white to represent alpha to
                            // actually just using alpha. Unless we can
                            // get greyscale stuff set up with the Image
                            // stuff.

                            p->rgba.a = 0xFF;
                            p->rgba.r = val;
                            p->rgba.g = val;
                            p->rgba.b = val;

                        }
                    }
                }

                // Keep track of the tallest thing in the row.
                tallestThingThisRow = MAX(((unsigned int)face->glyph->advance.y >> 6), tallestThingThisRow);
                tallestThingThisRow = MAX((unsigned int)face->glyph->bitmap.rows, tallestThingThisRow);

                page->characterRecords[recordNum].left   = currentXPos;
                page->characterRecords[recordNum].right  = currentXPos + maxx;

                // TODO: Maybe swap these?
                page->characterRecords[recordNum].top    = currentYPos;
                page->characterRecords[recordNum].bottom = currentYPos + maxy;

                page->characterRecords[recordNum].width  = face->glyph->bitmap.pitch;
                page->characterRecords[recordNum].height = face->glyph->bitmap.rows;

                // currentXPos += (face->glyph->advance.x >> 6);
                currentXPos += face->glyph->bitmap.pitch;
                currentXPos += padding;

                numGlyphsDumped++;
            }

            // Go through and correct all the UVs to 0-1 range. (Now that we
            // know the final size of the image.)
            for(unsigned int i = 0; i < 256; i++) {
                page->characterRecords[i].left   /= float(img->getWidth());
                page->characterRecords[i].right  /= float(img->getWidth());
                page->characterRecords[i].top    /= float(img->getHeight());
                page->characterRecords[i].bottom /= float(img->getHeight());
            }

            EXPOP_ASSERT_GL();

            // Create the GL texture.
            ImageTexture imgTex;
            createGLTextureFromImage(img, &imgTex, IMAGE_FORMAT_GREY_8);
            page->glTextureNum = imgTex.glTexNum;

            // Unbind it so someone doesn't screw it up later.
            // FIXME: Possibly remove this. Redundant?
            glc->glBindTexture(GL_TEXTURE_2D, 0);

            // FIXME: Rebind whatever texture was already bound.

            // FIXME: Add gl debug stuff around this.

            // FIXME: Incorporate state wrapper thingy.

            // TODO: Add an option to skip this cleanup and preserve
            // the Image until later on, so we can use it for things
            // like bitmap font rendering.

            // Clean up some stuff.
            assert(page->img == img);

            cout << img->getWidth() << endl;
            cout << img->getHeight() << endl;


            delete img;
            img = NULL;
            page->img = NULL;

            EXPOP_ASSERT_GL();
        }

      #endif

        static void makePageForBitmap(Image *img, unsigned int pageNum, FontPage *page) {

            // We only do ASCII fonts at the moment, so refuse to do
            // anything outside of that range.
            if(pageNum != 0) return;

            // Create the texture straight from the image. This one is
            // okay to use color on.
            ImageTexture imgTex;
            createGLTextureFromImage(img, &imgTex, IMAGE_FORMAT_RGBA_32);
            page->glTextureNum = imgTex.glTexNum;

            // TODO: Add an option to store a COPY OF THE IMAGE on the
            // page structure so we can do bitmap fonts without GL
            // stuff.
            page->img = NULL;

            // Now go through and set up the character records.
            for(unsigned int y = 0; y < 16; y++) {
                for(unsigned int x = 0; x < 16; x++) {
                    FontCharacterRecord *rec = &(page->characterRecords[x + y * 16]);
                    rec->left   = float(x)   / 16.0;
                    rec->right  = float(x+1) / 16.0;
                    rec->top    = float(y)   / 16.0;
                    rec->bottom = float(y+1) / 16.0;
                    rec->baseline = float(img->getHeight()) / 16.0;
                    rec->width  = float(img->getWidth()) / 16.0;
                    rec->height = float(img->getHeight()) / 16.0;
                    rec->advance = float(img->getWidth()) / 16.0;
                    rec->isValid = true;
                }
            }
        }

        // ----------------------------------------------------------------------
        // Internal classes implementation
        // ----------------------------------------------------------------------

        FontCharacterRecord::FontCharacterRecord(void) {
            isValid = false;
            left = right = top = bottom = baseline = 0;
        }

        // FontPage -------------------------------------------------------------

        FontPage::FontPage(void) {
            img = NULL;
            glTextureNum = 0;
        }

        FontPage::~FontPage(void) {

            // TODO: Possibly keep the rendered Image until here, then
            // delete it.
            assert(!glTextureNum);
            assert(!img);
        }

        void FontPage::destroyImageData(GLContext *glc) {

            if(glTextureNum) {
                glc->glDeleteTextures(1, (GLuint*)&glTextureNum);
                glTextureNum = 0;
            }
            if(img) {
                delete img;
                img = NULL;
            }

        }

        // RenderInfo -----------------------------------------------------------

        Font::RenderInfo::FontVertex::FontVertex(
            const FVec3 &position,
            const FVec2 &uv,
            const FVec3 &color,
            float verticalPos,
            float characterIndex) {

            this->position = position;
            this->uv = uv;
            this->color = color;
            this->verticalPos = verticalPos;
            this->characterIndex = characterIndex;

        }

        Font::RenderInfo::~RenderInfo(void) {
            assert(!streams.size());
        }

        void Font::RenderInfo::destroyStreams(GLContext *glc) {
            for(unsigned int i = 0; i < streams.size(); i++) {
                streams[i]->destroyVBO(glc);
                delete streams[i];
            }
            streams.clear();
        }

        Font::RenderInfo::FontVertexStream *Font::RenderInfo::getVertexStreamForTexture(
            unsigned int glTextureNumber) {

            FontVertexStream *stream = NULL;

            // Find the right vertex stream.
            for(unsigned int i = 0; i < streams.size(); i++) {
                if(streams[i]->glTextureNumber == glTextureNumber) {
                    stream = streams[i];
                }
            }

            // If we haven't found it, we need to add one for this
            // texture number.
            if(!stream) {
                stream = new FontVertexStream(glTextureNumber);
                streams.push_back(stream);
            }

            return stream;
        }

        unsigned short Font::RenderInfo::addVertex(
            unsigned int glTextureNumber,
            const FVec3 &position,
            const FVec2 &uv,
            const FVec3 &color,
            float verticalPos,
            float characterIndex) {

            FontVertexStream *stream =
                getVertexStreamForTexture(glTextureNumber);

            // Add the vertex.
            stream->vertices.push_back(
                FontVertex(
                    position,
                    uv,
                    color,
                    verticalPos,
                    characterIndex));

            return (unsigned short)(stream->vertices.size() - 1);

        }

        void Font::RenderInfo::addIndex(
            unsigned int glTextureNumber,
            unsigned short index) {

            FontVertexStream *stream =
                getVertexStreamForTexture(glTextureNumber);

            stream->indices.push_back(
                index);
        }

        void Font::RenderInfo::buildVBOs(GLContext *glc) {

            for(unsigned int i = 0; i < streams.size(); i++) {
                streams[i]->buildVBO(glc);
            }

        }

        // VertexStream ---------------------------------------------------------

        Font::RenderInfo::FontVertexStream::FontVertexStream(unsigned int glTextureNumber) {
            this->numCharacters = 0;
            this->glTextureNumber = glTextureNumber;
            vbo = 0;
            indexBuffer = 0;
            numIndices = 0;
        }

        Font::RenderInfo::FontVertexStream::~FontVertexStream(void) {
            assert(!vbo);
            assert(!indexBuffer);
        }

        void Font::RenderInfo::FontVertexStream::destroyVBO(GLContext *glc) {

            if(vbo) {
                glc->glDeleteBuffers(1, (GLuint*)&vbo);
                vbo = 0;
            }
            if(indexBuffer) {
                glc->glDeleteBuffers(1, (GLuint*)&indexBuffer);
                indexBuffer = 0;
            }

        }

        void Font::RenderInfo::FontVertexStream::buildVBO(GLContext *glc) {

            // Build all the vertices.
            assert(!vbo);
            glc->glGenBuffers(1, (GLuint*)&vbo);
            glc->glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glc->glBufferData(GL_ARRAY_BUFFER,
                              sizeof(FontVertex) * vertices.size(),
                              &(vertices[0]), GL_STATIC_DRAW);

            // Build all the triangles using those vertices.
            assert(!indexBuffer);
            glc->glGenBuffers(1, (GLuint*)&indexBuffer);
            glc->glBindBuffer(GL_ARRAY_BUFFER, indexBuffer);
            glc->glBufferData(GL_ARRAY_BUFFER,
                              sizeof(unsigned short) * indices.size(),
                              &(indices[0]), GL_STATIC_DRAW);

            glc->glBindBuffer(GL_ARRAY_BUFFER, 0);

            numIndices = indices.size();

            // We don't need these anymore. Save some memory and
            // nuke them.
            vertices.clear();
            indices.clear();
        }

        // StringActionOutput ---------------------------------------------------

        Font::StringActionOutput::StringActionOutput(void) {
            renderInfo = NULL;
            renderedWidth = 0;
            renderedHeight = 0;
            cursorX = 0;
            cursorY = 0;
            cursorWidth = 0;
            cursorHeight = 0;
            mouseCursorLocation = 0;
        }

        Font::StringActionOutput::~StringActionOutput(void) {
            assert(!renderInfo);
        }

        void Font::StringActionOutput::destroyRenderInfo(GLContext *glc) {

            if(renderInfo) {
                renderInfo->destroyStreams(glc);
                delete renderInfo;
                renderInfo = NULL;
            }

        }

        // StringActionInput ----------------------------------------------------

        Font::StringActionInput::StringActionInput(void) {
            // A bunch of nice defaults.
            wordWrapWidth = 512;
            scale = 1;
            defaultColor = FVec3(0.5, 0.5, 0.5);
            wordWrapIndent = 0;
            cursorPosition = 0;
            mouseCursorX = 0.0f;
            mouseCursorY = 0.0f;
            highlightMin = ~(unsigned int)0;
            highlightMax = ~(unsigned int)0;
        }

        // ----------------------------------------------------------------------
        // Main Font class implementation
        // ----------------------------------------------------------------------

        Font::Font(GLContext *glc) {

            // This is mostly initialized by FontManager.
            this->internal = NULL;
            this->glc = glc;
        }

        Font::~Font(void) {

            for(std::map<unsigned int, FontPage*>::iterator i = internal->pages.begin(); i != internal->pages.end(); i++) {
                if((*i).second) {
                    (*i).second->destroyImageData(glc);
                    delete (*i).second;
                }
            }

          #if EXPOP_USE_FREETYPE2
            if(internal->type == Font::FONTTYPE_TRUETYPE) {
                FT_Done_Face(internal->face);
                delete[] internal->fontFileBuffer;
            }
          #endif

            if(internal->type == Font::FONTTYPE_BITMAP) {
                delete internal->img;
            }

            delete internal;
        }

        FontPage *Font::getPage(unsigned int pageNum) {

            FontPage *currentPage = internal->pages[pageNum];

            if(!currentPage) {

                // Page doesn't exist yet. Make it!
                currentPage = new FontPage;
                internal->pages[pageNum] = currentPage;

                if(internal->type == Font::FONTTYPE_TRUETYPE) {

                  #if EXPOP_USE_FREETYPE2

                    // Make the image and turn it into a texture.
                    renderFontPage(internal->face, pageNum, currentPage, glc);

                  #endif

                } else if(internal->type == Font::FONTTYPE_BITMAP) {

                    // Just do the basic bitmap font setup.
                    makePageForBitmap(internal->img, pageNum, currentPage);

                }
            }

            return currentPage;
        }

        bool Font::nextWordShouldWrap(
            const std::vector<unsigned int> &str,
            unsigned int currentIndex,
            float currentX,
            float width,
            float scale) {

            // FIXME: This interferes with the VT100 hack and probably
            // sees characters that are within the VT100 color commands.

            for(unsigned int i = currentIndex; i < str.size(); i++) {

                unsigned int charNum = str[i];

                // Reached a space. We're done here.
                if(charNum == 0x20 || charNum == '\n' || charNum == '\t') return false;

                unsigned int currentPageNum = (charNum >> 8);
                FontPage *currentPage = getPage(currentPageNum);
                FontCharacterRecord *record = &(currentPage->characterRecords[charNum & 0xFF]);

                if(record->isValid) {

                    // If we go over the edge, we're wrapping.
                    currentX += record->advance * scale;
                }

                if(currentX > width) return true;
            }

            return false;

        }

        Font::StringActionOutput *Font::doStringAction(
            const std::vector<unsigned int> &str,
            Font::StringActionBits stringActionBits,
            Font::StringActionInput *inputVals) {

            // FIXME: This function needs some pretty massive
            // refactoring. It's a giant mess.

            // If we don't have inputs, then just use some defaults.
            bool myInputVals = false;
            if(!inputVals) {
                inputVals = new Font::StringActionInput();
                myInputVals = true;
            }

            // Mainly for VT100 color stuff...
            FVec3 vt100FgColor = inputVals->defaultColor;
            bool vt100Bold = false;

            // Starting value is known invalid. We just want the
            // page-switch logic to happen the first time.
            unsigned int currentPageNum = 0xffffffff;

            Font::StringActionOutput *output = new Font::StringActionOutput();

            if(stringActionBits & Font::STRINGACTION_BUILDRENDERINFO) {
                output->renderInfo = new Font::RenderInfo();
            }

            // FontPage *currentPage = internal->pages[currentPageNum];
            FontPage *currentPage = NULL;

            // Our starting point is considered the upper-left of the
            // first character.
            float x = 0;
            float y = verticalAdvance * inputVals->scale;

            // Word wrapping stuff.
            float wrapWidth = inputVals->wordWrapWidth;

            bool lastRecordWasValid = true;
            bool cursorPositionSet = false;
            bool mousePositionSet = false;

            float maxY = 0.0f;

            // FIXME: The #defines I stuck in here to reduce redundant
            // code have made Emacs very unhappy.

            // Highlight-related #defines

            // FIXME: The 0.25f is totally arbitrary because we
            // don't know how far below the baseline we need to
            // go!
          #define ADD_HIGHLIGHT_START()                                 \
            output->highlightQuads.push_back(x);                        \
            output->highlightQuads.push_back(y + verticalAdvance * inputVals->scale * 0.25f);

          #define ADD_HIGHLIGHT_END()                                   \
            output->highlightQuads.push_back(x+1);                      \
            output->highlightQuads.push_back(y - verticalAdvance * inputVals->scale * 1.0f);

          #define IN_HIGHLIGHT()                \
            (i >= inputVals->highlightMin &&    \
             i < inputVals->highlightMax)

            // Cursor-related #defines
          #define SET_CURSOR_POSITION() {                               \
            cursorPositionSet = true;                                   \
            output->cursorX = x;                                        \
            output->cursorY = y;                                        \
            output->cursorHeight = verticalAdvance * inputVals->scale;  \
            output->cursorWidth = 1; }

            for(unsigned int i = 0; i < str.size(); i++) {

                unsigned int charNum = str[i];

                // FIXME: Duplicated code.
                // Handle cursor setup.
                if(!cursorPositionSet && inputVals->cursorPosition <= i) {
                    SET_CURSOR_POSITION();
                }

                // FIXME: Duplicated code.
                // Mouse cursor location. Hovering on some character.
                if(!mousePositionSet &&
                   inputVals->mouseCursorX &&
                   inputVals->mouseCursorY) {
                    if(inputVals->mouseCursorX < x &&
                       inputVals->mouseCursorY < y) {
                        mousePositionSet = true;
                        if(i == 0) {
                            output->mouseCursorLocation = 0;
                        } else {
                            output->mouseCursorLocation = i - 1;
                        }
                    }
                }

                // Can't use an 'or' operator here. We might have to
                // start and end the quads on the same spot.
                if(i == inputVals->highlightMin) {
                    ADD_HIGHLIGHT_START();
                }
                if(i == inputVals->highlightMax) {
                    ADD_HIGHLIGHT_END();
                }

                // First we try to handle special cases, then we default
                // to doing stuff normally.
                if(charNum == '\n') {

                    // FIXME: Duplicated code.
                    // Mouse cursor location. After the end of a newline.
                    if(inputVals->mouseCursorY < y &&
                       !mousePositionSet) {
                        mousePositionSet = true;
                        output->mouseCursorLocation = i;
                    }

                    if(IN_HIGHLIGHT()) {
                        ADD_HIGHLIGHT_END();
                    }

                    // Advance line.
                    y += verticalAdvance * inputVals->scale;
                    x = 0;


                    if(IN_HIGHLIGHT()) {
                        ADD_HIGHLIGHT_START();
                    }

                } else if(charNum == 0x1B && (stringActionBits & STRINGACTION_VT100COLOR)) {

                    // FIXME: I don't know why I need this crap
                    // anymore.

                    // Handle VT100 color emulation (so we can easily hook
                    // it up to our existing Console system, mainly.)

                    // We do some nasty string manipulation here, so try
                    // to only do it once per console line.

                    // Skip the escape character and the opening bracket.
                    i += 2;
                    unsigned int attribsStart = i;

                    // Find the 'm' that terminates one of these
                    // statements.
                    while(i < str.size()) {
                        if(str[i] == 'm') {
                            break;
                        }
                        i++;
                    }

                    if(i < str.size() && attribsStart < str.size()) {

                        // Convert back to an ASCII string (assuming the
                        // parts we're dealing with are ASCII). Then
                        // tokenize it based on ';'.

                        ostringstream attribsStr;
                        for(unsigned int k = attribsStart; k < i; k++) {
                            attribsStr << ((char)str[k]);
                        }

                        // Split the attributes list by ';'.

                        vector<std::string> attribsSplit;
                        stringTokenize(attribsStr.str(), ";", attribsSplit, false);

                        // We have our list of attributes now as
                        // strings. Iterate through them and apply them.

                        for(unsigned int attribNum = 0; attribNum < attribsSplit.size(); attribNum++) {
                            int realAttribVal = atoi(attribsSplit[attribNum].c_str());

                            if(realAttribVal == 0) {

                                // Reset everything.
                                vt100FgColor = inputVals->defaultColor;
                                vt100Bold = false;

                            } else if(realAttribVal >= 30 && realAttribVal <= 37) {

                                // FG color modification. The color list
                                // is arranged in such a way that each bit
                                // in the color number corresponds to red,
                                // green, and blue. (Hurray.)
                                int colorVal = realAttribVal - 30;
                                vt100FgColor[0] = 0.5f * float(!!(colorVal & 1));
                                vt100FgColor[1] = 0.5f * float(!!(colorVal & 2));
                                vt100FgColor[2] = 0.5f * float(!!(colorVal & 4));

                            } else if(realAttribVal == 1) {

                                vt100Bold = true;

                            } else if(realAttribVal == 2) {

                                vt100Bold = false;

                            }

                        }

                    }

                } else {

                    // See if we have to switch pages.
                    if((charNum >> 8) != currentPageNum) {

                        currentPageNum = (charNum >> 8);
                        currentPage = getPage(currentPageNum);
                    }

                    unsigned int recordNum = charNum & 0xFF;
                    FontCharacterRecord *record = &(currentPage->characterRecords[recordNum]);

                    if(record->isValid) {

                        if(stringActionBits & STRINGACTION_WORDWRAP) {

                            // Handle word wrapping. We allow line
                            // breaking on invalid characters and
                            // whitespace.
                            if(i > 0 && ((str[i-1] == 0x20) || !lastRecordWasValid)) {
                                if(nextWordShouldWrap(str, i, x, wrapWidth, inputVals->scale)) {

                                    // Mouse cursor location. Handle
                                    // cursor at the end of a line
                                    // when that line break is due to
                                    // word wrapping.
                                    if(inputVals->mouseCursorY < y &&
                                       !mousePositionSet) {
                                        mousePositionSet = true;
                                        if(i) {
                                            output->mouseCursorLocation = i-1;
                                        } else {
                                            output->mouseCursorLocation = 0;
                                        }
                                    }

                                    if(IN_HIGHLIGHT()) {
                                        ADD_HIGHLIGHT_END();
                                    }

                                    // Advance a line.
                                    x = inputVals->wordWrapIndent;
                                    y += verticalAdvance * inputVals->scale;

                                    if(IN_HIGHLIGHT()) {
                                        ADD_HIGHLIGHT_START();
                                    }
                                }
                            }

                            // FIXME: Duplicated code.
                            // Handle cursor setup.
                            if(inputVals->cursorPosition == i) {
                                SET_CURSOR_POSITION();
                            }

                        }

                        // renderInfo will only be there if this is a
                        // STRINGACTION_BUILDRENDERINFO, so check for that
                        // instead. (In case something else implies that
                        // flag or something?)
                        if(output->renderInfo) {

                            // Build a vertex buffer..

                            float yOffset = -record->baseline * inputVals->scale;
                            float xOffset = inputVals->scale * (record->advance - record->width) / 2.0;

                            // if(y - yOffset > maxY) maxY = y - yOffset;

                            float colorScale =
                                (vt100Bold || !(stringActionBits & Font::STRINGACTION_VT100COLOR)) ? 2.0 : 1.0;

                            // Top
                            unsigned int index0 = output->renderInfo->addVertex(
                                currentPage->glTextureNum,
                                FVec3(x + xOffset, y + yOffset, 0),
                                FVec2(record->left, record->top),
                                vt100FgColor * colorScale,
                                yOffset, i);

                            unsigned int index1 = output->renderInfo->addVertex(
                                currentPage->glTextureNum,
                                FVec3(x + xOffset + record->width * inputVals->scale, y + yOffset, 0),
                                FVec2(record->right, record->top),
                                vt100FgColor * colorScale,
                                yOffset, i);

                            // Bottom
                            unsigned int index2 = output->renderInfo->addVertex(
                                currentPage->glTextureNum,
                                FVec3(x + xOffset + record->width * inputVals->scale, y + yOffset + record->height * inputVals->scale, 0),
                                FVec2(record->right, record->bottom),
                                vt100FgColor * colorScale,
                                yOffset + record->height * inputVals->scale, i);

                            unsigned int index3 = output->renderInfo->addVertex(
                                currentPage->glTextureNum,
                                FVec3(x + xOffset, y + yOffset + record->height * inputVals->scale, 0),
                                FVec2(record->left, record->bottom),
                                vt100FgColor * colorScale,
                                yOffset + record->height * inputVals->scale, i);

                            // First triangle
                            output->renderInfo->addIndex(
                                currentPage->glTextureNum,
                                index0);

                            output->renderInfo->addIndex(
                                currentPage->glTextureNum,
                                index1);

                            output->renderInfo->addIndex(
                                currentPage->glTextureNum,
                                index2);

                            // Second triangle.
                            output->renderInfo->addIndex(
                                currentPage->glTextureNum,
                                index2);

                            output->renderInfo->addIndex(
                                currentPage->glTextureNum,
                                index3);

                            output->renderInfo->addIndex(
                                currentPage->glTextureNum,
                                index0);


                            output->renderedHeight = MAX(
                                y + yOffset + record->height * inputVals->scale,
                                output->renderedHeight);
                        }

                        // Move the cursor along.
                        x += record->advance * inputVals->scale;

                        output->renderedWidth = MAX(x, output->renderedWidth);

                        lastRecordWasValid = true;

                    } else {

                        lastRecordWasValid = false;
                    }
                }
            }

            // FIXME: Duplicated code.
            // If we still don't have a cursor set by now, it's probably
            // off the end (which is okay.)
            if(!cursorPositionSet) {
                SET_CURSOR_POSITION();
            }

            // FIXME: Duplicated code.
            // If we still haven't set the mouse cursor location, set
            // it now.
            if(!mousePositionSet) {
                mousePositionSet = true;
                output->mouseCursorLocation = str.size();
            }

            if(inputVals->highlightMin < str.size() &&
               inputVals->highlightMax >= str.size()) {
                ADD_HIGHLIGHT_END();
            }

            // Do a final check of the output bounds.
            output->renderedWidth  = MAX(x, output->renderedWidth);
            output->renderedHeight =
                MAX(MAX(y, output->renderedHeight), maxY);

            // Clean up default inputs if we created it.
            if(myInputVals) {
                delete inputVals;
            }

            EXPOP_ASSERT_GL();

            // Create VBOs for render info.
            if(output->renderInfo && !(stringActionBits & STRINGACTION_DONTBUILDVBOS)) {
                output->renderInfo->buildVBOs(glc);
            }


            EXPOP_ASSERT_GL();

            return output;
        }

        float Font::getVerticalAdvance(void) {
            return verticalAdvance;
        }

    }

}


