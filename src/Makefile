TOPDIR = ..

include $(TOPDIR)/config

CC = gcc -Wall -O2

ifdef DEBUG
	CC += -g
endif

COMMON_SRC = \
  app.c \
  disp.c \
  fnt.c \
  frame.c \
  dlist.c \
  utils.c \
  $(NULL)

COMMON_LIBS = -lm

ifeq ($(PLAT),win32)
	OS_LIBS = -luser32 -lgdi32
	GL_LIBS = -lopengl32 -lglu32
	FT_LIBS = $(LIBDIR)/freetype.lib
	MAIN_SRC = main-win32.c
else ifeq ($(PLAT),mac)
	OS_LIBS	= -framework Cocoa
	GL_LIBS = -framework OpenGL
	FT_LIBS = $(LIBDIR)/libfreetype.a $(LIBDIR)/libz.a $(LIBDIR)/libbz2.a
	MAIN_SRC = main-mac.m
else ifeq ($(PLAT),nix)
	OS_LIBS = -lX11
	GL_LIBS = -lGL -lGLU
	FT_LIBS = $(LIBDIR)/libfreetype.a $(LIBDIR)/libz.a
	MAIN_SRC = main-nix.c
endif

SOURCE = $(COMMON_SRC) $(MAIN_SRC)
LDFLAGS = $(COMMON_LIBS) $(OS_LIBS) $(GL_LIBS) $(FT_LIBS)
CFLAGS = -I$(TOPDIR)/freetype -I$(TOPDIR)/freetype/freetype2

all:
	$(CC) $(SOURCE) -o $(TOPDIR)/$(BINARY) $(CFLAGS) $(LDFLAGS)
