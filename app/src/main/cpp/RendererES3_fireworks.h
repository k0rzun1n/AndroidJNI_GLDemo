#ifndef GLES3JNI_FW_H

#define GLES3JNI_FW_H 1

#include <EGL/egl.h>
#include <android/log.h>
#include <math.h>
#include <GLES3/gl3.h>
#include <jni.h>
#include <stdlib.h>
#include <time.h>
#include "Renderer.h"

#define MAX_INSTANCES_PER_SIDE 16
#define MAX_INSTANCES   (MAX_INSTANCES_PER_SIDE * MAX_INSTANCES_PER_SIDE)
#define TWO_PI          (2.0 * M_PI)
#define MAX_ROT_SPEED   (0.3 * TWO_PI)

class RendererES3_fireworks: public Renderer  {
public:
    ~RendererES3_fireworks();

    void resize(int w, int h);

//    void render();

    RendererES3_fireworks();

    bool init();

private:
    void calcSceneParams(unsigned int w, unsigned int h, float *offsets);

    void step();

    unsigned int mNumInstances;
    float mScale[2];
    float mAngularVelocity[MAX_INSTANCES];
    uint64_t mLastFrameNs;
    float mAngles[MAX_INSTANCES];

    enum {
        VB_INSTANCE, VB_SCALEROT, VB_OFFSET, VB_COUNT
    };

    // return a pointer to a buffer of MAX_INSTANCES * sizeof(vec2).
    // the buffer is filled with per-instance offsets, then unmapped.
    float *mapOffsetBuf();

    void unmapOffsetBuf();

    // return a pointer to a buffer of MAX_INSTANCES * sizeof(vec4).
    // the buffer is filled with per-instance scale and rotation transforms.
    float *mapTransformBuf();

    void unmapTransformBuf();

    void draw();

    const EGLContext mEglContext;
    GLuint mProgram;
    GLuint mVB[VB_COUNT];
    GLuint mVBState;
};

#endif //GLES3JNI_FW_H