UNAME = $(shell uname)

CC = gcc -Wall -O2

BINARY = candlestick

SOURCE = app.c display.c fnt.c frame.c dlist.c

ifeq ($(UNAME), Darwin)
	LDFLAGS	= -framework Cocoa -framework OpenGL
	LDFLAGS += -lbz2
	SOURCE += nibless.m
endif

ifeq ($(UNAME), Linux)
	LDFLAGS = -lX11 -lGL -lGLU
	SOURCE += main.c
endif

LDFLAGS += -L./lib -lm -lfreetype -lz

CFLAGS = `freetype-config --cflags`

all:
	$(CC) $(SOURCE) -o $(BINARY) $(CFLAGS) $(LDFLAGS)

clean:
	@echo Cleaning up...
	@rm $(BINARY)
	@echo Done.
