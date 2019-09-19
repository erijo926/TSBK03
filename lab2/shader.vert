#version 150

//in vec3 in_Color;
in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_TexCoord;
uniform mat4 matrix;
uniform mat4 Rbone1;
uniform mat4 Rbone2;
uniform mat4 Tbone1;
uniform mat4 Tbone2;

out vec4 g_color;
const vec3 lightDir = normalize(vec3(0.3, 0.5, 1.0));

// Uppgift 3: Soft-skinning p� GPU
//
// Flytta �ver din implementation av soft skinning fr�n CPU-sidan
// till vertexshadern. Mer info finns p� hemsidan.

void main(void)
{
    mat4 Mbone1 = (Tbone1*Rbone1);
    mat4 Mbone2 = (Tbone2*Rbone2);
    mat4 inv_bone1 = inverse(Mbone1);
    mat4 inv_bone2 = inverse(Mbone2);

    vec4 vert_pos_bone1 = (((Tbone1*inv_bone1)*vec4(in_Position,1.0))*in_TexCoord.x);
    mat4 M_prim2 = (Tbone1*((Tbone2*inv_bone2)*inv_bone1));
    vec4 vert_pos_bone2 = ((M_prim2*vec4(in_Position,1.0))*in_TexCoord.y);

	// transformera resultatet med ModelView- och Projection-matriserna
	gl_Position = matrix * vec4(in_Position, 1.0);
    // gl_Position = matrix * (vert_pos_bone1+vert_pos_bone2);

	// s�tt r�d+gr�n f�rgkanal till vertex Weights
	vec4 color = vec4(in_TexCoord.x, in_TexCoord.y, 0.0, 1.0);

	// L�gg p� en enkel ljuss�ttning p� vertexarna
	float intensity = dot(in_Normal, lightDir);
	color.xyz *= intensity;

	g_color = color;
}
