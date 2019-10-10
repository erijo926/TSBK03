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
TextureData *sheepFace, *blackFace, *dogFace, *foodFace;

void SpriteBehavior() // Din kod!
{
    float coh_dist = 100.0;
    float sep_dist = 50.0;
    SpritePtr i = gSpriteRoot;

    do {
        i->avg_position.h = 0;
        i->avg_position.v = 0;
        i->sep_position.h = 0;
        i->sep_position.v = 0;
        i->speed_diff.h = 0;
        i->speed_diff.v = 0;

        int count = 0;
        SpritePtr j = gSpriteRoot;
        do {
            if (i != j)
            {
                float dist_h = j->position.h - i->position.h;
                float dist_v = j->position.v - i->position.v;
                float dist_norm = sqrt(dist_h*dist_h+dist_v*dist_v);
                if (dist_norm < coh_dist)
                {
                    i->avg_position.h +=(j->position.h - i->position.h);
                    i->avg_position.v +=(j->position.v - i->position.v);
                    i->speed_diff.h += (j->speed.h- i->speed.h);
                    i->speed_diff.v += (j->speed.v- i->speed.v);

                    count += 1;
                }

                if (j->face == dogFace && dist_norm < 200.0)
                {
                    // i->dogDistance = dist_norm;
                    i->sep_position.h +=(i->position.h - j->position.h)*(1+(1-dist_norm/200.0)*20.0);
                    i->sep_position.v +=(i->position.v - j->position.v)*(1+(1-dist_norm/200.0)*20.0);
                }
                else if (dist_norm < sep_dist)
                {
                    i->sep_position.h +=(i->position.h - j->position.h)*(1+(1-dist_norm/sep_dist)*10.0);
                    i->sep_position.v +=(i->position.v - j->position.v)*(1+(1-dist_norm/sep_dist)*10.0);
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
    float cohesion_weight = 0.0005;
    float align_weight = 0.1;
    float sep_weight = 0.01;
    float v, h;
    do {

        float sep_norm = sqrt(i->sep_position.h*i->sep_position.h + i->sep_position.v *i->sep_position.v);
        h = sep_weight*(i->sep_position.h);//(sep_norm+0.00001);
        v = sep_weight*(i->sep_position.v);//(sep_norm+0.00001);
        h += i->avg_position.h*cohesion_weight ;
        v += i->avg_position.v*cohesion_weight ;
        i->speed.h += h+i->speed_diff.h*align_weight;
        i->speed.v += v+i->speed_diff.h*align_weight;

        float v_norm = sqrt(i->speed.h *i->speed.h + i->speed.v *i->speed.v );

        // if (i->dogDistance < 100.0 && i->dogDistance > 1.0){
        //     i->speed.h += 0.1 * (1+(1-i->dogDistance/100)*2);
        //     i->speed.v += 0.1 * (1+(1-i->dogDistance/100)*2);
        // }

        if (v_norm < 3.0) {
            i->speed.h *= 3.0/v_norm;
            i->speed.v *= 3.0/v_norm;
        }
        if (v_norm > 4.0) {
            i->speed.h *= 4.0/v_norm;
            i->speed.v *= 4.0/v_norm;
        }

        if (i->face == blackFace)
        {
            float rnd = (random()%100+50)*0.02;
            i->speed.v *= rnd;
            i->speed.h *= rnd;
        }

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

void mouse_dragged(int x, int y)
{
    gSpriteRoot->position.h = x;
    gSpriteRoot->position.v = 600.0-y;
}

void Init()
{

	LoadTGATextureSimple("bilder/leaves.tga", &backgroundTexID); // Bakgrund

	sheepFace = GetFace("bilder/sheep.tga"); // Ett f�r
	blackFace = GetFace("bilder/blackie.tga"); // Ett svart f�r
	dogFace = GetFace("bilder/dog.tga"); // En hund
	foodFace = GetFace("bilder/mat.tga"); // Mat

    for (int i = 0; i < 100; i++)
    {
        NewSprite(sheepFace, random()%500, random()%500, 1, 1);
    }

    NewSprite(blackFace, 300, 300, -1, 1.5);
    NewSprite(dogFace, 500, 500, -1, 1.5);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutInitContextVersion(3, 2);
	glutCreateWindow("SpriteLight demo / Flocking");
    glutMotionFunc(mouse_dragged);
	glutDisplayFunc(Display);
	glutTimerFunc(20, Timer, 0); // Should match the screen synch
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);

	InitSpriteLight();
	Init();

	glutMainLoop();
	return 0;
}
