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
	BINARY = $(APPNAME).exe
else
	UNAME = $(shell uname)
	PLAT = nix
	
	ifeq ($(UNAME),Darwin)
		PLAT = mac
	endif
endif

# Mac OS X now requires 64-bit.
# Everything else though should be 32-bit for compatibility.
ifneq ($(PLAT),mac)
	CC += -m32
endif

SRCDIR = src
LIBDIR = lib/$(PLAT)
RESDIR = res
OUTDIR = package
APPDIR = $(APPNAME)app
MACAPP = $(APPNAME).app
FONTDIR = $(RESDIR)/common/font

# It's better to list out the source files than glob them.
# This way, there can be other WIP files in the directory w/o issue,
# and there's no unnecessary compiling.
SRC = \
  app.c \
  disp.c \
  fnt.c \
  line.c \
  frame.c \
  list.c \
  utils.c \
  utf8.c \
  rune.c \
  scroll.c \
  $(NULL)

FREETYPE_INC = -I$(SRCDIR)/freetype -I$(SRCDIR)/freetype/freetype2

COMMON_LIBS = -lm

ifeq ($(PLAT),win32)
	OS_LIBS = -luser32 -lgdi32 -lkernel32
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
	FT_LIBS = $(LIBDIR)/libfreetype.a $(LIBDIR)/libz.a $(LIBDIR)/libbz2.a
	MAIN_SRC = main-nix.c
	SRC += keysym2ucs.c
endif

SOURCE = $(addprefix $(SRCDIR)/, $(SRC) $(MAIN_SRC))
LDFLAGS = $(COMMON_LIBS) $(OS_LIBS) $(GL_LIBS) $(FT_LIBS)
CFLAGS = $(FREETYPE_INC)


######################################################################
# Compiling the app
######################################################################

.PHONY: all
all: $(BINARY)

$(BINARY): $(SOURCE)
	@echo
	@echo "Compiling sources in '$(SRCDIR)'..."
	@echo
	$(CC) $(SOURCE) -o $(BINARY) $(CFLAGS) $(LDFLAGS)
	@echo
	@echo "...Done building."


######################################################################
# Packaging the app for distribution
######################################################################

.PHONY: package-mac
package-mac: package-common $(RESDIR)/mac/*
	@cp -Ra $(RESDIR)/mac/$(MACAPP) $(OUTDIR)/$(APPDIR)
	@cp -a $(BINARY) $(OUTDIR)/$(APPDIR)/$(MACAPP)/Contents/MacOS
	@cp -Ra $(FONTDIR) $(OUTDIR)/$(APPDIR)/$(MACAPP)/Contents/MacOS
	@cd $(OUTDIR) && tar -cjf $(ARCHIVE) $(APPDIR)
	@echo "Packaged $(APPNAME) into $(APPDIR)/$(MACAPP)"
	@echo
	@echo "Archived into $(OUTDIR)/$(ARCHIVE)"

.PHONY: package-nix
package-nix: package-common $(RESDIR)/nix/*
	@cp -a $(BINARY) $(OUTDIR)/$(APPDIR)
	@cp -Ra $(FONTDIR) $(OUTDIR)/$(APPDIR)
	@echo "Packaged $(APPNAME) into $(APPDIR)."

.PHONY: package-win32
package-win32: package-common $(RESDIR)/win32/*
	@cp -a $(BINARY) $(OUTDIR)/$(APPDIR)
	@cp -Ra $(FONTDIR) $(OUTDIR)/$(APPDIR)
	@echo "Packaged $(APPNAME) into $(APPDIR)."

.PHONY: package-common
package-common: $(BINARY) $(RESDIR)/common/*
	@echo
	@echo "Packaging $(APPNAME)..."
	@mkdir -p $(OUTDIR)/$(APPDIR)

.PHONY: package
package: package-$(PLAT)
	@echo
	@echo "Done packaging. See the $(APPDIR) folder."
	@echo


######################################################################
# Running the app
######################################################################

.PHONY: run-mac
run-mac: package
	@open $(OUTDIR)/$(APPDIR)/$(MACAPP)
	
.PHONY: run-nix
run-nix: package
	@cd $(OUTDIR)/$(APPDIR) && ./$(BINARY)

.PHONY: run-win32
run-win32: package
	@cd $(OUTDIR)/$(APPDIR) && ./$(BINARY)

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
