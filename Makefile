###########################################################
#
# Makefile for candlestick app
#
# Copyright 2013 Thomas Klemz
#
# Licensed under GPLv3 or later at your choice.
# You are free to use, distribute, and modify this
# as long as you grant the same freedoms to others.
# See the file COPYING for the full license.
###########################################################

BINARY = candlestick
ARCHIVE = candlestick.tar.bz2

CC = gcc -Wall -O2

ifdef DEBUG
	CC += -g
endif

ifeq ($(OS),Windows_NT)
	PLAT = win32
else
	UNAME = $(shell uname)
	PLAT = nix
	
	ifeq ($(UNAME),Darwin)
		PLAT = mac
	endif
endif

SRCDIR = src
LIBDIR = lib/$(PLAT)
RESDIR = res
OUTDIR = package
MACAPP = $(BINARY).app

COMMON_SRC = \
  app.c \
  disp.c \
  fnt.c \
  frame.c \
  dlist.c \
  utils.c \
  $(NULL)

FREETYPE_INC = -I$(SRCDIR)/freetype -I$(SRCDIR)/freetype/freetype2

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

SOURCE = $(addprefix $(SRCDIR)/, $(COMMON_SRC) $(MAIN_SRC))
LDFLAGS = $(COMMON_LIBS) $(OS_LIBS) $(GL_LIBS) $(FT_LIBS)
CFLAGS = $(FREETYPE_INC)

.PHONY: all
all: $(BINARY)

$(BINARY):
	@echo "\nCompiling sources in '$(SRCDIR)'...\n"
	$(CC) $(SOURCE) -o $(BINARY) $(CFLAGS) $(LDFLAGS)
	@echo "\n...Done building."

.PHONY: package-mac
package-mac: $(BINARY) $(RESDIR)/mac/* $(RESDIR)/common/*
	@echo "\nPackaging $(BINARY)..."
	@mkdir -p $(OUTDIR)
	@cp -Ra $(RESDIR)/mac/$(MACAPP) $(OUTDIR)
	@cp -a $(BINARY) $(OUTDIR)/$(MACAPP)/Contents/MacOS
	@cp -Ra $(RESDIR)/common/font $(OUTDIR)/$(MACAPP)/Contents/MacOS
	@tar -cjf $(OUTDIR)/$(ARCHIVE) $(OUTDIR)/$(MACAPP)
	@echo "Packaged $(BINARY) into $(OUTDIR)/$(MACAPP)\nArchived into $(OUTDIR)/$(ARCHIVE)"

.PHONY: package
package: package-$(PLAT)

.PHONY: run
run: package
	@echo "Running $(BINARY)..."
	@open $(OUTDIR)/$(MACAPP)

.PHONY: clean
clean:
	@echo Cleaning up...
	-@rm $(BINARY) 2> /dev/null || echo "There was no binary..."
	-@rm -r $(OUTDIR) 2> /dev/null || echo "There was no archive..."
	@echo Done.
