# set this variable to the director in which you saved the common files

commondir = ../common/

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	osdir = Mac/
	glutext = .m
	glflags = -framework OpenGL -framework Cocoa -lm
endif
ifeq ($(UNAME_S),Linux)
	osdir = Linux/
	glutext = .c
	glflags = -lXt -lX11 -lGL -lm
endif

all : main

main : main.c $(commondir)GL_utilities.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)$(osdir)/MicroGlut$(glutext)
	gcc -Wall -o main -I$(commondir) -I$(commondir)$(osdir) -DGL_GLEXT_PROTOTYPES main.c $(commondir)GL_utilities.c $(commondir)loadobj.c $(commondir)VectorUtils3.c $(commondir)$(osdir)/MicroGlut$(glutext) $(glflags)

clean :
	rm main
