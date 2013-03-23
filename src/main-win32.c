/*************************************************************************
 * main-win32.c -- Uses Win32 to create a window and start the app.
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

#include "opengl.h"
#include "app.h"

static const char * const APP_NAME = "Candlestick";

// Function Declarations
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC);
void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);

// WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int iCmdShow)
{
	WNDCLASS wc;
	HWND hWnd;
	HDC hDC;
	HGLRC hRC;
	MSG msg;
	BOOL quit = FALSE;
	
	// register window class
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = (LPCSTR)APP_NAME;
	RegisterClass(&wc);
	
	// create main window
	hWnd = CreateWindow( 
		(LPCSTR)APP_NAME, (LPCSTR)APP_NAME, 
		WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
		0, 0, WIN_INIT_WIDTH, WIN_INIT_HEIGHT,
		NULL, NULL, hInstance, NULL);
	
	// enable OpenGL for the window
	EnableOpenGL(hWnd, &hDC, &hRC);

	App_OnInit();
	
	while (!quit) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			// handle or dispatch messages
			if (msg.message == WM_QUIT) {
				quit = TRUE;
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else {
			//render
			App_OnRender();

			//swap buffers
			SwapBuffers(hDC);
		}
	}

	App_OnDestroy();
	
	// shutdown OpenGL
	DisableOpenGL(hWnd, hDC, hRC);
	
	// destroy the window explicitly
	DestroyWindow(hWnd);
	
	return msg.wParam;
	
}

BOOL enterFullscreen(HWND hWnd) {
    DEVMODE fullscreenSettings;

	HDC windowHDC = GetDC(hWnd);
	int fullscreenWidth  = GetDeviceCaps(windowHDC, HORZRES);
	int fullscreenHeight = GetDeviceCaps(windowHDC, VERTRES);
	int colourBits       = GetDeviceCaps(windowHDC, BITSPIXEL);
	int refreshRate      = GetDeviceCaps(windowHDC, VREFRESH);

    EnumDisplaySettings(NULL, 0, &fullscreenSettings);
    fullscreenSettings.dmPelsWidth        = fullscreenWidth;
    fullscreenSettings.dmPelsHeight       = fullscreenHeight;
    fullscreenSettings.dmBitsPerPel       = colourBits;
    fullscreenSettings.dmDisplayFrequency = refreshRate;
    fullscreenSettings.dmFields           = DM_PELSWIDTH |
                                            DM_PELSHEIGHT |
                                            DM_BITSPERPEL |
                                            DM_DISPLAYFREQUENCY;

    SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
    SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, fullscreenWidth, fullscreenHeight, SWP_SHOWWINDOW);
	//only need this line if changing the resolution
    //isChangeSuccessful = ChangeDisplaySettings(&fullscreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
    ShowWindow(hWnd, SW_MAXIMIZE);

    return TRUE;
}

BOOL exitFullscreen(HWND hWnd, int windowX, int windowY, int windowedWidth, int windowedHeight, int windowedPaddingX, int windowedPaddingY) {
    BOOL isChangeSuccessful;

    SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_LEFT);
    SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    isChangeSuccessful = ChangeDisplaySettings(NULL, CDS_RESET) == DISP_CHANGE_SUCCESSFUL;
    SetWindowPos(hWnd, HWND_NOTOPMOST, windowX, windowY, windowedWidth + windowedPaddingX, windowedHeight + windowedPaddingY, SWP_SHOWWINDOW);
    ShowWindow(hWnd, SW_RESTORE);

    return isChangeSuccessful;
}

// Window Procedure

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	switch (message)
	{
	case WM_CREATE:
		return 0;

	case WM_SIZE:
		{
			int w = LOWORD(lParam);
			int h = HIWORD(lParam);
			App_OnResize(w, h);
			return 0;
		}
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
		
	case WM_DESTROY:
		return 0;

	case WM_CHAR:
		App_OnKeyDown(wParam);
		return 0;
		
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		case VK_F1:
			{
				static BOOL fullscreen = FALSE;

				if(!fullscreen) {
					fullscreen = enterFullscreen(hWnd);
				} else {
					fullscreen = !exitFullscreen(hWnd, 0, 0, WIN_INIT_WIDTH, WIN_INIT_HEIGHT, 0, 0);
				}
			}
			return 0;
		}
		return 0;
	
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// Enable OpenGL

void EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int format;
	
	// get the device context (DC)
	*hDC = GetDC(hWnd);
	
	// set the pixel format for the DC
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat(*hDC, &pfd);
	SetPixelFormat(*hDC, format, &pfd);
	
	// create and enable the render context (RC)
	*hRC = wglCreateContext(*hDC);
	wglMakeCurrent(*hDC, *hRC);
}

// Disable OpenGL

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);
}
