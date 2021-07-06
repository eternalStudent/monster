// include standard headers
#include <Windows.h>
#include <math.h>
#include <intrin.h> // this is MSVC specific, should be replaced with cross-compiler header
#include <emmintrin.h>
#include <memory.h>

#include "types.h"

// define ssize and set arena-allocator as the allocator
#define ssize int32
#include "arena.cpp"
static Arena arena;
#define Alloc(size) 						ArenaAlloc(&arena, size, false)
#define ReAlloc(data, old_size, new_size) 	ArenaResize(&arena, data, old_size, new_size, false)
#define Free(data)

// util libraries
#include "string.cpp"
#include "array.cpp"

// now define assert and log
#define Assert(Expression) 					if(!(Expression)) {*(volatile int *)0 = 0;}
#define Log(text) 							MessageBoxA(NULL, text, "Monster", 0)
#define Fail(text)							(Log(text), 0)

// file-formats
#include "bitmap.cpp"
#include "truetype.cpp"

#include "win32io.cpp"
#define LoadText 							Win32LoadText
#define LoadStream 							Win32LoadStream
#define SaveStream							Win32SaveStream

static Dimensions2i windowDim;

#include "win32_opengl.cpp"
#include "opengl.cpp"
#define TextureHandle 						GLuint
#define GenerateTextureFromRGBA 			OpenGLGenerateTextureFromRGBA
#define GenerateTextureFromImage			OpenGLGenerateTextureFromImage
#define GenerateTextureFromFile(file, filter)	\
											GenerateTextureFromImage(LoadBMP((byte*)LoadStream(file)), filter)
#define Render 								OpenGLRender
#define RenderBox2							OpenGLRenderBox2
#define ClearScreen 						OpenGLClearScreen
#define Smooth								GL_LINEAR
#define Pixelated							GL_NEAREST

#include "font.cpp"
static BakedFont debugFont;
#define DebugPrintText(x, y, text)			PrintText(debugFont, x, y, text)

#include "tiles.cpp"
#include "gui.cpp"

#define Game 1
#define Editor 1
#if Game
	#include "game.cpp"
	#define Init 								GameInit
#elif Editor
	#include "editor.cpp"
	#define Init 								EditorInit
#else
	#include "gui_playground.cpp"
	#define Init 								PlaygroundInit
#endif

/*
 *   TODO:
 *   - tile properties
 */

// Windows & Monitors
// ------------------

static bool running = true;
static WINDOWPLACEMENT windowPlacement = { sizeof(WINDOWPLACEMENT) };

// https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
void ToggleFullscreen(HWND window){
	DWORD dwStyle = GetWindowLong(window, GWL_STYLE);
	if (dwStyle & WS_OVERLAPPEDWINDOW) {
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(window, &windowPlacement) &&
			GetMonitorInfo(MonitorFromWindow(window,
				MONITOR_DEFAULTTOPRIMARY), &mi)) {
			SetWindowLong(window, GWL_STYLE,
				dwStyle & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(window, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else {
		SetWindowLong(window, GWL_STYLE,
			dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(window, &windowPlacement);
		SetWindowPos(window, NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

WNDCLASSA CreateWindowClass(HINSTANCE hInstance, WNDPROC windowCallBack) {
	WNDCLASSA windowClass = {};
	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = windowCallBack;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = "WindowClass";

	RegisterClassA(&windowClass);

	return windowClass;
}

HWND Win32CreateWindow(HINSTANCE hInstance, WNDPROC windowCallBack, int width, int height) {
	WNDCLASSA windowClass = CreateWindowClass(hInstance, windowCallBack);

	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	RECT rect = { 0, 0, width, height };
	BOOL success = AdjustWindowRect(&rect, style, FALSE);
	if (!success)
		Log("fail to adjust window size");

	return CreateWindowExA(0,
		windowClass.lpszClassName, "Monster",
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		0, 0, hInstance, 0);
}

inline HMONITOR GetPrimaryMonitorHandle(){
	const POINT ptZero = { 0, 0 };
	return MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
}

HWND Win32CreateWindowFullScreen(HINSTANCE hInstance, WNDPROC windowCallBack, Dimensions2i* dimensions) {
	WNDCLASSA windowClass = CreateWindowClass(hInstance, windowCallBack);
	HMONITOR monitor = GetPrimaryMonitorHandle();
	MONITORINFO mi = { sizeof(mi) };

	if (!GetMonitorInfo(monitor, &mi)) {
		Log("failed to get monitor-info");
		return NULL;
	}

	dimensions->width = mi.rcMonitor.right - mi.rcMonitor.left;
	dimensions->height = mi.rcMonitor.bottom - mi.rcMonitor.top;

	HWND window = CreateWindowExA(0,
		windowClass.lpszClassName, "Monster",
		WS_VISIBLE,
		mi.rcMonitor.left,
		mi.rcMonitor.top,
		dimensions->width,
		dimensions->height,
		0, 0, hInstance, 0);
	
	// NOTE: WTF ?!?
	DWORD dwStyle = GetWindowLong(window, GWL_STYLE);
	SetWindowLong(window, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

	return window;
}

// TODO: move input??

// Input
// ---------

#define JUMP  0x01
#define LEFT  0x02
#define DOWN  0x04
#define RIGHT 0x08

Position2 Win32GetCursorPosition(HWND window){
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	RECT clientRect;
	GetClientRect(window, &clientRect);
	pixels x = (pixels)cursorPos.x-clientRect.left;
	int32 height = clientRect.bottom - clientRect.top;
	pixels y = (pixels)(height - (cursorPos.y - clientRect.left) - 1);
	return Position2{x, y};
}

void Win32ProcessPendingMessages(HWND window, uint32* keysPressed, MouseEventQueue* mouseEventQueue)
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		switch (message.message)
		{
		case WM_QUIT:
		{
			running = false;
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32 VKCode = (uint32)message.wParam;
			int32 wasDown = ((message.lParam & (1 << 30)) != 0);
			int32 isDown = ((message.lParam & (1 << 31)) == 0);
			if (wasDown != isDown)
			{
				if (VKCode == 'W')
				{
					if (isDown) *keysPressed |= JUMP;
					else *keysPressed &= ~JUMP;
				}
				else if (VKCode == 'A')
				{
					if (isDown) *keysPressed |= LEFT;
					else *keysPressed &= ~LEFT;
				}
				else if (VKCode == 'S')
				{
					if (isDown) *keysPressed |= DOWN;
					else *keysPressed &= ~DOWN;
				}
				else if (VKCode == 'D')
				{
					if (isDown) *keysPressed |= RIGHT;
					else *keysPressed &= ~RIGHT;
				}
				else if (VKCode == VK_UP)
				{
					if (isDown) *keysPressed |= JUMP;
					else *keysPressed &= ~JUMP;
				}
				else if (VKCode == VK_LEFT)
				{
					if (isDown) *keysPressed |= LEFT;
					else *keysPressed &= ~LEFT;
				}
				else if (VKCode == VK_DOWN)
				{
					if (isDown) *keysPressed |= DOWN;
					else *keysPressed &= ~DOWN;
				}
				else if (VKCode == VK_RIGHT)
				{
					if (isDown) *keysPressed |= RIGHT;
					else *keysPressed &= ~RIGHT;
				}
				else if (VKCode == VK_ESCAPE)
				{
					running = false;
				}
				else if (VKCode == VK_SPACE)
				{
					if (isDown) *keysPressed |= JUMP;
					else *keysPressed &= ~JUMP;
				}
			}

			int32 altKeyWasDown = (message.lParam & (1 << 29));
			if ((VKCode == VK_F4) && altKeyWasDown)
			{
				running = false;
			}
		} break;
		case WM_LBUTTONDOWN: {
			Enqueue(mouseEventQueue, LDN);
		}break;
		case WM_LBUTTONUP: {
			Enqueue(mouseEventQueue, LUP);
		}break;
		case WM_RBUTTONDOWN: {
			Enqueue(mouseEventQueue, RDN);
		}break;
		case WM_RBUTTONUP: {
			Enqueue(mouseEventQueue, RUP);
		}break;

		default:
		{
			TranslateMessage(&message);
			DispatchMessageA(&message);
		} break;
		}
	}
}

void Win32SetCursorToMove(){
	SetCursor(LoadCursorA(NULL, IDC_SIZEALL));
}

void Win32SetCursorToResize(){
	SetCursor(LoadCursorA(NULL, IDC_SIZENWSE));
}

void Win32SetCursorToArrow(){
	SetCursor(LoadCursorA(NULL, IDC_ARROW));
}

void Win32SetCursorToHand(){
	SetCursor(LoadCursorA(NULL, IDC_HAND));
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message)
	{
	case WM_SETCURSOR:{
#if Game
		SetCursor(0);
#endif
	} break;
	case WM_CLOSE:
	case WM_DESTROY: {
		running = false;
		return 0;
	}
	default:
	{
		return DefWindowProcA(window, message, wParam, lParam);
	}
	}
	return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	ssize arena_capacity = 1024 * 1024 * 256;
	void* buffer = VirtualAlloc(0, arena_capacity, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
 	ArenaInit(&arena, buffer, arena_capacity);
	HWND window = Win32CreateWindowFullScreen(hInstance, Win32MainWindowCallback, &windowDim);
	if (!window) return -1;

	if (!Win32InitOpenGL(window)) return -1;
	OpenGLSetVisibleArea();
	OpenGLCreateProgram();
	debugFont = BakeFont("times.ttf");
	
	//int win32RefreshRate = GetDeviceCaps(context, VREFRESH);
	//float32 gameUpdateHz = (float32)((win32RefreshRate > 1) ? win32RefreshRate : 60);
	//uint32 expectedFramesPerUpdate = 1;
	//float32 targetSecondsPerFrame = (float32)expectedFramesPerUpdate / (float32)gameUpdateHz;
	SpritesInit();
	Init();

	LARGE_INTEGER frequency, startTime, endTime;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startTime);

	running = true;
	uint32 keysPressed = 0;
	float32 deltaTime = 0.0f;
	MouseEventQueue mouseEventQueue = {};
	QReserve(&mouseEventQueue, 2);

	while (running) {
		ArenaFreeAll(&arena);
		QueryPerformanceCounter(&endTime);
		deltaTime = (float32)((endTime.QuadPart - startTime.QuadPart) * 1000) / (float32)frequency.QuadPart; // milliseconds
		startTime = endTime;
		Win32ProcessPendingMessages(window, &keysPressed, &mouseEventQueue);
		Position2 cursorPos = Win32GetCursorPosition(window);

#if Game
		GameUpdateAndRender(keysPressed, deltaTime, &mouseEventQueue, cursorPos);
#elif Editor 
		EditorUpdateAndRender(&mouseEventQueue, cursorPos);
#else
		PlaygroundUpdateAndRender(&mouseEventQueue, cursorPos);
#endif
		
		//Sleep(100.0/6.0 - deltaTime);

		SwapBuffers(wglGetCurrentDC());
	}

	return 0;
}