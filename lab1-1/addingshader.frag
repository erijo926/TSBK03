#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
uniform sampler2D texUnit2;
out vec4 out_Color;

void main(void)
{
    vec4 original = texture(texUnit, outTexCoord);
    vec4 bloom = clamp(texture(texUnit2, outTexCoord), 0.0,1.0);
    out_Color = clamp(original+bloom, 0.0, 1.0);
}
