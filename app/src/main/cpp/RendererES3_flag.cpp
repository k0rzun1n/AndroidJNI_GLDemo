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

#include "gles3jni.h"
#include "RendererES3_flag.h"

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define COLOR_ATTRIB 1
#define SCALEROT_ATTRIB 2
#define OFFSET_ATTRIB 3

static const char VERTEX_SHADER[] =
        "#version 300 es\n"
                "layout(location = " STRV(POS_ATTRIB) ") in vec2 pos;\n"
                "layout(location=" STRV(COLOR_ATTRIB) ") in vec4 color;\n"
                "layout(location=" STRV(SCALEROT_ATTRIB) ") in vec4 scaleRot;\n"
                "layout(location=" STRV(OFFSET_ATTRIB) ") in vec2 offset;\n"
                "out vec4 vColor;\n"
                "void main() {\n"
                "    mat2 sr = mat2(scaleRot.xy, scaleRot.zw);\n"
                "    gl_Position = vec4(sr*pos + offset, 0.0, 1.0);\n"
                "    vColor = color;\n"
                "}\n";

static const char FRAGMENT_SHADER[] =
        "#version 300 es\n"
                "precision mediump float;\n"
                "in vec4 vColor;\n"
                "out vec4 outColor;\n"
                "void main() {\n"
                "    outColor = vColor;\n"
                "}\n";


RendererES3_flag::RendererES3_flag()
        : mNumInstances(0),
          mLastFrameNs(0),
          mEglContext(eglGetCurrentContext()),
          mProgram(0),
          mVBState(0) {
    memset(mScale, 0, sizeof(mScale));
    memset(mAngularVelocity, 0, sizeof(mAngularVelocity));
    memset(mAngles, 0, sizeof(mAngles));
    for (int i = 0; i < VB_COUNT; i++)
        mVB[i] = 0;
}

RendererES3_flag::~RendererES3_flag() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */
    if (eglGetCurrentContext() != mEglContext)
        return;
    glDeleteVertexArrays(1, &mVBState);
    glDeleteBuffers(VB_COUNT, mVB);
    glDeleteProgram(mProgram);
}

void RendererES3_flag::resize(int w, int h) {
    auto offsets = mapOffsetBuf();
    calcSceneParams(w, h, offsets);
    unmapOffsetBuf();

    // Auto gives a signed int :-(
    for (auto i = (unsigned) 0; i < mNumInstances; i++) {
        mAngles[i] = drand48() * TWO_PI;
        mAngularVelocity[i] = MAX_ROT_SPEED * (2.0 * drand48() - 1.0);
    }

    mLastFrameNs = 0;

    glViewport(0, 0, w, h);
}

void RendererES3_flag::calcSceneParams(unsigned int w, unsigned int h,
                                            float *offsets) {
    // number of cells along the larger screen dimension
    const float NCELLS_MAJOR = MAX_INSTANCES_PER_SIDE;
    // cell size in scene space
    const float CELL_SIZE = 2.0f / NCELLS_MAJOR;

    // Calculations are done in "landscape", i.e. assuming dim[0] >= dim[1].
    // Only at the end are values put in the opposite order if h > w.
    const float dim[2] = {fmaxf(w, h), fminf(w, h)};
    const float aspect[2] = {dim[0] / dim[1], dim[1] / dim[0]};
    const float scene2clip[2] = {1.0f, aspect[0]};
    const int ncells[2] = {
            static_cast<int>(NCELLS_MAJOR),
            (int) floorf(NCELLS_MAJOR * aspect[1])
    };

    float centers[2][MAX_INSTANCES_PER_SIDE];
    for (int d = 0; d < 2; d++) {
        auto offset = -ncells[d] / NCELLS_MAJOR; // -1.0 for d=0
        for (auto i = 0; i < ncells[d]; i++) {
            centers[d][i] = scene2clip[d] * (CELL_SIZE * (i + 0.5f) + offset);
        }
    }

    int major = w >= h ? 0 : 1;
    int minor = w >= h ? 1 : 0;
    // outer product of centers[0] and centers[1]
    for (int i = 0; i < ncells[0]; i++) {
        for (int j = 0; j < ncells[1]; j++) {
            int idx = i * ncells[1] + j;
            offsets[2 * idx + major] = centers[0][i];
            offsets[2 * idx + minor] = centers[1][j];
        }
    }

    mNumInstances = ncells[0] * ncells[1];
    mScale[major] = 0.5f * CELL_SIZE * scene2clip[0];
    mScale[minor] = 0.5f * CELL_SIZE * scene2clip[1];
}

void RendererES3_flag::step() {
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    auto nowNs = now.tv_sec * 1000000000ull + now.tv_nsec;

    if (mLastFrameNs > 0) {
        float dt = float(nowNs - mLastFrameNs) * 0.000000001f;

        for (unsigned int i = 0; i < mNumInstances; i++) {
            mAngles[i] += mAngularVelocity[i] * dt;
            if (mAngles[i] >= TWO_PI) {
                mAngles[i] -= TWO_PI;
            } else if (mAngles[i] <= -TWO_PI) {
                mAngles[i] += TWO_PI;
            }
        }

        float *transforms = mapTransformBuf();
        for (unsigned int i = 0; i < mNumInstances; i++) {
            float s = sinf(mAngles[i]);
            float c = cosf(mAngles[i]);
            transforms[4 * i + 0] = c * mScale[0];
            transforms[4 * i + 1] = s * mScale[1];
            transforms[4 * i + 2] = -s * mScale[0];
            transforms[4 * i + 3] = c * mScale[1];
        }
        unmapTransformBuf();
    }

    mLastFrameNs = nowNs;
}

//void RendererES3_flag::render() {
//    step();
//
//    glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    draw(mNumInstances);
//    checkGlError("Renderer::render");
//}

Renderer *createES3Renderer_flag() {
    RendererES3_flag *renderer = new RendererES3_flag;
    if (!renderer->init()) {
        delete renderer;
        return NULL;
    }
    return renderer;
}


bool RendererES3_flag::init() {
    mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (!mProgram)
        return false;

    glGenBuffers(VB_COUNT, mVB);
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_INSTANCE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD), &QUAD[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_SCALEROT]);
    glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCES * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_OFFSET]);
    glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCES * 2 * sizeof(float), NULL, GL_STATIC_DRAW);

    glGenVertexArrays(1, &mVBState);
    glBindVertexArray(mVBState);

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_INSTANCE]);
    glVertexAttribPointer(POS_ATTRIB, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (const GLvoid *) offsetof(Vertex, pos));
    glVertexAttribPointer(COLOR_ATTRIB, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                          (const GLvoid *) offsetof(Vertex, rgba));
    glEnableVertexAttribArray(POS_ATTRIB);
    glEnableVertexAttribArray(COLOR_ATTRIB);

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_SCALEROT]);
    glVertexAttribPointer(SCALEROT_ATTRIB, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(SCALEROT_ATTRIB);
    glVertexAttribDivisor(SCALEROT_ATTRIB, 1);

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_OFFSET]);
    glVertexAttribPointer(OFFSET_ATTRIB, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(OFFSET_ATTRIB);
    glVertexAttribDivisor(OFFSET_ATTRIB, 1);

    ALOGV("Using OpenGL ES 3.0 renderer");
    return true;
}


float *RendererES3_flag::mapOffsetBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_OFFSET]);
    return (float *) glMapBufferRange(GL_ARRAY_BUFFER,
                                      0, MAX_INSTANCES * 2 * sizeof(float),
                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void RendererES3_flag::unmapOffsetBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

float *RendererES3_flag::mapTransformBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_SCALEROT]);
    return (float *) glMapBufferRange(GL_ARRAY_BUFFER,
                                      0, MAX_INSTANCES * 4 * sizeof(float),
                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void RendererES3_flag::unmapTransformBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void RendererES3_flag::draw() {
    step();
    glUseProgram(mProgram);
    glBindVertexArray(mVBState);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 11);
}
