// Demo of heavily simplified sprite engine
// by Ingemar Ragnemalm 2009
// used as base for lab 4 in TSBK03.
// OpenGL 3 conversion 2013.

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#include "MicroGlut.h"
	// uses framework Cocoa
#else
	#include <GL/gl.h>
	#include "MicroGlut.h"
#endif

#include <stdlib.h>
#include <math.h>
#include "LoadTGA.h"
#include "SpriteLight.h"
#include "GL_utilities.h"

// L�gg till egna globaler h�r efter behov.

void SpriteBehavior() // Din kod!
{
    float max_dist = 300.0;
    float max_speed = 25.0;
    SpritePtr i = gSpriteRoot;
    do {
        int count = 0;
        SpritePtr j = gSpriteRoot;
        do {
            if (i != j)
            {
                float dist_h = j->position.h - i->position.h;
                float dist_v = j->position.v - i->position.v;
                float dist_norm = sqrt(dist_h*dist_h+dist_v*dist_v);

                if (dist_norm < max_dist)
                {
                    i->avg_position.h +=(j->position.h - i->position.h);
                    i->avg_position.v +=(j->position.v - i->position.v);
                    i->sep_position.h +=(i->position.h - j->position.h);
                    i->sep_position.v +=(i->position.v - j->position.v);
                    i->speed_diff.h += (j->speed.h- i->speed.h);
                    i->speed_diff.v += (j->speed.v- i->speed.v);

                    count += 1;
                }
            }

            j = j->next;
        } while (j != NULL);
        if (count != 0)
        {
            i->avg_position.h /= count;
            i->avg_position.v /= count;
            i->sep_position.h /= count;
            i->sep_position.v /= count;
            i->speed_diff.h /= count;
            i->speed_diff.v /= count;
        }
        i = i->next;
    } while (i != NULL);

    i = gSpriteRoot;
    float cohesion_weight = 0.001;
    float separation_weight = 0.0005;
    float align_weight = 0.01;
    float v, h;
    do {
        h = i->avg_position.h*cohesion_weight-i->sep_position.h*separation_weight;
        v = i->avg_position.v*cohesion_weight-i->sep_position.v*separation_weight;
        i->speed.h += h+i->speed_diff.h*align_weight;
        i->speed.v += v+i->speed_diff.h*align_weight;

        // if (i->speed.h > max_speed || i->speed.v > max_speed)
        // {
        //     i->speed.h = max_speed;
        //     i->speed.v = max_speed;
        //     printf("%f %f\n", i->speed.h,i->speed.v);
        // }

        i = i->next;
    } while (i != NULL);
}

// Drawing routine
void Display()
{
	SpritePtr sp;

	glClearColor(0, 0, 0.2, 1);
	glClear(GL_COLOR_BUFFER_BIT+GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	DrawBackground();

	SpriteBehavior(); // Din kod!

    // Loop though all sprites. (Several loops in real engine.)
	sp = gSpriteRoot;
	do
	{
		HandleSprite(sp); // Callback in a real engine
		DrawSprite(sp);
		sp = sp->next;
	} while (sp != NULL);

	glutSwapBuffers();
}

void Reshape(int h, int v)
{
	glViewport(0, 0, h, v);
	gWidth = h;
	gHeight = v;
}

void Timer(int value)
{
	glutTimerFunc(20, Timer, 0);
	glutPostRedisplay();
}

// Example of user controllable parameter
float someValue = 0.0;

void Key(unsigned char key,
         __attribute__((unused)) int x,
         __attribute__((unused)) int y)
{
  switch (key)
  {
    case '+':
    	someValue += 0.1;
    	printf("someValue = %f\n", someValue);
    	break;
    case '-':
    	someValue -= 0.1;
    	printf("someValue = %f\n", someValue);
    	break;
    case 0x1b:
      exit(0);
  }
}

void Init()
{
	TextureData *sheepFace, *blackFace, *dogFace, *foodFace;

	LoadTGATextureSimple("bilder/leaves.tga", &backgroundTexID); // Bakgrund

	sheepFace = GetFace("bilder/sheep.tga"); // Ett f�r
	blackFace = GetFace("bilder/blackie.tga"); // Ett svart f�r
	dogFace = GetFace("bilder/dog.tga"); // En hund
	foodFace = GetFace("bilder/mat.tga"); // Mat

	NewSprite(sheepFace, 100, 200, 1, 1);
	NewSprite(sheepFace, 200, 100, 1.5, -1);
	NewSprite(sheepFace, 250, 200, -1, 1.5);
    NewSprite(sheepFace, 200, 250, -1, 1.5);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutInitContextVersion(3, 2);
	glutCreateWindow("SpriteLight demo / Flocking");

	glutDisplayFunc(Display);
	glutTimerFunc(20, Timer, 0); // Should match the screen synch
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);

	InitSpriteLight();
	Init();

	glutMainLoop();
	return 0;
}
