#version 300 es
layout(location = 0) in int state;
layout(location = 1) in vec2 pos;
layout(location = 2) in vec2 spd;
layout(location = 3) in float timer;
//layout(location = 4) in vec4 color;
flat out int outState;
out vec4 outPosSpd;
out float outTimer;
void main() {
//    mat2 sr = mat2(scaleRot.xy, scaleRot.zw);
    outPosSpd = vec4(pos+vec2(0.02),spd);
    outTimer = 1.;
    outState = 1;
}