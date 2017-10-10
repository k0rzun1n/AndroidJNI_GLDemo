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

#define PARTICLES_PER_RAY 100
#define RAYS_PER_FIREWORK 10 + 1
//different nesting levels
#define FIREWORKS_N0_AMOUNT 10
#define FIREWORKS_N1_AMOUNT 10
#define PARTICLES_PER_N0 PARTICLES_PER_RAY
#define PARTICLES_PER_N1 (PARTICLES_PER_RAY * RAYS_PER_FIREWORK )
#define PARTICLES_AMOUNT (FIREWORKS_N0_AMOUNT * PARTICLES_PER_N0 + FIREWORKS_N1_AMOUNT * PARTICLES_PER_N1)

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
//        VB_INSTANCE, VB_SCALEROT, VB_OFFSET, VB_COUNT
        VB_STATE, VB_POS_SPD, VB_TIMER, VB_COLOR, VB_COUNT
    };

    int *mapStateBuf();
    float *mapPosBuf();
    float *mapSpdBuf();
    float *mapTimerBuf();
    void unmapBuf();

    void draw();

    const EGLContext mEglContext;
    GLuint mProgram, mProgramTF;
    GLuint mVB[VB_COUNT];
    GLuint mVBState;
};

#endif //GLES3JNI_FW_H