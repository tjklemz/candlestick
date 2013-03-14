
ifeq ($(OS),Windows_NT)
	PLAT=win
else
	UNAME = $(shell uname)
	PLAT=nix
	
	ifeq ($(UNAME),Darwin)
		PLAT=mac
	endif
endif

LIBDIR = ./lib/$(PLAT)

CC = gcc -Wall -O2

ifdef DEBUG
	CC += -g
endif

BINARY = candlestick

SOURCE = \
  app.c \
  disp.c \
  fnt.c \
  frame.c \
  dlist.c \
  $(NULL)

LDFLAGS = $(NULL)

ifeq ($(PLAT),mac)
	LDFLAGS	+= -framework Cocoa -framework OpenGL
	LDFLAGS += $(LIBDIR)/libbz2.a
	SOURCE += nibless.m
else ifeq ($(PLAT),nix)
	LDFLAGS += -lX11 -lGL -lGLU
	SOURCE += main.c
endif

LDFLAGS += -lm $(LIBDIR)/libfreetype.a $(LIBDIR)/libz.a

CFLAGS = `freetype-config --cflags`

all:
	$(CC) $(SOURCE) -o $(BINARY) $(CFLAGS) $(LDFLAGS)

clean:
	@echo Cleaning up...
	@rm $(BINARY)
	@echo Done.
