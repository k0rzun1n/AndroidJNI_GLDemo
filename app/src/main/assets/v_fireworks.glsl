#version 300 es
layout(location = 0) in int state;
layout(location = 1) in vec2 pos;
layout(location = 2) in vec2 spd;
layout(location = 3) in float timer;
layout(location = 4) in vec4 color;
out vec4 vColor;

//uniform float time;
float rand(float n){return fract(sin(n) * 4358.54123);}

void main() {
//    mat2 sr = mat2(scaleRot.xy, scaleRot.zw);

    if(state == 0){//head
        gl_Position = vec4(pos.x, pos.y, -0.3, 1.0);
        gl_PointSize = 300.;
        vColor = vec4(1.0,0.,0.,1.);
    }else{
        gl_Position = vec4(pos.x, pos.y, 0., 1.0);
        gl_PointSize = 70.;
        vColor = color;
    }
    float id = float(gl_VertexID);
//    vColor = vec4(rand(id),rand(id*2.),rand(id*3.),1.0);
    vColor = color;
}