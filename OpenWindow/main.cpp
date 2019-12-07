//#include <iostream>
#include "util_window.h"
#include "renderer.h"
#include "ctime"

const int width = 800;
const int height = 800;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   HWND                hwnd;
   MSG                 Msg;

   hwnd = create_window(width, height, hInstance);
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
