#include "GetDelayDialog.hpp"
#include "Main.hpp"
#include "resource.h"


static BOOL CALLBACK DialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);


static int Delay;


////////////////////////////////////////////////////////////////////////////////

int GetDelayDialog(HWND parent)
{
    DialogBox(MainInstance, MAKEINTRESOURCE(IDD_GETDELAY), parent, DialogProc);
    return Delay;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {

        case WM_COMMAND: {
            
            if (LOWORD(wparam) == IDOK) {
                BOOL translated;
                Delay = GetDlgItemInt(window, IDC_DELAY, &translated, FALSE);

                EndDialog(window, 0);
                return TRUE;
            }

            if (LOWORD(wparam) == IDCANCEL) {
                EndDialog(window, 0);
                return TRUE;
            }

            return FALSE;
        }

        default: {
            return FALSE;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
