#ifndef WINDOW_PROC_HPP
#define WINDOW_PROC_HPP


#include <windows.h>


LRESULT CALLBACK MainWindowProc(
  HWND window,
  UINT message,
  WPARAM wparam,
  LPARAM lparam);


#endif // WINDOW_PROC_HPP
