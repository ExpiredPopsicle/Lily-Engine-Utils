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

#include <vector>
#include <cstring>
using namespace std;

#include <GL/gl.h>
#include <GL/glext.h>

#include <lilyengine/glcontext.h>
#include <lilyengine/font.h>
#include <lilyengine/fontrenderer.h>

namespace ExPop {

    namespace Gfx {

        FontRendererBasic::FontRendererBasic(GLContext *glc) {

            this->glc = glc;

            vertShader = glc->glCreateShader(GL_VERTEX_SHADER);
            fragShader = glc->glCreateShader(GL_FRAGMENT_SHADER);

            const char *vertShaderSrc =
                "attribute vec3 inPosition;\n"
                "attribute vec4 inColor;\n"
                "attribute vec2 inUv;\n"
                "attribute float inVPos;\n"
                "uniform mat4 matrix;\n"
                "varying vec2 outUv;\n"
                "varying vec4 outColor;\n"
                "varying float outVPos;\n"
                "void main() {\n"
                "  gl_Position = matrix * vec4(inPosition.xyz, 1);\n"
                "  outUv = inUv;\n"
                "  outColor = inColor;\n"
                "  outVPos  = inVPos;\n"
                "}\n";

            const char *fragShaderSrc =
                "varying vec2 outUv;\n"
                "varying vec4 outColor;\n"
                "varying float outVPos;\n"
                "uniform sampler2D sampler;\n"
                "uniform vec4 colorScale;\n"
                "void main() {\n"
                // "  float fadeAmount = clamp(1.0-outVPos/16.0 - 0.5, 0.5, 1);\n"
                // "  vec4 color = fadeAmount * outColor * vec4(colorScale.xyz, 1);\n"
                "  vec4 color = outColor * vec4(colorScale.xyz, 1);\n"
                // "  vec4 color = outColor * vec4(1, 1, 1, 1);\n"
                "  float alpha = texture2D(sampler, outUv).x * colorScale.w;\n"
                "  gl_FragColor = vec4(color.xyz, alpha);\n"
                // "  gl_FragColor = vec4(1, 1, 1, 1) - gl_FragColor;\n"
                "}\n";

            int compileResult = 0;

            glc->glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
            glc->glCompileShader(vertShader);
            glc->glGetShaderiv(vertShader, GL_COMPILE_STATUS, (GLint*)&compileResult);
            assert(compileResult);

            glc->glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
            glc->glCompileShader(fragShader);
            glc->glGetShaderiv(fragShader, GL_COMPILE_STATUS, (GLint*)&compileResult);
            assert(compileResult);

            shaderProgram = glc->glCreateProgram();
            glc->glAttachShader(shaderProgram, vertShader);
            glc->glAttachShader(shaderProgram, fragShader);
            glc->glLinkProgram(shaderProgram);
            glc->glGetProgramiv(shaderProgram, GL_LINK_STATUS, (GLint*)&compileResult);
            assert(compileResult);

            // Now get the handles for all the inputs.
            matrixHandle   = glc->glGetUniformLocation(shaderProgram, "matrix");
            positionHandle = glc->glGetAttribLocation(shaderProgram, "inPosition");
            uvHandle       = glc->glGetAttribLocation(shaderProgram, "inUv");
            colorHandle    = glc->glGetAttribLocation(shaderProgram, "inColor");
            vposHandle     = glc->glGetAttribLocation(shaderProgram, "inVPos");
            samplerHandle  = glc->glGetAttribLocation(shaderProgram, "sampler");
            colorScaleHandle = glc->glGetUniformLocation(shaderProgram, "colorScale");

            EXPOP_ASSERT_GL();

        }

        FontRendererBasic::~FontRendererBasic(void) {
            glc->glDeleteProgram(shaderProgram);
            glc->glDeleteShader(vertShader);
            glc->glDeleteShader(fragShader);
        }

        void FontRendererBasic::render(
            Font::StringActionOutput *output,
            float x, float y,
            const FVec4 &colorScale,
            float scrWidth, float scrHeight) {

            if(!scrWidth || !scrHeight) {
                // Get the GL viewport size, because we want to display stuff
                // in pixel positions no matter what.
                float viewport[4];
                glGetFloatv(GL_VIEWPORT, viewport);
                scrWidth = viewport[2];
                scrHeight = viewport[3];
            }

            assert(output->renderInfo);

            glc->glEnable(GL_TEXTURE_2D);
            glc->glEnable(GL_BLEND);
            glc->glBlendEquation(GL_FUNC_ADD);
            glc->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            EXPOP_ASSERT_GL();

            glc->glUseProgram(shaderProgram);

            EXPOP_ASSERT_GL();

            if(positionHandle != -1) glc->glEnableVertexAttribArray(positionHandle);
            if(colorHandle != -1)    glc->glEnableVertexAttribArray(colorHandle);
            if(uvHandle != -1)       glc->glEnableVertexAttribArray(uvHandle);
            if(vposHandle != -1)     glc->glEnableVertexAttribArray(vposHandle);

            EXPOP_ASSERT_GL();

            // Put this thing in screen coordinates.
            FMatrix4x4 mat =
                makeTranslationMatrix(FVec3(-1.0, 1.0, 0.0)) *
                makeScaleMatrix(FVec3(2.0 / scrWidth, -2.0/scrHeight, 1.0)) *
                makeTranslationMatrix(FVec3(x, y, 0));

            EXPOP_ASSERT_GL();

            glc->glUniformMatrix4fv(matrixHandle,
                                    1, false, mat.data);

            glc->glUniform4f(colorScaleHandle,
                             colorScale.data[0], colorScale.data[1],
                             colorScale.data[2], colorScale.data[3]);

            EXPOP_ASSERT_GL();

            glc->glActiveTexture(GL_TEXTURE0);
            glc->glUniform1i(samplerHandle, 0);

            EXPOP_ASSERT_GL();

            for(unsigned int streamNum = 0; streamNum < output->renderInfo->streams.size(); streamNum++) {

                EXPOP_ASSERT_GL();

                glc->glBindTexture(GL_TEXTURE_2D, output->renderInfo->streams[streamNum]->glTextureNumber);

                EXPOP_ASSERT_GL();

                glc->glBindBuffer(GL_ARRAY_BUFFER, output->renderInfo->streams[streamNum]->vbo);
                glc->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, output->renderInfo->streams[streamNum]->indexBuffer);

                if(positionHandle != -1) glc->glVertexAttribPointer(
                    positionHandle, 3, GL_FLOAT, false,
                    sizeof(Font::RenderInfo::FontVertex),
                    OFFSET_OF(Font::RenderInfo::FontVertex, position));

                if(colorHandle != -1) glc->glVertexAttribPointer(
                    colorHandle, 3, GL_FLOAT, false,
                    sizeof(Font::RenderInfo::FontVertex),
                    OFFSET_OF(Font::RenderInfo::FontVertex, color));

                if(uvHandle != -1) glc->glVertexAttribPointer(
                    uvHandle, 2, GL_FLOAT, false,
                    sizeof(Font::RenderInfo::FontVertex),
                    OFFSET_OF(Font::RenderInfo::FontVertex, uv));

                if(vposHandle != -1) glc->glVertexAttribPointer(
                    vposHandle, 1, GL_FLOAT, false,
                    sizeof(Font::RenderInfo::FontVertex),
                    OFFSET_OF(Font::RenderInfo::FontVertex, verticalPos));

                EXPOP_ASSERT_GL();

                glc->glDrawElements(
                    GL_TRIANGLES, output->renderInfo->streams[streamNum]->numIndices,
                    GL_UNSIGNED_SHORT, NULL);

                EXPOP_ASSERT_GL();

            }

            // Clean up GL state.

            EXPOP_ASSERT_GL();

            if(positionHandle != -1) glc->glDisableVertexAttribArray(positionHandle);
            if(colorHandle != -1)    glc->glDisableVertexAttribArray(colorHandle);
            if(uvHandle != -1)       glc->glDisableVertexAttribArray(uvHandle);
            if(vposHandle != -1)     glc->glDisableVertexAttribArray(vposHandle);

            EXPOP_ASSERT_GL();

            glc->glBindBuffer(GL_ARRAY_BUFFER, 0);
            glc->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            glc->glBindTexture(GL_TEXTURE_2D, 0);

            glc->glUseProgram(0);

            glc->glDisable(GL_BLEND);
            glc->glBlendFunc(GL_ONE, GL_ZERO);

            EXPOP_ASSERT_GL();

        }

        void FontRendererBasic::renderTextDirect(
            Font *font,
            const std::string &text,
            float x, float y,
            const FVec4 &colorScale,
            bool wordWrap) {

            // Get the screen size.
            float viewport[4];
            glGetFloatv(GL_VIEWPORT, viewport);
            float scrWidth = viewport[2];

            // Convert to UTF-32
            vector<unsigned int> utf32Ver;
            strUTF8ToUTF32(text, utf32Ver);

            Font::StringActionInput sti;
            sti.wordWrapWidth = scrWidth;
            Font::StringActionOutput *entryRenderInfo =
                font->doStringAction(
                    utf32Ver, (Font::StringActionBits)(
                        Font::STRINGACTION_BUILDRENDERINFO |
                        (wordWrap ? Font::STRINGACTION_WORDWRAP : 0)
                        ), &sti);

            render(entryRenderInfo, x, y, colorScale);

            entryRenderInfo->destroyRenderInfo(glc);

            delete entryRenderInfo;

        }
    }
}
