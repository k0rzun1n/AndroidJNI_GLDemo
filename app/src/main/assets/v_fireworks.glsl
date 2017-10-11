#version 300 es
layout(location = 0) in int state;
layout(location = 1) in vec2 pos;
layout(location = 2) in vec2 spd;
layout(location = 3) in float timer;
layout(location = 4) in vec4 color;
out vec4 vColor;

//uniform float time;

void main() {
//    mat2 sr = mat2(scaleRot.xy, scaleRot.zw);
    gl_Position = vec4(pos.x, pos.y, 0., 1.0);
    if(state == 0){//head
        gl_PointSize = 50.;
        vColor = vec4(1.0,0.,0.,1.);
    }else{
        gl_PointSize = 5.;
        vColor = color;
    }
}