#version 300 es
precision mediump float;
in vec4 vColor;
out vec4 outColor;

uniform sampler2D headTex;
uniform sampler2D trailTex;

void main() {
//    outColor = vColor;
    float lum = texture( headTex, gl_PointCoord).x;
//    float lum = texture( trailTex, gl_PointCoord).x;
    outColor = vColor*lum;
//    outColor = vec4(1.,0,0,lum);
//    outColor = vec4(texture( trailTex, gl_PointCoord).xyz,1.);
//    outColor = vec4(gl_PointCoord,0.,1.);
}