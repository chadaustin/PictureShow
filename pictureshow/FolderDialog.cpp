#include <shlobj.h>
#include "FolderDialog.hpp"


bool BrowseForFolderDialog(
  HWND parent,
  const char* title,
  char folder[MAX_PATH])
{
  BROWSEINFO bi;
  bi.hwndOwner      = parent;
  bi.pidlRoot       = NULL;
  bi.pszDisplayName = NULL;
  bi.lpszTitle      = title;
  bi.ulFlags        = BIF_RETURNONLYFSDIRS;
  bi.lpfn           = NULL;
  bi.lParam         = 0;
  bi.iImage         = 0;

  // browse dialog
  LPITEMIDLIST idlist = SHBrowseForFolder(&bi);
  if (!idlist) {
    return false;
  }

  // convert id list into path
  if (!SHGetPathFromIDList(idlist, folder)) {
    return false;
  }

  // free the id list
  LPMALLOC malloc;
  SHGetMalloc(&malloc);
  malloc->Free(idlist);
  malloc->Release();

  return true;
}