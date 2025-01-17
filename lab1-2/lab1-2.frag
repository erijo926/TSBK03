#version 150
// bump mapping should be calculated
// 1) in view coordinates
// 2) in texture coordinates

in vec2 outTexCoord;
in vec3 out_Normal;
in vec3 Ps;
in vec3 Pt;
in vec3 pixPos;  // Needed for specular reflections
uniform sampler2D bumpTex;
out vec4 out_Color;

void main(void)
{
    vec3 light = normalize(vec3(0.0, 0.7, 0.7)); // Light source in view coordinates
    // Calculate gradients here
	float offset = 1.0 / 256.0; // texture size, same in both directions

    // PART 1
    vec4 bs_vec = (texture(bumpTex, outTexCoord+vec2(offset, 0)) -
                    texture(bumpTex, outTexCoord-vec2(0, 0)));
    float bs = 10*(bs_vec.x+bs_vec.y+bs_vec.z) /3;

    vec4 bt_vec = (texture(bumpTex, outTexCoord+vec2(0, offset)) -
                    texture(bumpTex, outTexCoord-vec2(0, 0)));
    float bt = 10*(bt_vec.x+bt_vec.y+bt_vec.z) /3;

    vec3 normal_prim = out_Normal + bs*Ps + bt*Pt;
    vec3 normal = normalize(normal_prim);

    // PART 2
    mat3 Mvt = transpose(mat3(Ps,Pt,normal)); //mat3 creates the matrix with the vectors as columns, whereas we want them as rows.
    vec3 light_T = normalize(Mvt*light);
    vec3 normal_T = normalize(Mvt*out_Normal);
    
	// Simplified lighting calculation.
	// A full solution would include material, ambient, specular, light sources, multiply by texture.
    // out_Color = vec4(dot(normal, light)); // * texture(bumpTex, outTexCoord); //Only light to see the result better in part A
    out_Color = vec4(dot(normal_T, light_T)); // part B
}
