#include <stdlib.h>
#include <time.h>
#include "WindowProc.hpp"


////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
  srand(time(0));

  // register window class
  WNDCLASS wc;
  memset(&wc, 0, sizeof(wc));
  wc.lpfnWndProc   = MainWindowProc;
  wc.hInstance     = instance;
  wc.hIcon         = NULL;
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = "PictureShowWindow";
  RegisterClass(&wc);  // if class registration fails, window creation will

  // create window
  HWND window = CreateWindow(
    "PictureShowWindow",
    "PictureShow",
    WS_CLIPCHILDREN | WS_POPUP,
    0, 0,
    GetSystemMetrics(SM_CXSCREEN),
    GetSystemMetrics(SM_CYSCREEN),
    0, 0, instance, 0);
  if (!window ) {
    MessageBox(NULL, "CreateWindow() failed", "PictureShow", MB_OK);
    return -1;
  }

  // display the window
  ShowWindow(window, SW_SHOW);
  UpdateWindow(window);

  // message loop
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.wParam;
}

////////////////////////////////////////////////////////////////////////////////
