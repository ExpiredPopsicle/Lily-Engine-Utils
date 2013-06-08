// Auto-generated GL function pointer setup for GLContext.
// (Compile this on its own and link it with the project.)
#include <GL/gl.h>
#include <GL/glext.h>
#include <lilyengine/glcontext.h>
namespace ExPop {
    namespace Gfx {
        void GLContext::initFunctions(void) {
            // Buffer related stuff.
            checkGetProcResult("glGenBuffers", (void*)(glGenBuffers = (void(EXPOP_GL_API *)(GLsizei, GLuint*))getGLProcAddress("glGenBuffers")));
            checkGetProcResult("glBindBuffer", (void*)(glBindBuffer = (void(EXPOP_GL_API *)(GLenum, GLuint))getGLProcAddress("glBindBuffer")));
            checkGetProcResult("glBufferData", (void*)(glBufferData = (void(EXPOP_GL_API *)(GLenum, ptrdiff_t, const GLvoid*, GLenum))getGLProcAddress("glBufferData")));
            checkGetProcResult("glEnableVertexAttribArray", (void*)(glEnableVertexAttribArray = (void(EXPOP_GL_API *)(GLuint))getGLProcAddress("glEnableVertexAttribArray")));
            checkGetProcResult("glDisableVertexAttribArray", (void*)(glDisableVertexAttribArray = (void(EXPOP_GL_API *)(GLuint))getGLProcAddress("glDisableVertexAttribArray")));
            checkGetProcResult("glDrawElements", (void*)(glDrawElements = (void(EXPOP_GL_API *)(GLenum, GLsizei, GLenum, GLvoid*))getGLProcAddress("glDrawElements")));

            // Shader related stuff.
            checkGetProcResult("glAttachShader", (void*)(glAttachShader = (void(EXPOP_GL_API *)(GLuint, GLuint))getGLProcAddress("glAttachShader")));
            checkGetProcResult("glLinkProgram", (void*)(glLinkProgram = (void(EXPOP_GL_API *)(GLuint))getGLProcAddress("glLinkProgram")));
            checkGetProcResult("glUseProgram", (void*)(glUseProgram = (void(EXPOP_GL_API *)(GLuint))getGLProcAddress("glUseProgram")));
            checkGetProcResult("glCreateShader", (void*)(glCreateShader = (GLuint(EXPOP_GL_API *)(GLenum))getGLProcAddress("glCreateShader")));
            checkGetProcResult("glCreateProgram", (void*)(glCreateProgram = (GLuint(EXPOP_GL_API *)(void))getGLProcAddress("glCreateProgram")));
            checkGetProcResult("glCompileShader", (void*)(glCompileShader = (void(EXPOP_GL_API *)(GLuint))getGLProcAddress("glCompileShader")));
            checkGetProcResult("glShaderSource", (void*)(glShaderSource = (void(EXPOP_GL_API *)(GLuint, GLsizei, const char**, const GLint*))getGLProcAddress("glShaderSource")));
            checkGetProcResult("glGetShaderiv", (void*)(glGetShaderiv = (void(EXPOP_GL_API *)(GLuint, GLenum, GLint*))getGLProcAddress("glGetShaderiv")));
            checkGetProcResult("glGetProgramiv", (void*)(glGetProgramiv = (void(EXPOP_GL_API *)(GLuint, GLenum, GLint*))getGLProcAddress("glGetProgramiv")));
            checkGetProcResult("glGetUniformLocation", (void*)(glGetUniformLocation = (GLint(EXPOP_GL_API *)(GLuint, const char*))getGLProcAddress("glGetUniformLocation")));
            checkGetProcResult("glGetAttribLocation", (void*)(glGetAttribLocation = (GLint(EXPOP_GL_API *)(GLuint, const char*))getGLProcAddress("glGetAttribLocation")));
            checkGetProcResult("glBindAttribLocation", (void*)(glBindAttribLocation = (void(EXPOP_GL_API *)(GLuint, GLuint, const char*))getGLProcAddress("glBindAttribLocation")));
            checkGetProcResult("glGetProgramInfoLog", (void*)(glGetProgramInfoLog = (void(EXPOP_GL_API *)(GLuint, GLsizei, GLsizei*, char*))getGLProcAddress("glGetProgramInfoLog")));
            checkGetProcResult("glGetShaderInfoLog", (void*)(glGetShaderInfoLog = (void(EXPOP_GL_API *)(GLuint, GLsizei, GLsizei*, char*))getGLProcAddress("glGetShaderInfoLog")));
            checkGetProcResult("glDeleteProgram", (void*)(glDeleteProgram = (void(EXPOP_GL_API *)(GLuint))getGLProcAddress("glDeleteProgram")));
            checkGetProcResult("glDeleteShader", (void*)(glDeleteShader = (void(EXPOP_GL_API *)(GLuint))getGLProcAddress("glDeleteShader")));

            // Shader inputs.
            checkGetProcResult("glUniform1f", (void*)(glUniform1f = (void(EXPOP_GL_API *)(GLint, GLfloat))getGLProcAddress("glUniform1f")));
            checkGetProcResult("glUniform2f", (void*)(glUniform2f = (void(EXPOP_GL_API *)(GLint, GLfloat, GLfloat))getGLProcAddress("glUniform2f")));
            checkGetProcResult("glUniform3f", (void*)(glUniform3f = (void(EXPOP_GL_API *)(GLint, GLfloat, GLfloat, GLfloat))getGLProcAddress("glUniform3f")));
            checkGetProcResult("glUniform4f", (void*)(glUniform4f = (void(EXPOP_GL_API *)(GLint, GLfloat, GLfloat, GLfloat, GLfloat))getGLProcAddress("glUniform4f")));

            checkGetProcResult("glUniform1i", (void*)(glUniform1i = (void(EXPOP_GL_API *)(GLint, GLint))getGLProcAddress("glUniform1i")));
            checkGetProcResult("glUniform2i", (void*)(glUniform2i = (void(EXPOP_GL_API *)(GLint, GLint, GLint))getGLProcAddress("glUniform2i")));
            checkGetProcResult("glUniform3i", (void*)(glUniform3i = (void(EXPOP_GL_API *)(GLint, GLint, GLint, GLint))getGLProcAddress("glUniform3i")));
            checkGetProcResult("glUniform4i", (void*)(glUniform4i = (void(EXPOP_GL_API *)(GLint, GLint, GLint, GLint, GLint))getGLProcAddress("glUniform4i")));

            checkGetProcResult("glUniform1fv", (void*)(glUniform1fv = (void(EXPOP_GL_API *)(GLint, GLsizei, const GLfloat*))getGLProcAddress("glUniform1fv")));
            checkGetProcResult("glUniform2fv", (void*)(glUniform2fv = (void(EXPOP_GL_API *)(GLint, GLsizei, const GLfloat*))getGLProcAddress("glUniform2fv")));
            checkGetProcResult("glUniform3fv", (void*)(glUniform3fv = (void(EXPOP_GL_API *)(GLint, GLsizei, const GLfloat*))getGLProcAddress("glUniform3fv")));
            checkGetProcResult("glUniform4fv", (void*)(glUniform4fv = (void(EXPOP_GL_API *)(GLint, GLsizei, const GLfloat*))getGLProcAddress("glUniform4fv")));

            checkGetProcResult("glUniform1iv", (void*)(glUniform1iv = (void(EXPOP_GL_API *)(GLint, GLsizei, const GLint*))getGLProcAddress("glUniform1iv")));
            checkGetProcResult("glUniform2iv", (void*)(glUniform2iv = (void(EXPOP_GL_API *)(GLint, GLsizei, const GLint*))getGLProcAddress("glUniform2iv")));
            checkGetProcResult("glUniform3iv", (void*)(glUniform3iv = (void(EXPOP_GL_API *)(GLint, GLsizei, const GLint*))getGLProcAddress("glUniform3iv")));
            checkGetProcResult("glUniform4iv", (void*)(glUniform4iv = (void(EXPOP_GL_API *)(GLint, GLsizei, const GLint*))getGLProcAddress("glUniform4iv")));

            checkGetProcResult("glUniformMatrix2fv", (void*)(glUniformMatrix2fv = (void(EXPOP_GL_API *)(GLint, GLsizei, GLboolean, const GLfloat*))getGLProcAddress("glUniformMatrix2fv")));
            checkGetProcResult("glUniformMatrix3fv", (void*)(glUniformMatrix3fv = (void(EXPOP_GL_API *)(GLint, GLsizei, GLboolean, const GLfloat*))getGLProcAddress("glUniformMatrix3fv")));
            checkGetProcResult("glUniformMatrix4fv", (void*)(glUniformMatrix4fv = (void(EXPOP_GL_API *)(GLint, GLsizei, GLboolean, const GLfloat*))getGLProcAddress("glUniformMatrix4fv")));
            checkGetProcResult("glUniformMatrix2x3fv", (void*)(glUniformMatrix2x3fv = (void(EXPOP_GL_API *)(GLint, GLsizei, GLboolean, const GLfloat*))getGLProcAddress("glUniformMatrix2x3fv")));
            checkGetProcResult("glUniformMatrix3x2fv", (void*)(glUniformMatrix3x2fv = (void(EXPOP_GL_API *)(GLint, GLsizei, GLboolean, const GLfloat*))getGLProcAddress("glUniformMatrix3x2fv")));
            checkGetProcResult("glUniformMatrix2x4fv", (void*)(glUniformMatrix2x4fv = (void(EXPOP_GL_API *)(GLint, GLsizei, GLboolean, const GLfloat*))getGLProcAddress("glUniformMatrix2x4fv")));
            checkGetProcResult("glUniformMatrix4x2fv", (void*)(glUniformMatrix4x2fv = (void(EXPOP_GL_API *)(GLint, GLsizei, GLboolean, const GLfloat*))getGLProcAddress("glUniformMatrix4x2fv")));
            checkGetProcResult("glUniformMatrix3x4fv", (void*)(glUniformMatrix3x4fv = (void(EXPOP_GL_API *)(GLint, GLsizei, GLboolean, const GLfloat*))getGLProcAddress("glUniformMatrix3x4fv")));
            checkGetProcResult("glUniformMatrix4x3fv", (void*)(glUniformMatrix4x3fv = (void(EXPOP_GL_API *)(GLint, GLsizei, GLboolean, const GLfloat*))getGLProcAddress("glUniformMatrix4x3fv")));

            checkGetProcResult("glVertexAttribPointer", (void*)(glVertexAttribPointer = (void(EXPOP_GL_API *)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*))getGLProcAddress("glVertexAttribPointer")));

            // Normal OpenGL stuff that's probably already present.
            checkGetProcResult("glEnable", (void*)(glEnable = (void(EXPOP_GL_API *)(GLenum))getGLProcAddress("glEnable")));
            checkGetProcResult("glDisable", (void*)(glDisable = (void(EXPOP_GL_API *)(GLenum))getGLProcAddress("glDisable")));
            checkGetProcResult("glBlendFunc", (void*)(glBlendFunc = (void(EXPOP_GL_API *)(GLenum, GLenum))getGLProcAddress("glBlendFunc")));
            checkGetProcResult("glBlendEquation", (void*)(glBlendEquation = (void(EXPOP_GL_API *)(GLenum))getGLProcAddress("glBlendEquation")));
            checkGetProcResult("glBindTexture", (void*)(glBindTexture = (void(EXPOP_GL_API *)(GLenum, GLuint))getGLProcAddress("glBindTexture")));
            checkGetProcResult("glActiveTexture", (void*)(glActiveTexture = (void(EXPOP_GL_API *)(GLenum))getGLProcAddress("glActiveTexture")));
            checkGetProcResult("glDeleteTextures", (void*)(glDeleteTextures = (void(EXPOP_GL_API *)(GLsizei, const GLuint*))getGLProcAddress("glDeleteTextures")));
            checkGetProcResult("glDeleteBuffers", (void*)(glDeleteBuffers = (void(EXPOP_GL_API *)(GLsizei, const GLuint*))getGLProcAddress("glDeleteBuffers")));

            // FBO stuff
            checkGetProcResult("glGenFramebuffers", (void*)(glGenFramebuffers = (void(EXPOP_GL_API *)(GLsizei, GLuint*))getGLProcAddress("glGenFramebuffers")));
            checkGetProcResult("glDeleteFramebuffers", (void*)(glDeleteFramebuffers = (void(EXPOP_GL_API *)(GLsizei, const GLuint*))getGLProcAddress("glDeleteFramebuffers")));
            checkGetProcResult("glBindFramebuffer", (void*)(glBindFramebuffer = (void(EXPOP_GL_API *)(GLenum, GLuint))getGLProcAddress("glBindFramebuffer")));

            checkGetProcResult("glFramebufferTexture2D", (void*)(glFramebufferTexture2D = (void(EXPOP_GL_API *)(GLenum, GLenum, GLenum, GLuint, GLint))getGLProcAddress("glFramebufferTexture2D")));


            // Renderbuffers
            checkGetProcResult("glGenRenderbuffers", (void*)(glGenRenderbuffers = (void(EXPOP_GL_API *)(GLsizei, GLuint*))getGLProcAddress("glGenRenderbuffers")));
            checkGetProcResult("glDeleteRenderbuffers", (void*)(glDeleteRenderbuffers = (void(EXPOP_GL_API *)(GLsizei, const GLuint*))getGLProcAddress("glDeleteRenderbuffers")));
            checkGetProcResult("glBindRenderbuffer", (void*)(glBindRenderbuffer = (void(EXPOP_GL_API *)(GLenum, GLuint))getGLProcAddress("glBindRenderbuffer")));
            checkGetProcResult("glRenderbufferStorage", (void*)(glRenderbufferStorage = (void(EXPOP_GL_API *)(GLenum, GLenum, GLsizei, GLsizei))getGLProcAddress("glRenderbufferStorage")));
            checkGetProcResult("glGetRenderbufferParameteriv", (void*)(glGetRenderbufferParameteriv = (void(EXPOP_GL_API *)(GLenum, GLenum, GLint*))getGLProcAddress("glGetRenderbufferParameteriv")));

        }
    }
}
