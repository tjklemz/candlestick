CC = gcc -Wall -O1

SOURCE = nibless.m app.c display.c font.c frame.c dlist.c

# LDFLAGS	= -lGL -lglut -lGLU
# Mac OS alternate cmdline link options
# ifeq "$(OSTYPE)" "Darwin"
#	LDFLAGS	= -framework Carbon -framework OpenGL -framework GLUT
# endif

FLAGS = -lm -framework Cocoa -framework OpenGL `freetype-config --cflags` -L./lib -lfreetype -lz -lbz2

BINARY = candlestick

all:
	$(CC) $(SOURCE) -o $(BINARY) $(FLAGS)

clean:
	@echo Cleaning up...
	@rm $(BINARY)
	@echo Done.
