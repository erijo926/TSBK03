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
GLfloat camera[] = {15.0,5.0,0.0};
GLfloat camDist = 15.0;
GLfloat ball_height = 0.5;
GLfloat drop_time;
GLboolean gravity = GL_FALSE;

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
    glUniform3fv(glGetUniformLocation(program, "cam_pos"), 1, camera);
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

    camera[0] = loc.x;
    camera[1] = loc.y; // loc.y
    camera[2] = loc.z;
    glUniform3fv(glGetUniformLocation(program, "cam_pos"), 1, camera);
    glutPostRedisplay();
}

float moveBall(float h)
{
    float change = 0.3;
    if (glutKeyIsDown('w')) {
        h += change;
    }
    if (glutKeyIsDown('s')) {
        h -= change;
    }
    return h;
}

void dropBall(float t)
{
    if (glutKeyIsDown('e')) {
        gravity = GL_FALSE;
        // if (gravity == GL_TRUE) {
        // }
        // else gravity = GL_TRUE;
    }
    if (glutKeyIsDown('d')) {
        drop_time = t;
        gravity = GL_TRUE;
    }
    if (glutKeyIsDown('r')) {
        ball_height = 0.0;
    }
}

void display(void)
{
    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //Timer, mainly for testing
    GLfloat time = 0.001*(GLfloat)glutGet(GLUT_ELAPSED_TIME);
    ball_height = moveBall(ball_height);
    dropBall(time);

    DrawModel(squareModel, program, "inPosition", NULL, NULL);
    glUniform1f(glGetUniformLocation(program, "time"), time);
    glUniform1f(glGetUniformLocation(program, "drop_time"), drop_time);
    glUniform1i(glGetUniformLocation(program, "gravity"), gravity);
    glUniform1f(glGetUniformLocation(program, "ball_height"), ball_height);
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
