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
#include <stdio.h>


WNDCLASS wc;
HWND hWnd;
HDC hDC;
HGLRC hRC;
MSG msg;
BOOL runLoop = FALSE;
BOOL quit = FALSE;
BOOL rendering = FALSE;


static
DWORD
WINAPI loop(LPVOID param)
{
	DWORD next_game_tick = GetTickCount();
	int sleep_time = 0;

	while(runLoop) {
		App_OnUpdate();
		
		InvalidateRect(hWnd, NULL, FALSE);
		
		next_game_tick += SKIP_TICKS;
		sleep_time = next_game_tick - GetTickCount();

		if(sleep_time >= 0) {
			Sleep(sleep_time);
		}
	}

	return 0;
}


static
void
startLoop()
{
	runLoop = TRUE;
	CreateThread(NULL, 0, loop, NULL, 0, NULL);
}


static
void
stopLoop()
{
	runLoop = FALSE;
}


static
void
EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
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


static
void
DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);
}


static
BOOL
enterFullscreen(HWND hWnd) 
{
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


static
void
getWindowPos(HWND hWnd, int * windowX, int * windowY)
{
	HDC windowHDC = GetDC(hWnd);
	int fullscreenWidth  = GetDeviceCaps(windowHDC, HORZRES);
	int fullscreenHeight = GetDeviceCaps(windowHDC, VERTRES);
	*windowX = (fullscreenWidth / 2) - (WIN_INIT_WIDTH / 2);
	*windowY = (fullscreenHeight / 2) - (WIN_INIT_HEIGHT / 2);
}


static
void
ClientResize(HWND hWnd, int nWidth, int nHeight)
{
	RECT rcClient, rcWind;
	POINT ptDiff;
	int border = GetSystemMetrics(SM_CXSIZEFRAME);

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWind);
	ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
	ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;

	/*
	 * This moves the window so that when toggling fullscreen,
	 * the text doesn't move. (Have to compensate for the border.)
	 */
	MoveWindow(hWnd, rcWind.left, rcWind.top, 
		nWidth, nHeight - ptDiff.y + border, TRUE);
}


static
BOOL
exitFullscreen(HWND hWnd)
{
    BOOL isChangeSuccessful;

    int windowX;
    int windowY;

    SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_LEFT);
    SetWindowLongPtr(hWnd, GWL_STYLE, WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

    isChangeSuccessful = ChangeDisplaySettings(NULL, CDS_RESET) == DISP_CHANGE_SUCCESSFUL;

    // move the window to the middle of the screen
    getWindowPos(hWnd, &windowX, &windowY);
    SetWindowPos(hWnd, HWND_NOTOPMOST, windowX, windowY, WIN_INIT_WIDTH, WIN_INIT_HEIGHT, SWP_SHOWWINDOW);

    /* 
     * Resizes the client (content area) so that the text appears
     * in the same place whether fullscreen or window mode.
     */
    ClientResize(hWnd, WIN_INIT_WIDTH, WIN_INIT_HEIGHT);
    ShowWindow(hWnd, SW_RESTORE);

    return isChangeSuccessful;
}


static
void
toggleFullscreen()
{
	static BOOL fullscreen = FALSE;

	if(!fullscreen) {
		fullscreen = enterFullscreen(hWnd);
	} else {
		fullscreen = !exitFullscreen(hWnd);
	}
}


static
void
onQuitRequest()
{
	runLoop = FALSE;
	quit = TRUE;
}


LRESULT
CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static cs_key_mod_t mods = CS_NONE;
	//static unsigned int painting = 0;
	//TODO: Unicode and Unichar
	//static char ch[256];

	switch (message) {
	case WM_CREATE:
		return 0;

	case WM_SIZE:
	{
		int w = LOWORD(lParam);
		int h = HIWORD(lParam);
		App_OnResize(w, h);

		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}

	case WM_PAINT:
	{
		App_OnRender();
		SwapBuffers(hDC);
		ValidateRect(hWnd, NULL);
		return 0;
	}

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	
	case WM_QUIT:
		return 0;

	case WM_DESTROY:
		return 0;

	case WM_CHAR:
	{
		char wide[3] = {0};
		wide[0] = (char)wParam;

		//printf("key: %c, code: %d\n", (unsigned char)wide[0], (int)wide[0]);
		if((unsigned char)wide[0] > 31) {
			App_OnKeyDown(wide, mods);
		} else if(wide[0] > 0 && wide[0] <= 26) {
			if(MODS_COMMAND(mods)) {
				wide[0] += 'a'-1;
				//printf("key: %c, code: %d\n", (unsigned char)wide[0], (int)wide[0]);
				App_OnKeyDown(wide, mods);
			} else {
				switch(wide[0]) {
				case '\r':
					wide[0] = '\n';
					//fall through
				case '\t':
				case '\b':
					App_OnKeyDown(wide, mods);
					break;
				default:
					break;
				}
			}
		}

		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	}

	case WM_KEYUP:
	{
		switch (wParam)
		{
		case VK_ESCAPE:      App_OnSpecialKeyUp(CS_ESCAPE,mods);          break;
		case VK_LEFT:        App_OnSpecialKeyUp(CS_ARROW_LEFT,mods);      break;
		case VK_RIGHT:       App_OnSpecialKeyUp(CS_ARROW_RIGHT,mods);     break;
		case VK_UP:          App_OnSpecialKeyUp(CS_ARROW_UP,mods);        break;
		case VK_DOWN:        App_OnSpecialKeyUp(CS_ARROW_DOWN,mods);      break;
		case VK_CONTROL:     mods ^= CS_CONTROL;                          break;
		case VK_SHIFT:       mods ^= CS_SHIFT;                            break;
		default:
			break;
		}

		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	}
		
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:      App_OnSpecialKeyDown(CS_ESCAPE,mods);        break;
		case VK_LEFT:        App_OnSpecialKeyDown(CS_ARROW_LEFT,mods);    break;
		case VK_RIGHT:       App_OnSpecialKeyDown(CS_ARROW_RIGHT,mods);   break;
		case VK_UP:          App_OnSpecialKeyDown(CS_ARROW_UP,mods);      break;
		case VK_DOWN:        App_OnSpecialKeyDown(CS_ARROW_DOWN,mods);    break;
		case VK_CONTROL:     mods |= CS_CONTROL;                          break;
		case VK_SHIFT:       mods |= CS_SHIFT;                            break;
		default:
			break;
		}

		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	}

	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


int
WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
               LPSTR lpCmdLine, int iCmdShow)
{
	int windowX;
	int windowY;

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

	getWindowPos(NULL, &windowX, &windowY);
	
	// create main window
	hWnd = CreateWindow(
		(LPCSTR)APP_NAME, (LPCSTR)APP_NAME, 
		WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		windowX, windowY, WIN_INIT_WIDTH, WIN_INIT_HEIGHT,
		NULL, NULL, hInstance, NULL);

	ClientResize(hWnd, WIN_INIT_WIDTH, WIN_INIT_HEIGHT);
	
	// enable OpenGL for the window
	EnableOpenGL(hWnd, &hDC, &hRC);

	App_OnInit();
	App_FullscreenDel(toggleFullscreen);
	App_AnimationDel(startLoop, stopLoop);
	App_QuitRequestDel(onQuitRequest);
	
	while(!quit && GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	stopLoop();

	App_OnDestroy();
	// shutdown OpenGL
	DisableOpenGL(hWnd, hDC, hRC);
	
	// destroy the window explicitly
	DestroyWindow(hWnd);
	
	return 0;
	//return (int)msg.wParam;
}
