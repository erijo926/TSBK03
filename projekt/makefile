# set this variable to the director in which you saved the common files
commondir = ./common/

all : main

main : main.c $(commondir)GL_utilities.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)Mac/MicroGlut.m
	gcc -Wall -o main -I$(commondir) -I$(commondir)Mac -DGL_GLEXT_PROTOTYPES main.c $(commondir)GL_utilities.c $(commondir)loadobj.c $(commondir)VectorUtils3.c $(commondir)Mac/MicroGlut.m -framework OpenGL -framework Cocoa -lm

#main : main.c pvm.c $(commondir)GL_utilities.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
#	gcc -o main -I$(commondir) -I../common/Linux -DGL_GLEXT_PROTOTYPES main.c pvm.c $(commondir)GL_utilities.c $(commondir)loadobj.c $(commondir)VectorUtils3.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

clean :
	rm main
