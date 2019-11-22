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

all :  lab1-1 pingpong

lab1-1 : lab1-1.c $(commondir)GL_utilities.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)zpr.c $(commondir)$(osdir)/MicroGlut$(glutext)
	gcc -Wall -o lab1-1 -I$(commondir) -I$(commondir)$(osdir) -DGL_GLEXT_PROTOTYPES lab1-1.c $(commondir)GL_utilities.c $(commondir)loadobj.c $(commondir)zpr.c $(commondir)VectorUtils3.c $(commondir)$(osdir)/MicroGlut$(glutext) $(glflags)

pingpong : lab1-1pingpong.c $(commondir)GL_utilities.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)zpr.c $(commondir)$(osdir)/MicroGlut$(glutext)
	gcc -Wall -o pingpong -I$(commondir) -I$(commondir)$(osdir) -DGL_GLEXT_PROTOTYPES lab1-1pingpong.c $(commondir)GL_utilities.c $(commondir)loadobj.c $(commondir)zpr.c $(commondir)VectorUtils3.c $(commondir)$(osdir)/MicroGlut$(glutext) $(glflags)

clean :
	rm lab1-1 pingpong
