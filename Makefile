######################################################################
#
# Makefile for candlestick app
#
# Copyright 2013 Thomas Klemz
#
# Licensed under GPLv3 or later at your choice.
# You are free to use, distribute, and modify this
# as long as you grant the same freedoms to others.
# See the file COPYING for the full license.
######################################################################

BINARY = candlestick
APPNAME = candlestick
ARCHIVE = $(APPNAME).tar.bz2

# Recommend gcc or anything compatible (such as Clang).
# Clang will compile much faster on the Mac,
#  since this program links to Cocoa.
# On Windows and Linux, the compile time is almost
#  negligible.
CC = cc -Wall -O2

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
APPDIR = $(OUTDIR)/$(APPNAME)app
MACAPP = $(BINARY).app
FONTDIR = $(RESDIR)/common/font

# It's better to list out the source files than glob them.
# This way, there can be other WIP files in the directory w/o issue,
# and there's no unnecessary compiling.
COMMON_SRC = \
  app.c \
  disp.c \
  fnt.c \
  frame.c \
  list.c \
  utils.c \
  utf8.c \
  rune.c \
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


######################################################################
# Compiling the app
######################################################################

.PHONY: all
all: $(BINARY)

$(BINARY): $(SOURCE)
	@echo "\nCompiling sources in '$(SRCDIR)'...\n"
	$(CC) $(SOURCE) -o $(BINARY) $(CFLAGS) $(LDFLAGS)
	@echo "\n...Done building."


######################################################################
# Packaging the app for distribution
######################################################################

.PHONY: package-mac
package-mac: package-common $(RESDIR)/mac/*
	@cp -Ra $(RESDIR)/mac/$(MACAPP) $(APPDIR)
	@cp -a $(BINARY) $(APPDIR)/$(MACAPP)/Contents/MacOS
	@cp -Ra $(FONTDIR) $(APPDIR)/$(MACAPP)/Contents/MacOS
	@tar -cjf $(OUTDIR)/$(ARCHIVE) $(APPDIR)/$(MACAPP)
	@echo "Packaged $(APPNAME) into $(APPDIR)/$(MACAPP)\n"
	@echo "Archived into $(OUTDIR)/$(ARCHIVE)"

.PHONY: package-nix
package-nix: package-common $(RESDIR)/nix/*
	@cp -a $(BINARY) $(APPDIR)
	@cp -Ra $(FONTDIR) $(APPDIR)
	@echo "Packaged $(APPNAME) into $(APPDIR)."

.PHONY: package-common
package-common: $(BINARY) $(RESDIR)/common/*
	@echo "\nPackaging $(APPNAME)..."
	@mkdir -p $(APPDIR)

.PHONY: package
package: package-$(PLAT)
	@echo "\nDone packaging. See the $(APPDIR) folder.\n"


######################################################################
# Running the app
######################################################################

.PHONY: run-mac
run-mac: package
	@open $(APPDIR)/$(MACAPP)
	
.PHONY: run-nix
run-nix: package
	@cd $(APPDIR) && ./$(BINARY)

.PHONY: run
run: run-$(PLAT)
	@echo "...Finished running $(APPNAME)."


######################################################################
# Maintenance (janitorial work)
######################################################################

.PHONY: clean
clean:
	@echo Cleaning up...
	-@rm $(BINARY) 2> /dev/null || echo "There was no binary..."
	-@rm -r $(OUTDIR) 2> /dev/null || echo "There was no package..."
	@echo Done.
