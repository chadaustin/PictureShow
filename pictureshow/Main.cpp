#include <stdlib.h>
#include <time.h>
#include "WindowProc.hpp"


HINSTANCE MainInstance;


////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    srand(time(NULL));

    MainInstance = instance;

    // register window class
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc   = MainWindowProc;
    wc.hInstance     = instance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "ImageShowMainWindow";
    if (RegisterClass(&wc) == 0) {
        MessageBox(NULL, "RegisterClass() failed", "Image Show", MB_OK);
        return -1;
    }

    // create window
    HWND window = CreateWindow(
      "ImageShowMainWindow",
      "Image Show",
      WS_CLIPCHILDREN | WS_POPUP,
      0,
      0,
      GetSystemMetrics(SM_CXSCREEN),
      GetSystemMetrics(SM_CYSCREEN),
      NULL,
      NULL,
      instance,
      NULL);
    if (window == NULL) {
        MessageBox(NULL, "CreateWindow() failed", "Image Show", MB_OK);
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
