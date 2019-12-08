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

bool HandleMouseMovement(LPARAM lParam) {
		int xPos, yPos;
		xPos = LOWORD(lParam);
		yPos = HIWORD(lParam);

		bool movement_detected_x = true;
		bool movement_detected_y = true;

		if (xPos > prev_mouse_x)
			move_camera_right();
		else if (xPos < prev_mouse_x)
			move_camera_left();
		else
			movement_detected_x = false;
		prev_mouse_x = xPos;

		if (yPos > prev_mouse_y)
			move_camera_up();
		else if (yPos < prev_mouse_y)
			move_camera_down();
		else
			movement_detected_y = false;
		prev_mouse_y = yPos;

		return movement_detected_x || movement_detected_y;
}

void Update_Window() {
	render();
	Update();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		if(HandleMouseMovement(lParam))
			Update_Window();
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
