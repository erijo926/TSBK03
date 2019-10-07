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
#include "zpr.h"
#include <string.h>

//Reference to shader program
GLuint program;

//Model and texture vars
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

Model *squareModel;
// Point3D cam, point;
// mat4 viewMatrix;

//Globals, most are sent to shader
int prevx = 0, prevy = 0;
float px = 0, py = 0;
float dist = 3;
GLfloat camera[] = {10.0,10.0,10.0};
GLfloat camDist = 3.0;

void init(void)
{
    // GL inits
    glClearColor(1.0,0.0,0.0,0);
    glEnable(GL_DEPTH_TEST);
    printError("GL inits");

    // Load and compile shader
    program = loadShaders("ray.vert", "ray.frag");
    glUseProgram(program);
    printError("init shader");

    squareModel = LoadDataToModel(
			square, NULL, squareTexCoord, NULL,
			squareIndices, 4, 6);
    // glUniform3fv(glGetUniformLocation(program, "sentCam"), 1, camera);

    // cam = SetVector(0, 5, 15);
    // point = SetVector(0, 1, 0);
    // zprInit(&viewMatrix, cam, point);
}

//Executes upon drag
void mouseUpDown(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        prevx = x;
        prevy = y;
    }
}

//Actual movement of the mouse
void mouseDragged(int x, int y)
{
    py = x-prevx;
    px = -(prevy-y);

    vec3 loc = {camDist*sin(py*(M_PI/180))*cos(px*(M_PI/180)),
                camDist*sin(px*(M_PI/180)),
                camDist*cos(py*(M_PI/180))*cos(px*(M_PI/180))};

    camera[0] = 10.0+loc.x;
    camera[1] = 10.0+loc.y;
    camera[2] = 10.0+loc.z;
    // glUniform3fv(glGetUniformLocation(program, "sentCam"), 1, camera);
    glutPostRedisplay();
}

void display(void)
{
    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //Timer, mainly testing
    GLfloat time = 0.001*(GLfloat)glutGet(GLUT_ELAPSED_TIME);
    glUniform1f(glGetUniformLocation(program, "time"), time);
    DrawModel(squareModel, program, "inPosition", NULL, NULL);
    glutSwapBuffers();
}

void timer(int i)
{
    glutTimerFunc(20, &timer, i);
    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(600,600);
    glutInitContextVersion(3,2);
    glutCreateWindow(":^)");
    glutDisplayFunc(display);
    glutMouseFunc(mouseUpDown);
    glutMotionFunc(mouseDragged);
    glutTimerFunc(20, &timer, 0);
    init();
    glutMainLoop();
    exit(0);
}
