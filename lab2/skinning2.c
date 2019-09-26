// Part B: Many-bone worm

// New version by Ingemar 2010
// Removed all dependencies of the Wild Magic (wml) library.
// Replaced it with VectorUtils2 (in source)
// Replaced old shader module with the simpler "ShaderUtils" unit.

// 2013: Adapted to VectorUtils3 and MicroGlut.

// gcc skinning2.c ../common/*.c -lGL -o skinning2 -I../common
// not working any more. This is untested but closer to the truth:
// gcc skinning2.c -o skinning2 ../common/*.c ../common/Linux/MicroGlut.c -I../common -I../common/Linux -DGL_GLEXT_PROTOTYPES -lXt -lX11 -lGL -lm

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#ifdef __APPLE__
// Mac
    #include <OpenGL/gl3.h>
//uses framework Cocoa
#else
    #ifdef WIN32
// MS
        #include <stdio.h>
        #include <GL/glew.h>
    #else
// Linux
        #include <GL/gl.h>
    #endif
#endif

#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include <string.h>

// Ref till shader
GLuint g_shader;


// vec2 is mostly useful for texture coordinates, otherwise you don't use it much.
// That is why VectorUtils3 doesn't support it (yet)
typedef struct vec2
{
    GLfloat s, t;
}
vec2, *vec2Ptr;


typedef struct Triangle
{
  GLuint        v1;
  GLuint        v2;
  GLuint        v3;
} Triangle;

#define CYLINDER_SEGMENT_LENGTH 0.37
#define kMaxRow 100
#define kMaxCorners 8
#define kMaxBones 10
#define kMaxg_poly ((kMaxRow-1) * kMaxCorners * 2)
#ifndef Pi
#define Pi 3.1416
#endif
#ifndef true
#define true 1
#endif

#define BONE_LENGTH 4.0

Triangle g_poly[kMaxg_poly];

// vertices
vec3 g_vertsOrg[kMaxRow][kMaxCorners];
vec3 g_normalsOrg[kMaxRow][kMaxCorners];
vec3 g_vertsRes[kMaxRow][kMaxCorners];
vec3 g_normalsRes[kMaxRow][kMaxCorners];

// vertex attributes
float g_boneWeights[kMaxRow][kMaxCorners][kMaxBones];
vec2 g_boneWeightVis[kMaxRow][kMaxCorners]; // Copy data to here to visualize your weights

Model *cylinderModel; // Collects all the above for drawing with glDrawElements

mat4 modelViewMatrix, projectionMatrix;

///////////////////////////////////////////////////
//      I N I T  B O N E  W E I G H T S
// Desc:  initierar benvikterna
//
void initBoneWeights(void)
{
    long    row, corner;
    int bone;

    // s�tter v�rden till alla vertexar i meshen
    for (row = 0; row < kMaxRow; row++)
        for (corner = 0; corner < kMaxCorners; corner++)
        {
            float boneWeights[kMaxBones];
            float totalBoneWeight = 0.f;

            float maxBoneWeight = 0.f;

            for (bone = 0; bone < kMaxBones; bone++)
            {
                float bonePos = BONE_LENGTH * bone;
                float boneDist = fabs(bonePos - g_vertsOrg[row][corner].x);
                float boneWeight = (BONE_LENGTH - boneDist) / (BONE_LENGTH);
                if (boneWeight < 0)
                    boneWeight = 0;
                boneWeights[bone] = boneWeight;
                totalBoneWeight += boneWeight;

                if (maxBoneWeight < boneWeight)
                    maxBoneWeight = boneWeight;
            }

            g_boneWeightVis[row][corner].s = 0;
            g_boneWeightVis[row][corner].t = 0;
            for (bone = 0; bone < kMaxBones; bone++)
            {
                g_boneWeights[row][corner][bone] = boneWeights[bone] / totalBoneWeight;

//              printf("%d %d %d\n", bone, bone & 1, (bone+1) & 1);
                if (bone & 1) g_boneWeightVis[row][corner].s += g_boneWeights[row][corner][bone]; // Copy data to here to visualize your weights or anything else
                if ((bone+1) & 1) g_boneWeightVis[row][corner].t += g_boneWeights[row][corner][bone]; // Copy data to here to visualize your weights
//              printf("%d %f\n", bone, g_boneWeights[row][corner][bone]);
            }

            // Visar vertexraderna
//          g_boneWeightVis[row][corner].s = row & 1; // Copy data to here to visualize your weights or anything else
//          g_boneWeightVis[row][corner].t = (row+1) & 1; // Copy data to here to visualize your weights
        }

    corner = 0;
    for (row = 0; row < kMaxRow; row++)
//      for (corner = 0; corner < kMaxCorners; corner++)
            for (bone = 0; bone < kMaxBones; bone++)
            {
//              printf("%d %d %f\n", row, bone, g_boneWeights[row][corner][bone]);
            }

}



///////////////////////////////////////////////////
//      B U I L D  C Y L I N D E R
// Desc:  bygger upp cylindern
//
void BuildCylinder()
{
  long  row, corner, cornerIndex;

  // s�tter v�rden till alla vertexar i meshen
  for (row = 0; row < kMaxRow; row++)
    for (corner = 0; corner < kMaxCorners; corner++)
      {
          g_vertsOrg[row][corner].x = (float) row * CYLINDER_SEGMENT_LENGTH;
          g_vertsOrg[row][corner].y = cos(corner * 2*Pi / kMaxCorners);
          g_vertsOrg[row][corner].z = sin(corner * 2*Pi / kMaxCorners);

          g_normalsOrg[row][corner].x = 0;
          g_normalsOrg[row][corner].y = cos(corner * 2*Pi / kMaxCorners);
          g_normalsOrg[row][corner].z = sin(corner * 2*Pi / kMaxCorners);
      };

  // g_poly definerar mellan vilka vertexar som
  // trianglarna ska ritas
  for (row = 0; row < kMaxRow-1; row++)
    for (corner = 0; corner < kMaxCorners; corner++)
      {
    // Quads built from two triangles

    if (corner < kMaxCorners-1)
      {
        cornerIndex = row * kMaxCorners + corner;
        g_poly[cornerIndex * 2].v1 = cornerIndex;
        g_poly[cornerIndex * 2].v2 = cornerIndex + 1;
        g_poly[cornerIndex * 2].v3 = cornerIndex + kMaxCorners + 1;

        g_poly[cornerIndex * 2 + 1].v1 = cornerIndex;
        g_poly[cornerIndex * 2 + 1].v2 = cornerIndex + kMaxCorners + 1;
        g_poly[cornerIndex * 2 + 1].v3 = cornerIndex + kMaxCorners;
      }
    else
      { // Specialfall: sista i varvet, g�u runt h�rnet korrekt
        cornerIndex = row * kMaxCorners + corner;
        g_poly[cornerIndex * 2].v1 = cornerIndex;
        g_poly[cornerIndex * 2].v2 = cornerIndex + 1 - kMaxCorners;
        g_poly[cornerIndex * 2].v3 = cornerIndex + kMaxCorners + 1 - kMaxCorners;

        g_poly[cornerIndex * 2 + 1].v1 = cornerIndex;
        g_poly[cornerIndex * 2 + 1].v2 = cornerIndex + kMaxCorners + 1 - kMaxCorners;
        g_poly[cornerIndex * 2 + 1].v3 = cornerIndex + kMaxCorners;
      }
      }

  // l�gger en kopia av orginal modellen i g_vertsRes
  memcpy(g_vertsRes,  g_vertsOrg, kMaxRow * kMaxCorners* sizeof(vec3));
  memcpy(g_normalsRes,  g_normalsOrg, kMaxRow * kMaxCorners* sizeof(vec3));
}


//////////////////////////////////////
//      B O N E
// Desc:    en enkel ben-struct med en
//          pos-vektor och en rot-vektor
//          rot vektorn skulle lika g�rna
//          kunna vara av 3x3 men VectorUtils2 har bara 4x4
typedef struct Bone
{
  vec3 pos;
  mat4 rot;
} Bone;


///////////////////////////////////////
//      G _ B O N E S
// v�rt skelett
Bone g_bones[kMaxBones]; // Ursprungsdata, �ndra ej
Bone g_bonesRes[kMaxBones]; // Animerat


///////////////////////////////////////////////////////
//      S E T U P  B O N E S
//
void setupBones(void)
{
    int bone;

  for (bone = 0; bone < kMaxBones; bone++)
  {
    g_bones[bone].pos = SetVector((float) bone * BONE_LENGTH, 0.0f, 0.0f);
    g_bones[bone].rot = IdentityMatrix();
  }
}


float anim_angle;

///////////////////////////////////////////////////////
//      D E F O R M  C Y L I N D E R
//
// Desc:    deformera cylinder meshen enligt skelettet
void DeformCylinder()
{
    //vec3 v[kMaxBones];

    //float w[kMaxBones];
    int row, corner;
    mat4 inv_M[kMaxBones], Mbone[kMaxBones];

    for (int bone = 0; bone < kMaxBones; bone++)
    {
        vec3 rel_translate;
        if (bone == 0)
        {
            rel_translate = g_bones[0].pos;
        } else {
            rel_translate = VectorSub(g_bones[bone].pos, g_bones[bone-1].pos);
        }



        mat4 TboneModel = T(rel_translate.x, rel_translate.y, rel_translate.z);
        inv_M[bone] = InvertMat4(TboneModel);

        mat4 RboneWorld = g_bonesRes[bone].rot;
        mat4 Mbone_tmp = Mult(RboneWorld, TboneModel);
        Mbone[bone] = Mbone_tmp;

    }
    // f�r samtliga vertexar

    mat4 m_mb[kMaxBones], m_bm[kMaxBones];
    for (int bone = 0; bone < kMaxBones; bone++)
    {
        mat4 temp_m = Mbone[0];
        mat4 temp_inv = inv_M[0];

        for (int i = 1; i <= bone; i++)
        {
            temp_m = Mult(temp_m,Mbone[i]);
            temp_inv = Mult(inv_M[i],temp_inv);
        }
        m_mb[bone] = temp_inv;
                //printf("%f\n", temp_inv.m[3]);
        m_bm[bone] = temp_m;

    }

    for (row = 0; row < kMaxRow; row++)
    {
        for (corner = 0; corner < kMaxCorners; corner++)
        {

            vec3 temp_vert=SetVector(0,0,0);
                        //vec3 temp_vert;   <-------  inte bra
                        //int mat = row % 10;
                        //printf("%i\n", mat);
                        //mat4 mat_bone = Mult(m_bm[mat], m_mb[mat]);
             for (int bone = 0; bone < kMaxBones; bone++)
             {
                             //if (row == 50){printf("%f\n", g_boneWeights[row][corner][bone]);}
                             mat4 mat_bone = Mult(m_bm[bone], m_mb[bone]);
               temp_vert = VectorAdd(temp_vert,(ScalarMult(MultVec3(mat_bone,g_vertsOrg[row][corner]),g_boneWeights[row][corner][bone])));

             }
                        //temp_vert = MultVec3(mat_bone,g_vertsOrg[row][corner]);
            g_vertsRes[row][corner] = temp_vert;
        }
    }
}


/////////////////////////////////////////////
//      A N I M A T E  B O N E S
// Desc:    en v�ldigt enkel animation av skelettet
//          vrider ben 1 i en sin(counter)
void animateBones(void)
{
    int bone;
    // Hur mycket kring varje led? �ndra g�rna.
    // float angleScales[10] = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };
    // float angleScales[10] = { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 };
    // float angleScales[10] = { 0.1, 0.2, 0.3, 0.4, 0.5, -0.5, -0.4, -0.3, -5, -10 };
    float angleScales[10] = { 2, 0.0, 0.0, 0.0, -4, 0.0, 0.0, 0.0, 0.0, 0.0 };
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    // Hur mycket skall vi vrida?

    float angle = 0.2*sin(time * 3.f) / 2.0f;
    anim_angle = angle;
    memcpy(&g_bonesRes, &g_bones, kMaxBones*sizeof(Bone));

    for (bone = 0; bone < kMaxBones; bone++)
  {
    g_bonesRes[bone].rot = Mult(g_bonesRes[bone].rot, Rz(angle * angleScales[bone]));
  }
}


///////////////////////////////////////////////
//      S E T  B O N E  R O T A T I O N
// Desc:    s�tter bone rotationen i vertex shadern
// (Ej obligatorisk.)
void setBoneRotation(void)
{
}


///////////////////////////////////////////////
//       S E T  B O N E  L O C A T I O N
// Desc:    s�tter bone positionen i vertex shadern
// (Ej obligatorisk.)
void setBoneLocation(void)
{
}


///////////////////////////////////////////////
//       D R A W  C Y L I N D E R
// Desc:    s�tter bone positionen i vertex shadern
void DrawCylinder()
{
  animateBones();

  // ---------=========  UPG 2 (extra) ===========---------
  // ers�tt DeformCylinder med en vertex shader som g�r vad DeformCylinder g�r.
  // begynelsen till shaderkoden ligger i filen "ShaderCode.vert" ...
  //

  DeformCylinder();

  // setBoneLocation();
  // setBoneRotation();

// update cylinder vertices:
    glBindVertexArray(cylinderModel->vao);
    glBindBuffer(GL_ARRAY_BUFFER, cylinderModel->vb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*kMaxRow*kMaxCorners, g_vertsRes, GL_DYNAMIC_DRAW);

    DrawModel(cylinderModel, g_shader, "in_Position", "in_Normal", "in_TexCoord");
}


void DisplayWindow()
{
    mat4 m;

  glClearColor(0.4, 0.4, 0.2, 1);
  glClear(GL_COLOR_BUFFER_BIT+GL_DEPTH_BUFFER_BIT);

    m = Mult(projectionMatrix, modelViewMatrix);
    glUniformMatrix4fv(glGetUniformLocation(g_shader, "matrix"), 1, GL_TRUE, m.m);

  DrawCylinder();

  glutSwapBuffers();
};

void OnTimer(int value)
{
  glutPostRedisplay();
  glutTimerFunc(20, &OnTimer, value);
}

void keyboardFunc( unsigned char key, int x, int y)
{
  if(key == 27) //Esc
    exit(1);
}

void reshape(GLsizei w, GLsizei h)
{
    vec3 cam = {16,0,30};
    vec3 look = {16,0,0};

    glViewport(0, 0, w, h);
    GLfloat ratio = (GLfloat) w / (GLfloat) h;
    projectionMatrix = perspective(90, ratio, 0.1, 1000);
    modelViewMatrix = lookAt(cam.x, cam.y, cam.z,
                                            look.x, look.y, look.z,
                                            0,1,0);
}

/////////////////////////////////////////
//      M A I N
//
int main(int argc, char **argv)
{
  glutInit(&argc, argv);

  glutInitWindowSize(512, 512);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitContextVersion(3, 2); // Might not be needed in Linux
  glutCreateWindow("Them bones, them bones");

  glutDisplayFunc(DisplayWindow);
  glutTimerFunc(20, &OnTimer, 0);
  glutKeyboardFunc( keyboardFunc );
    glutReshapeFunc(reshape);

  // Set up depth buffer
  glEnable(GL_DEPTH_TEST);

  // initiering
#ifdef WIN32
  glewInit();
#endif
  BuildCylinder();
  setupBones();
  initBoneWeights();

    // Build Model from cylinder data
    cylinderModel = LoadDataToModel(
            (GLfloat*) g_vertsRes,
            (GLfloat*) g_normalsRes,
            (GLfloat*) g_boneWeightVis, // texCoords
            NULL, // (GLfloat*) g_boneWeights, // colors
            (GLuint*) g_poly, // indices
            kMaxRow*kMaxCorners,
            kMaxg_poly * 3);

  g_shader = loadShaders("shader0.vert" , "shader0.frag");

  glutMainLoop();
  exit(0);
}
