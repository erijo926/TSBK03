// Lab 1-1, multi-pass rendering with FBOs and HDR.
// Revision for 2013, simpler light, start with rendering on quad in final stage.
// Switched to low-res Stanford Bunny for more details.
// No HDR is implemented to begin with. That is your task.

// You can compile like this:
// gcc lab1-1.c ../common/*.c -lGL -o lab1-1 -I../common

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifdef __APPLE__
// Mac
	#include <OpenGL/gl3.h>
	#include "MicroGlut.h"
	// uses framework Cocoa
#else
	#ifdef WIN32
// MS
		#include <windows.h>
		#include <stdio.h>
		#include <GL/glew.h>
		#include <GL/glut.h>
	#else
// Linux
		#include <stdio.h>
		#include <GL/gl.h>
		#include "MicroGlut.h"
//		#include <GL/glut.h>
	#endif
#endif

#include "VectorUtils3.h"
#include "GL_utilities.h"
#include "loadobj.h"
#include "zpr.h"

// initial width and heights
#define W 512
#define H 512

#define NUM_LIGHTS 4

void OnTimer(int value);

mat4 projectionMatrix;
mat4 viewMatrix;


GLfloat square[] = {
							-1,-1,0,
							-1,1, 0,
							1,1, 0,
							1,-1, 0};
GLfloat squareTexCoord[] = {
							 0, 0,
							 0, 1,
							 1, 1,
							 1, 0};
GLuint squareIndices[] = {0, 1, 2, 0, 2, 3};

Model* squareModel;

//----------------------Globals-------------------------------------------------
Point3D cam, point;
Model *model1;
FBOstruct *fbo1, *fbo2;
GLuint phongshader = 0, plaintextureshader = 0;

//-------------------------------------------------------------------------------------

void init(void)
{
	dumpInfo();  // shader info

	// GL inits
	glClearColor(0.1, 0.1, 0.3, 0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	printError("GL inits");

	// Load and compile shaders
	plaintextureshader = loadShaders("plaintextureshader.vert", "plaintextureshader.frag");  // puts texture on teapot
	phongshader = loadShaders("phong.vert", "phong.frag");  // renders with light (used for initial renderin of teapot)

	printError("init shader");

	fbo1 = initFBO(W, H, 0);
	fbo2 = initFBO(W, H, 0);

	// load the model
//	model1 = LoadModelPlus("teapot.obj");
	model1 = LoadModelPlus("stanford-bunny.obj");

	squareModel = LoadDataToModel(
			square, NULL, squareTexCoord, NULL,
			squareIndices, 4, 6);

	cam = SetVector(0, 5, 15);
	point = SetVector(0, 1, 0);

	glutTimerFunc(5, &OnTimer, 0);

	zprInit(&viewMatrix, cam, point);
}

void OnTimer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(5, &OnTimer, value);
}

void runfilter(GLuint shader, FBOstruct *in1, FBOstruct *in2, FBOstruct *out)
{
    glUseProgram(shader);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glUniform1i(glGetUniformLocation(shader, "texUnit"), 0);
    glUniform1i(glGetUniformLocation(shader, "texUnit2"), 1);
    useFBO(out, in1, in2);
    DrawModel(squareModel, shader, "in_Position", NULL, "in_TexCoord");
    glFlush();
}

//-------------------------------callback functions------------------------------------------
void display(void)
{
	mat4 vm2;

	// This function is called whenever it is time to render
	//  a new frame; due to the idle()-function below, this
	//  function will get called several times per second

	// render to fbo1!
    // out in in
	useFBO(fbo1, 0L, 0L);

	// Clear framebuffer & zbuffer
	glClearColor(0.1, 0.1, 0.3, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate shader program
	glUseProgram(phongshader);

	vm2 = viewMatrix;
	// Scale and place bunny since it is too small
	vm2 = Mult(vm2, T(0, -8.5, 0));
	vm2 = Mult(vm2, S(80,80,80));

	glUniformMatrix4fv(glGetUniformLocation(phongshader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(phongshader, "modelviewMatrix"), 1, GL_TRUE, vm2.m);
	glUniform3fv(glGetUniformLocation(phongshader, "camPos"), 1, &cam.x);
	glUniform1i(glGetUniformLocation(phongshader, "texUnit"), 0);


    GLfloat t = 0.001*(GLfloat)glutGet(GLUT_ELAPSED_TIME);

	// Enable Z-buffering
	glEnable(GL_DEPTH_TEST);
	// Enable backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	DrawModel(model1, phongshader, "in_Position", "in_Normal", NULL);

	// Done rendering the FBO! Set up for rendering on screen, using the result as texture!

//	glFlush(); // Can cause flickering on some systems. Can also be necessary to make drawing complete.
    useFBO(0L, fbo1, 0L);
    glClearColor(0.0, 0.0, 0.0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate second shader program
    glUseProgram(plaintextureshader);
    runfilter(plaintextureshader, fbo1, 0L, fbo2);
    if (t>3)
    {
        runfilter(plaintextureshader, fbo2, 0L, fbo1);
    }
    /*for(int i=0; i <3; ++i)
    {
        if (i % 2 == 0)
        {
            runfilter(plaintextureshader, fbo1, 0L, fbo2);
        }else{
            runfilter(plaintextureshader, fbo2, 0L, fbo1);
        }
    }*/

    useFBO(0L, fbo1, 0L);
    DrawModel(squareModel, plaintextureshader, "in_Position", NULL, "in_TexCoord");

	glutSwapBuffers();
}


void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	GLfloat ratio = (GLfloat) w / (GLfloat) h;
	projectionMatrix = perspective(90, ratio, 1.0, 1000);
}


// This function is called whenever the computer is idle
// As soon as the machine is idle, ask GLUT to trigger rendering of a new
// frame
void idle()
{
  glutPostRedisplay();
}

//-----------------------------main-----------------------------------------------
int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(W, H);

	glutInitContextVersion(3, 2);
	glutCreateWindow ("Render to texture with FBO");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	init();
	glutMainLoop();
	exit(0);
}
