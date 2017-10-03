#ifndef GLES3JNI_RENDERER_H

#define GLES3JNI_RENDERER_H 1
class Renderer {
protected:
    Renderer()=default;
public:
    virtual ~Renderer(){};

//    virtual void resize(int w, int h);
    virtual void resize(int w, int h) =0;
    virtual void draw() = 0;
    virtual void step() =0;
};

#endif //GLES3JNI_RENDERER_H