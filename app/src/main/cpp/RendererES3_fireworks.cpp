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
#include "RendererES3_fireworks.h"

#define STR(s) #s
#define STRV(s) STR(s)

#define STATE_ATTRIB 0
#define POS_ATTRIB 1
#define SPEED_ATTRIB 2
#define TIMER_ATTRIB 3
#define COLOR_ATTRIB 4


RendererES3_fireworks::RendererES3_fireworks()
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

RendererES3_fireworks::~RendererES3_fireworks() {
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

void RendererES3_fireworks::resize(int w, int h) {
//    auto offsets = mapOffsetBuf();
//    calcSceneParams(w, h, offsets);
//    unmapOffsetBuf();
//
//    // Auto gives a signed int :-(
//    for (auto i = (unsigned) 0; i < mNumInstances; i++) {
//        mAngles[i] = drand48() * TWO_PI;
//        mAngularVelocity[i] = MAX_ROT_SPEED * (2.0 * drand48() - 1.0);
//    }
//
//    mLastFrameNs = 0;

    glViewport(0, 0, w, h);
}

//void RendererES3_fireworks::calcSceneParams(unsigned int w, unsigned int h,
//                                            float *offsets) {
//    // number of cells along the larger screen dimension
//    const float NCELLS_MAJOR = MAX_INSTANCES_PER_SIDE;
//    // cell size in scene space
//    const float CELL_SIZE = 2.0f / NCELLS_MAJOR;
//
//    // Calculations are done in "landscape", i.e. assuming dim[0] >= dim[1].
//    // Only at the end are values put in the opposite order if h > w.
//    const float dim[2] = {fmaxf(w, h), fminf(w, h)};
//    const float aspect[2] = {dim[0] / dim[1], dim[1] / dim[0]};
//    const float scene2clip[2] = {1.0f, aspect[0]};
//    const int ncells[2] = {
//            static_cast<int>(NCELLS_MAJOR),
//            (int) floorf(NCELLS_MAJOR * aspect[1])
//    };
//
//    float centers[2][MAX_INSTANCES_PER_SIDE];
//    for (int d = 0; d < 2; d++) {
//        auto offset = -ncells[d] / NCELLS_MAJOR; // -1.0 for d=0
//        for (auto i = 0; i < ncells[d]; i++) {
//            centers[d][i] = scene2clip[d] * (CELL_SIZE * (i + 0.5f) + offset);
//        }
//    }
//
//    int major = w >= h ? 0 : 1;
//    int minor = w >= h ? 1 : 0;
//    // outer product of centers[0] and centers[1]
//    for (int i = 0; i < ncells[0]; i++) {
//        for (int j = 0; j < ncells[1]; j++) {
//            int idx = i * ncells[1] + j;
//            offsets[2 * idx + major] = centers[0][i];
//            offsets[2 * idx + minor] = centers[1][j];
//        }
//    }
//
//    mNumInstances = ncells[0] * ncells[1];
//    mScale[major] = 0.5f * CELL_SIZE * scene2clip[0];
//    mScale[minor] = 0.5f * CELL_SIZE * scene2clip[1];
//}

void RendererES3_fireworks::step() {

    int rnd = rand() % 100;
    if (rnd < 2) {//spawn N0
        ALOGV("SPAWN N0 :%i", curN0);
        float *transforms = mapPosSpdBuf();
        float posX = ((float) (rand() % 1000) / 500. - 1.);
        float spdX = 0.02*((float) (rand() % 1000) / 500. - 1.);
        float spdY = (float) (rand() % 1000) / 100000. + 0.1;
        int curOffset = curN0 * PARTICLES_PER_N0;
        for (unsigned int i = 0; i < PARTICLES_PER_N0; i++) {
            transforms[4 * (i + curOffset) + 0] = posX;
            transforms[4 * (i + curOffset) + 1] = -0.95;
            transforms[4 * (i + curOffset) + 2] = spdX;
            transforms[4 * (i + curOffset) + 3] = spdY;
//            transforms[4 * (i + curOffset) + 2] = ((float) (rand() % 1000) / 500. - 1.) * 0.001;
//            transforms[4 * (i + curOffset) + 3] = ((float) (rand() % 100) / 50. - 1.) * 0.005 + 0.01;
        }
        unmapBuf();

        int *state = mapStateBuf();
        for (unsigned int i = 0; i < PARTICLES_PER_N0; i++) {
            state[i + curOffset] = 1; //trail
        }
        state[curOffset] = 0; //head
        unmapBuf();
        curN0++;
        curN0 %= FIREWORKS_N0_AMOUNT;
    }

    glUseProgram(mProgramTF);
    glEnable(GL_RASTERIZER_DISCARD);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, PARTICLES_AMOUNT);
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

//    timespec now;
//    clock_gettime(CLOCK_MONOTONIC, &now);
//    auto nowNs = now.tv_sec * 1000000000ull + now.tv_nsec;
//
//    float dt = float(nowNs - mLastFrameNs) * 0.000000001f;
//
//    float *transforms = mapPosSpdBuf();
//    for (unsigned int i = 0; i < PARTICLES_AMOUNT; i++) {
//        transforms[4 * i + 0] = 0.5;
//        transforms[4 * i + 1] = 0.5;
//        transforms[4 * i + 2] = 0.5;
//        transforms[4 * i + 3] = 0.5;
//    }
//    unmapBuf();
//    mLastFrameNs = nowNs;

}

//void RendererES3_fireworks::render() {
//    step();
//
//    glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    draw(mNumInstances);
//    checkGlError("Renderer::render");
//}

Renderer *createES3Renderer_fireworks() {
    RendererES3_fireworks *renderer = new RendererES3_fireworks;
    if (!renderer->init()) {
        delete renderer;
        return NULL;
    }
    return renderer;
}

bool RendererES3_fireworks::init() {

//    mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    mProgram = createProgramFromFiles("v_fireworks.glsl", "f_fireworks.glsl", NULL);
    const GLchar *feedbackVaryings[] = {"outState", "outPosSpd", "outTimer"};
    mProgramTF = createProgramFromFiles("v_tf_fireworks.glsl", "f_tf_fireworks.glsl",
                                        feedbackVaryings);

    if (!mProgram)
        return false;
    if (!mProgramTF)
        return false;

//    uTime = glGetUniformLocation(mProgram,"time");

    glGenBuffers(VB_COUNT, mVB);
//    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_INSTANCE]);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD), &QUAD[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_STATE]);
    glBufferData(GL_ARRAY_BUFFER, PARTICLES_AMOUNT * 1 * sizeof(int), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_POS_SPD]);
    glBufferData(GL_ARRAY_BUFFER, PARTICLES_AMOUNT * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_TIMER]);
    glBufferData(GL_ARRAY_BUFFER, PARTICLES_AMOUNT * 1 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_COLOR]);
    glBufferData(GL_ARRAY_BUFFER, PARTICLES_AMOUNT * 4 * sizeof(float), NULL, GL_STATIC_DRAW);

    glGenVertexArrays(1, &mVBState);
    glBindVertexArray(mVBState);

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_STATE]);
    glVertexAttribPointer(STATE_ATTRIB, 1, GL_INT, GL_FALSE, 1 * sizeof(int), 0);
    glEnableVertexAttribArray(STATE_ATTRIB);

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_POS_SPD]);
    glVertexAttribPointer(POS_ATTRIB, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (const GLvoid *) (0 * sizeof(float)));
    glVertexAttribPointer(SPEED_ATTRIB, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (const GLvoid *) (2 * sizeof(float)));
    glEnableVertexAttribArray(POS_ATTRIB);
    glEnableVertexAttribArray(SPEED_ATTRIB);

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_TIMER]);
    glVertexAttribPointer(TIMER_ATTRIB, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), 0);
    glEnableVertexAttribArray(TIMER_ATTRIB);

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_COLOR]);
    glVertexAttribPointer(COLOR_ATTRIB, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(COLOR_ATTRIB);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mVB[VB_STATE]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, mVB[VB_POS_SPD]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, mVB[VB_TIMER]);
//    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 3, mVB[VB_STATE]);


    float *transforms = mapPosSpdBuf();
    for (unsigned int i = 0; i < PARTICLES_AMOUNT; i++) {
        transforms[4 * i + 0] = 2.f;//clip
        transforms[4 * i + 1] = 2.f;
        transforms[4 * i + 2] = 0.f;
        transforms[4 * i + 3] = 0.f;
    }
    unmapBuf();
    float *colors = mapColorBuf();
    for (unsigned int i = 0; i < PARTICLES_AMOUNT; i++) {
        colors[4 * i + 0] = 0.f;
        colors[4 * i + 1] = 1.f;
        colors[4 * i + 2] = 1.f;
        colors[4 * i + 3] = 1.f;
    }
    unmapBuf();
    ALOGV("%i", PARTICLES_AMOUNT);

    ALOGV("Using OpenGL ES 3.0 renderer");
    return true;
}

int *RendererES3_fireworks::mapStateBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_STATE]);
    return (int *) glMapBufferRange(GL_ARRAY_BUFFER,
                                    0 * sizeof(int), PARTICLES_AMOUNT * 1 * sizeof(int),
                                    GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

float *RendererES3_fireworks::mapPosSpdBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_POS_SPD]);
    return (float *) glMapBufferRange(GL_ARRAY_BUFFER,
                                      0 * sizeof(float), PARTICLES_AMOUNT * 4 * sizeof(float),
//                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                                      GL_MAP_WRITE_BIT);
}

float *RendererES3_fireworks::mapColorBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_COLOR]);
    return (float *) glMapBufferRange(GL_ARRAY_BUFFER,
                                      0 * sizeof(float), PARTICLES_AMOUNT * 4 * sizeof(float),
                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

float *RendererES3_fireworks::mapTimerBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_TIMER]);
    return (float *) glMapBufferRange(GL_ARRAY_BUFFER,
                                      0, PARTICLES_AMOUNT * 1 * sizeof(float),
                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void RendererES3_fireworks::unmapBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void RendererES3_fireworks::draw() {
    step();
    glUseProgram(mProgram);
    glBindVertexArray(mVBState);
    glDrawArrays(GL_POINTS, 0, PARTICLES_AMOUNT);
}
