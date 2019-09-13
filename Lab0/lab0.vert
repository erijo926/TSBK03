#version 150

in  vec3  in_Position;
in  vec3  in_Normal;
in  vec2  in_TexCoord;

uniform mat3 rotationMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelToWorldToView;

uniform float time;

out vec2 frag_texcoord;
out vec3 ex_Normal;
out vec3 ex_Position;

void main(void)
{
    ex_Normal = rotationMatrix*in_Normal;
    ex_Position = in_Position;

    frag_texcoord=in_TexCoord;

    vec3 pos = in_Position;
    // pos.y += sin(pos.x+time);
	gl_Position = projectionMatrix*modelToWorldToView*vec4(pos, 1.0);
}
