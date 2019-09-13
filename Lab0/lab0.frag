#version 150

in vec3 ex_Normal;
in vec3 ex_Position;
out vec4 out_Color;
uniform vec3 camPos;
uniform mat4 modelToWorldToView;

in vec2 frag_texcoord;
uniform sampler2D exampletexture;

void main(void)
{
    vec3 light1 = vec3(0,0,1.0);
    float diff, spec;
    float spec_exp = 30.0;

    vec3 r1 = normalize(reflect((ex_Position-light1),ex_Normal));
    vec3 v = normalize(camPos-ex_Position);
    float spec1 = dot(r1,v);

    spec1 = pow(spec1,spec_exp);
    spec = spec1;

    diff = dot(normalize(light1),ex_Normal);
    diff = clamp(diff, 0, 1);
    spec = clamp(spec, 0, 1);
    float shade = diff+spec;
    shade = clamp(shade, 0, 1);
    vec3 color = vec3(1,1,1);

    out_Color=shade*texture(exampletexture,frag_texcoord);

}
