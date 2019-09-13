#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

void main(void)
{
    int steps = 3;
    vec4 color = vec4(0.0,0.0,0.0,0.0);
    float offset = 1.0/512.0;
    for (int s=-steps; s < steps; ++s)
    {
        for (int t=-steps; t < steps; ++t)
        {
            color += texture(texUnit, outTexCoord+vec2(s*offset,t*offset));
        }
    }

    float norm_coefficient = 1/pow(2*steps,2);//(color.x+color.y+color.z);
    out_Color = clamp(color*norm_coefficient,0,1);
}
