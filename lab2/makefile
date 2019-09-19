# set this variable to the director in which you saved the common files
commondir = ../common/

all : lab2-1 lab2-2

UNAME_S := $(shell uname -s)
#ifeq ($(UNAME_S),Linux)
#ifeq ($(UNAME_S),Darwin)
#lab2-1 : skinning.c $(commondir)GL_utilities.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)Linux/MicroGlut.c
#	gcc -Wall -o skinning -I$(commondir) -I$(commondir)/Linux -DGL_GLEXT_PROTOTYPES skinning.c $(commondir)GL_utilities.c $(commondir)loadobj.c $(commondir)VectorUtils3.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

#lab2-2 : skinning2.c $(commondir)GL_utilities.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)Linux/MicroGlut.c
#	gcc -Wall -o skinning2 -I$(commondir) -I$(commondir)/Linux -DGL_GLEXT_PROTOTYPES skinning2.c $(commondir)GL_utilities.c $(commondir)loadobj.c $(commondir)VectorUtils3.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

lab2-1 : skinning.c $(commondir)GL_utilities.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)Mac/MicroGlut.m
	gcc -Wall -o skinning -I$(commondir) -I$(commondir)/Mac -DGL_GLEXT_PROTOTYPES skinning.c $(commondir)GL_utilities.c $(commondir)loadobj.c $(commondir)VectorUtils3.c $(commondir)Mac/MicroGlut.m -framework OpenGL -framework Cocoa -lm

lab2-2 : skinning2.c $(commondir)GL_utilities.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)Mac/MicroGlut.m
	gcc -Wall -o skinning2 -I$(commondir) -I$(commondir)/Mac -DGL_GLEXT_PROTOTYPES skinning2.c $(commondir)GL_utilities.c $(commondir)loadobj.c $(commondir)VectorUtils3.c $(commondir)Mac/MicroGlut.m -framework OpenGL -framework Cocoa -lm

clean :
	rm skinning skinning2
