#ifndef FOLDER_DIALOG_H
#define FOLDER_DIALOG_H


#include <windows.h>


extern bool BrowseForFolderDialog(
  HWND parent,
  const char* title,
  char folder[MAX_PATH]);


#endif
