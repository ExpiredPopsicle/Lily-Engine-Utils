#include <iostream>
#include <string>
using namespace std;

#include <GL/gl.h>
#include <GL/glext.h>

#include <lilyengine/glutils.h>
#include <lilyengine/glcontext.h>
#include <lilyengine/filesystem.h>
#include <lilyengine/preprocess.h>

namespace ExPop {

    namespace Gfx {

        GLuint linkShaders(
            GLContext *glc,
            GLuint vertexShader,
            GLuint fragmentShader,
            std::string &errorOut) {

            if(!vertexShader || !fragmentShader) {
                errorOut += "No vertex shader or no fragment shader passed into linkShaders().\n";
                return 0;
            }

            if(errorOut.size()) return 0;

            GLuint program = glc->glCreateProgram();

            glc->glAttachShader(program, vertexShader);
            glc->glAttachShader(program, fragmentShader);

            glc->glLinkProgram(program);

            GLint linkStatus = 0;
            glc->glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if(linkStatus == GL_FALSE) {

                // Pull out the error message.
                GLint logLength = 0;
                glc->glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
                char *shaderLog = new char[logLength + 1];
                glc->glGetProgramInfoLog(program, logLength, NULL, shaderLog);
                shaderLog[logLength] = 0;

                errorOut = "Program link error:\n" + string(shaderLog);
                delete[] shaderLog;

                // Clean up.
                glc->glDeleteProgram(program);
                program = 0;
            }

            return program;
        }

        GLuint compileShader(
            GLContext *glc,
            const std::string &shaderText,
            GLuint shaderType,
            std::string &errorOut) {

            if(errorOut.size()) return 0;

            GLuint shader = glc->glCreateShader(shaderType);

            const char *cShaderSourceStr = shaderText.c_str();
            glc->glShaderSource(shader, 1, &cShaderSourceStr, NULL);
            glc->glCompileShader(shader);

            // Get compile status.
            GLint compileResult = 0;
            glc->glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
            if(compileResult == GL_FALSE) {

                // Pull out the error message.
                GLint logLength = 0;
                glc->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
                char *shaderLog = new char[logLength + 1];
                glc->glGetShaderInfoLog(shader, logLength, NULL, shaderLog);
                shaderLog[logLength] = 0;

                errorOut = "Shader compile error:\n" + string(shaderLog) + "\nShader text:\n" + shaderText;
                delete[] shaderLog;

                // Clean up.
                glc->glDeleteShader(shader);
                shader = 0;
            }

            return shader;
        }

        GLuint loadShaderFile(
            GLContext *glc,
            const std::string &fileName,
            GLuint shaderType,
            std::string &errorOut) {

            std::string inFile = FileSystem::loadFileString(fileName);
            if(!inFile.size()) {
                errorOut = string("Could not load file or file empty: ") + fileName;
                return 0;
            }

            PreprocessorState ppstate;
            string preprocessedStr = preprocess(fileName, inFile, ppstate);

            if(ppstate.hadError) {
                errorOut = ppstate.errorText;
                return 0;
            }

            return compileShader(
                glc, preprocessedStr,
                shaderType, errorOut);
        }
    }
}

