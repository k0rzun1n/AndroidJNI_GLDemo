#version 300 es
layout(location = 0) in vec2 pos;
layout(location = 1) in vec4 color;
layout(location = 2) in vec4 scaleRot;
layout(location = 3) in vec2 offset;
out vec4 vColor;
void main() {
    mat2 sr = mat2(scaleRot.xy, scaleRot.zw);
    gl_Position = vec4(sr*pos + offset, 0.0, 1.0);
    vColor = color;
}