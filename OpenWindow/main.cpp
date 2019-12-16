//#include <iostream>
#include "util_window.h"
#include "renderer.h"
#include "ctime"

const int screen_width = 1000;
const int screen_height = 1000;

#define TARGET_FRAMERATE 30

int prev_mouse_x = screen_width/2;
int prev_mouse_y = screen_height/2;

float TIME = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   HWND                hwnd;
   MSG                 Msg;

   hwnd = create_window(hInstance);
   ShowCursor(false);
   ShowWindow(hwnd, nCmdShow);

   init_camera();


   SetTimer(hwnd, NULL, 1000 / TARGET_FRAMERATE, (TIMERPROC)FixedUpdate);

   while (GetMessage(&Msg, NULL, 0, 0))
   {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
   }
   return Msg.wParam;
}

void HandleMouseMovement() {
		POINT point;
		GetCursorPos(&point);

		camera.rotate_hor(point.x - prev_mouse_x);
		camera.rotate_ver(point.y - prev_mouse_y);

		SetCursorPos(prev_mouse_x, prev_mouse_y);
}

bool HandleButtonPressed() {
	if (GetAsyncKeyState(0x57) & 0x8000)
		camera.move_camera_forward();
	if (GetAsyncKeyState(0x53) & 0x8000)
		camera.move_camera_backward();
	if (GetAsyncKeyState(0x44) & 0x8000)
		camera.move_camera_right();
	if (GetAsyncKeyState(0x41) & 0x8000)
		camera.move_camera_left();
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		camera.rise();
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		camera.fall();
	return true;
}

void CALLBACK FixedUpdate(HWND hwnd, UINT message, UINT uInt, DWORD dWord)
{
	TIME += 0.167;
	HandleButtonPressed();
	camera.ApplyChanges();
	render();
	Update();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		HandleMouseMovement();
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
}
