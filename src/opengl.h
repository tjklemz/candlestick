/*************************************************************************
 * opengl.h -- Includes the appropriate OpenGL headers based on platform.
 *
 * Candlestick App: Just Write. A minimalist, cross-platform writing app.
 * Copyright (C) 2013 Thomas Klemz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#ifndef MY_OPENGL_H
#define MY_OPENGL_H

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#endif

#ifdef __APPLE__
#	include <OpenGL/gl.h>
#	include <OpenGL/glu.h>
#else
#	include <GL/gl.h>
#	include <GL/glu.h>
#endif

#if defined(__linux__) && !defined(__APPLE__)
#	include <X11/Xlib.h>
#	include <GL/glx.h>
#endif

#endif
