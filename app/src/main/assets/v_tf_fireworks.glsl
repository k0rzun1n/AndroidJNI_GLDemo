#version 300 es
layout(location = 0) in int state;
layout(location = 1) in vec2 pos;
layout(location = 2) in vec2 spd;
layout(location = 3) in float timer;
//layout(location = 4) in vec4 color;
flat out int outState;
out vec4 outPosSpd;
out float outTimer;

float rand(float n){return fract(sin(n) * 4358.54123);}

void main() {
    outTimer = timer; //probably useless
    outState = state;
    vec2 npos = pos;
    vec2 nspd = spd;
    float damp = 1.-float(gl_VertexID % 100)/100.;
    if(state == 1)//trail
            npos += 0.0001*(-vec2(0.5) + vec2(rand(float(gl_VertexID)),rand(-101.101+float(gl_VertexID))));
    nspd *=0.9;
    nspd *= pow(damp,0.05);
    outPosSpd = vec4(npos + nspd,nspd);
}