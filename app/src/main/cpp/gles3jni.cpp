/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <jni.h>
#include <stdlib.h>
#include <time.h>

#include "gles3jni.h"

const Vertex QUAD[4] = {
        // Square with diagonal < 2 so that it fits in a [-1 .. 1]^2 square
        // regardless of rotation.
        {{-0.7f, -0.7f}, {0x00, 0xFF, 0x00}},
        {{0.7f,  -0.7f}, {0x00, 0x00, 0xFF}},
        {{-0.7f, 0.7f},  {0xFF, 0x00, 0x00}},
        {{0.7f,  0.7f},  {0xFF, 0xFF, 0xFF}},
};

bool checkGlError(const char *funcName) {
    GLint err = glGetError();
    if (err != GL_NO_ERROR) {
        ALOGE("GL error after %s(): 0x%08x\n", funcName, err);
        return true;
    }
    return false;
}

GLuint createShader(GLenum shaderType, const char *src) {
    GLuint shader = glCreateShader(shaderType);
    if (!shader) {
        checkGlError("glCreateShader");
        return 0;
    }
    glShaderSource(shader, 1, &src, NULL);

    GLint compiled = GL_FALSE;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLogLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0) {
            GLchar *infoLog = (GLchar *) malloc(infoLogLen);
            if (infoLog) {
                glGetShaderInfoLog(shader, infoLogLen, NULL, infoLog);
                ALOGE("Could not compile %s shader:\n%s\n",
                      shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment",
                      infoLog);
                free(infoLog);
            }
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

AAssetManager *mAssMan;

GLuint createProgramFromFiles(const char *vsfname, const char *fsfname, const char **tfVaryings) {
    AAsset *asset = AAssetManager_open(mAssMan, vsfname, AASSET_MODE_BUFFER);
    size_t fileLength = AAsset_getLength(asset);
    char *vtxSrc = new char[fileLength + 1];
    AAsset_read(asset, vtxSrc, fileLength);
    AAsset_close(asset);
    vtxSrc[fileLength] = 0;

    asset = AAssetManager_open(mAssMan, fsfname, AASSET_MODE_BUFFER);
    fileLength = AAsset_getLength(asset);
    char *fragSrc = new char[fileLength + 1];
    AAsset_read(asset, fragSrc, fileLength);
    AAsset_close(asset);
    fragSrc[fileLength] = 0;

    if (tfVaryings != NULL)
        ALOGV("fbv:%s", tfVaryings);
    ALOGV("fshader:%s", fragSrc);
    ALOGV("vshader:%s", vtxSrc);

    GLuint mProg = createProgram(vtxSrc, fragSrc, tfVaryings);

    delete[] fragSrc;
    delete[] vtxSrc;

    return mProg;
}

GLuint createProgram(const char *vtxSrc, const char *fragSrc, const char **tfVaryings) {
    GLuint vtxShader = 0;
    GLuint fragShader = 0;
    GLuint program = 0;
    GLint linked = GL_FALSE;

    program = glCreateProgram();
    if (!program) {
        checkGlError("glCreateProgram");
        goto exit;
    }

    vtxShader = createShader(GL_VERTEX_SHADER, vtxSrc);
    if (!vtxShader)
        goto exit;
    glAttachShader(program, vtxShader);

    fragShader = createShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!fragShader)
        goto exit;
    glAttachShader(program, fragShader);

    if (tfVaryings != NULL)
        glTransformFeedbackVaryings(program, 3, tfVaryings, GL_SEPARATE_ATTRIBS);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        ALOGE("Could not link program");
        GLint infoLogLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen) {
            GLchar *infoLog = (GLchar *) malloc(infoLogLen);
            if (infoLog) {
                glGetProgramInfoLog(program, infoLogLen, NULL, infoLog);
                ALOGE("Could not link program:\n%s\n", infoLog);
                free(infoLog);
            }
        }
        glDeleteProgram(program);
        program = 0;
    }

    exit:
    glDeleteShader(vtxShader);
    glDeleteShader(fragShader);
    return program;
}

static void printGlString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    ALOGV("GL %s: %s\n", name, v);
}


// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

static Renderer *g_renderer_fireworks = NULL;
static Renderer *g_renderer_flag = NULL;
static Renderer *g_renderer_lp_animal = NULL;
static Renderer *g_renderer_butterfly = NULL;

extern "C" {
JNIEXPORT void JNICALL
Java_com_krz_DemoGLJNIYo_GLES3JNILib_init(JNIEnv *env, jobject obj, jobject assetManager);
JNIEXPORT void JNICALL
Java_com_krz_DemoGLJNIYo_GLES3JNILib_resize(JNIEnv *env, jobject obj, jint width, jint height);
JNIEXPORT void JNICALL Java_com_krz_DemoGLJNIYo_GLES3JNILib_step(JNIEnv *env, jobject obj);
};

#if !defined(DYNAMIC_ES3)

static GLboolean gl3stubInit() {
    return GL_TRUE;
}
#endif

JNIEXPORT void JNICALL
Java_com_krz_DemoGLJNIYo_GLES3JNILib_init(JNIEnv *env, jobject obj, jobject assetManager) {

    mAssMan = AAssetManager_fromJava(env, assetManager);

    if (g_renderer_fireworks) {
        delete g_renderer_fireworks;
        g_renderer_fireworks = NULL;
    }
    if (g_renderer_flag) {
        delete g_renderer_flag;
        g_renderer_flag = NULL;
    }
    if (g_renderer_lp_animal) {
        delete g_renderer_lp_animal;
        g_renderer_lp_animal = NULL;
    }
    if (g_renderer_butterfly) {
        delete g_renderer_butterfly;
        g_renderer_butterfly = NULL;
    }

    printGlString("Version", GL_VERSION);
    printGlString("Vendor", GL_VENDOR);
    printGlString("Renderer", GL_RENDERER);
    printGlString("Extensions", GL_EXTENSIONS);

    const char *versionStr = (const char *) glGetString(GL_VERSION);
    if (strstr(versionStr, "OpenGL ES 3.") && gl3stubInit()) {
        g_renderer_fireworks = createES3Renderer_fireworks();
//        g_renderer_flag = createES3Renderer();
//        g_renderer_lp_animal = createES3Renderer();
//        g_renderer_butterfly = createES3Renderer();
    } else {
//    } else if (strstr(versionStr, "OpenGL ES 2.")) {
//        g_renderer = createES2Renderer();
        ALOGE("Unsupported OpenGL ES version");
    }
}

JNIEXPORT void JNICALL
Java_com_krz_DemoGLJNIYo_GLES3JNILib_resize(JNIEnv *env, jobject obj, jint width, jint height) {
    if (g_renderer_fireworks) {
        g_renderer_fireworks->resize(width, height);
    }
    if (g_renderer_flag) {
        g_renderer_flag->resize(width, height);
    }
    if (g_renderer_lp_animal) {
        g_renderer_lp_animal->resize(width, height);
    }
    if (g_renderer_butterfly) {
        g_renderer_butterfly->resize(width, height);
    }
}

JNIEXPORT void JNICALL
Java_com_krz_DemoGLJNIYo_GLES3JNILib_step(JNIEnv *env, jobject obj) {
    if (g_renderer_lp_animal) {
        g_renderer_lp_animal->draw();
    }
    if (g_renderer_butterfly) {
        g_renderer_butterfly->draw();
    }
    if (g_renderer_flag) {
        g_renderer_flag->draw();
    }
    if (g_renderer_fireworks) {
        g_renderer_fireworks->draw();
    }

}
