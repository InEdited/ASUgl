//#include <iostream>
#include "util_window.h"
#include "renderer.h"
#include "ctime"

const int screen_width = 1000;
const int screen_height = 1000;

int prev_mouse_x = 0;
int prev_mouse_y = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   HWND                hwnd;
   MSG                 Msg;

   hwnd = create_window(hInstance);
   ShowWindow(hwnd, nCmdShow);

   char debug_str[100];

   clock_t first_time = clock();
   render();
   clock_t end_time = clock();
   Update();

   sprintf_s(debug_str, "%f\n", (double)(end_time - first_time) / CLOCKS_PER_SEC);
   OutputDebugString(debug_str);
   
   while (GetMessage(&Msg, NULL, 0, 0))
   {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
   }
   return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		int xPos, yPos;
			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);

		if (xPos == prev_mouse_x)
			break;
		else if (xPos > prev_mouse_x)
			move_camera_right();
		else
			move_camera_left();
		prev_mouse_x = xPos;

		if (yPos == prev_mouse_y)
			break;
		else if (yPos > prev_mouse_y)
			move_camera_up();
		else
			move_camera_down();
		prev_mouse_y = yPos;

		render();
		Update();
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONDOWN:
		Update();
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
