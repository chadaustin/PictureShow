#include <memory>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <corona.h>
#include "FolderDialog.hpp"
#include "GetDelayDialog.hpp"
#include "WindowProc.hpp"
#include "strcmp_ci.hpp"
#include "resource.h"


#define TIMER_ID 900


struct StringNode
{
    std::string str;
    StringNode* next;
};

typedef unsigned char byte;


enum DM { DM_ORIGINALSIZE, DM_STRETCHED };


static bool DestroyImageList();
static bool CreateImageList(const char* directory);
static const char* GetImage(int index);
static void AddImage(const char* image);
static bool UpdateImage();
static bool NextImage();
static bool PrevImage();
static void DrawShadedText(HDC dc, int x, int y, const char* text);
static void ResetTimer(HWND window);


static HDC     ImageDC;
static HBITMAP ImageBitmap;
static int     ImageWidth;
static int     ImageHeight;

static StringNode* ImageList;
static int         ImageListSize;
static int         CurrentImage = -1;


static int Timer = 5000;
static DM  DisplayMode;

static bool Random;
static bool RecurseSubdirectories;


////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK MainWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {

        case WM_CREATE: {
            ImageDC = CreateCompatibleDC(NULL);

            SetTimer(window, TIMER_ID, 15, NULL);
            return 0;
        }

        ////////////////////////////////////////////////////////////////////////

        case WM_DESTROY: {
            KillTimer(window, TIMER_ID);
            DeleteDC(ImageDC);
            DestroyImageList();
            PostQuitMessage(0);
            return 0;
        }

        ////////////////////////////////////////////////////////////////////////

        case WM_RBUTTONUP: {
            HMENU menu = LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(IDR_MAIN));
            HMENU sub_menu = GetSubMenu(menu, 0);

            // update the menu
            switch (Timer) {
                case 0:     CheckMenuItem(menu, ID_MAIN_DELAY_NONE,  BST_CHECKED); break;
                case 1000:  CheckMenuItem(menu, ID_MAIN_DELAY_1S,    BST_CHECKED); break;
                case 2000:  CheckMenuItem(menu, ID_MAIN_DELAY_2S,    BST_CHECKED); break;
                case 5000:  CheckMenuItem(menu, ID_MAIN_DELAY_5S,    BST_CHECKED); break;
                case 10000: CheckMenuItem(menu, ID_MAIN_DELAY_10S,   BST_CHECKED); break;
                default:    CheckMenuItem(menu, ID_MAIN_DELAY_OTHER, BST_CHECKED); break;
            }

            if (DisplayMode == DM_ORIGINALSIZE) {
                CheckMenuItem(menu, ID_MAIN_DISPLAY_ORIGINALSIZE, BST_CHECKED);
            } else {
                CheckMenuItem(menu, ID_MAIN_DISPLAY_STRETCHED, BST_CHECKED);
            }

            if (Random) {
                CheckMenuItem(menu, ID_MAIN_OPTIONS_RANDOM, BST_CHECKED);
            }

            if (RecurseSubdirectories) {
                CheckMenuItem(menu, ID_MAIN_OPTIONS_RECURSESUBDIRECTORIES, BST_CHECKED);
            }

            POINT cursor;
            GetCursorPos(&cursor);
            TrackPopupMenuEx(sub_menu, TPM_LEFTALIGN | TPM_TOPALIGN, cursor.x, cursor.y, window, NULL);
            return 0;
        }

        ////////////////////////////////////////////////////////////////////////

        case WM_LBUTTONDOWN: {

            if (wparam & MK_SHIFT) {
                PrevImage();
            } else {
                NextImage();
            }
            InvalidateRect(window, NULL, TRUE);

            ResetTimer(window);
            return 0;
        }

        ////////////////////////////////////////////////////////////////////////

        case WM_COMMAND: {
            switch (LOWORD(wparam)) {

                case ID_MAIN_SETDIRECTORY: {
                    char image_folder[MAX_PATH];
                    if (BrowseForFolderDialog(window, "Choose Image Directory", image_folder)) {
                        // now that we have the directory, update the image list
                        CurrentImage = -1;
                        DestroyImageList();
                        CreateImageList(image_folder);
                        UpdateImage();

                        InvalidateRect(window, NULL, TRUE);
                        ResetTimer(window);
                    }
                    return 0;
                }

                case ID_MAIN_DELAY_NONE: {
                    Timer = 0;
                    ResetTimer(window);
                    return 0;
                }

                case ID_MAIN_DELAY_1S: {
                    Timer = 1000;
                    ResetTimer(window);
                    return 0;
                }

                case ID_MAIN_DELAY_2S: {
                    Timer = 2000;
                    ResetTimer(window);
                    return 0;
                }

                case ID_MAIN_DELAY_5S: {
                    Timer = 5000;
                    ResetTimer(window);
                    return 0;
                }

                case ID_MAIN_DELAY_10S: {
                    Timer = 10000;
                    ResetTimer(window);
                    return 0;
                }

                case ID_MAIN_DELAY_OTHER: {
                    int timer = GetDelayDialog(window);
                    if (timer != -1)
                        Timer = timer;
                    ResetTimer(window);
                    return 0;
                }

                case ID_MAIN_DISPLAY_ORIGINALSIZE: {
                    DisplayMode = DM_ORIGINALSIZE;
                    InvalidateRect(window, NULL, TRUE);
                    return 0;
                }

                case ID_MAIN_DISPLAY_STRETCHED: {
                    DisplayMode = DM_STRETCHED;
                    InvalidateRect(window, NULL, TRUE);
                    return 0;
                }

                case ID_MAIN_OPTIONS_RANDOM: {
                    Random = !Random;
                    return 0;
                }

                case ID_MAIN_OPTIONS_RECURSESUBDIRECTORIES: {
                    RecurseSubdirectories = !RecurseSubdirectories;
                    return 0;
                }

                case ID_MAIN_ABOUT: {
                    MessageBox(window, "(c) Chad Austin 2000\n"
                                       __DATE__, "PictureShow", MB_OK);
                    return 0;
                }

                case ID_MAIN_EXIT: {
                    DestroyWindow(window);
                    return 0;
                }
            }
            return 0;
        }

        ////////////////////////////////////////////////////////////////////////

        case WM_TIMER: {
            if (Timer != 0) {
                NextImage();
                InvalidateRect(window, NULL, TRUE);
            }
            return 0;
        }

        ////////////////////////////////////////////////////////////////////////

        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(window, &ps);

            // draw the image
            if (ImageWidth != 0 && ImageHeight != 0) {

                int screen_width  = GetSystemMetrics(SM_CXSCREEN);
                int screen_height = GetSystemMetrics(SM_CYSCREEN);

                bool stretched_view = DisplayMode == DM_STRETCHED;

                if (DisplayMode == DM_ORIGINALSIZE) {   // original size view

                    if (ImageWidth < screen_width &&
                        ImageHeight < screen_height) {

                        int x = (screen_width  - ImageWidth)  / 2;
                        int y = (screen_height - ImageHeight) / 2;
                        BitBlt(ps.hdc, x, y, ImageWidth, ImageHeight, ImageDC, 0, 0, SRCCOPY);

                    } else {  // do the stretched view if the image is bigger than the screen
                        stretched_view = true;
                    }

                }

                // stretched view
                if (stretched_view) {

                    float screen_aspect_ratio = (float)screen_width / screen_height;

                    // calculate destination rectangle
                    float image_aspect_ratio = (float)ImageWidth / ImageHeight;

                    int dest_x = 0;
                    int dest_y = 0;
                    int dest_w;
                    int dest_h;

                    // we're going to fill the width, but not the height
                    if (image_aspect_ratio > screen_aspect_ratio) {
                        dest_w = screen_width;
                        dest_h = (int)(screen_height * screen_aspect_ratio / image_aspect_ratio);
                    } else {
                        dest_w = (int)(screen_width * image_aspect_ratio / screen_aspect_ratio);
                        dest_h = screen_height;
                    }
                    dest_x = (screen_width  - dest_w) / 2;
                    dest_y = (screen_height - dest_h) / 2;
            
                    StretchBlt(ps.hdc, dest_x, dest_y, dest_w, dest_h, ImageDC, 0, 0, ImageWidth, ImageHeight, SRCCOPY);
                }
            }

            // draw the image filename
            char text[MAX_PATH + 80];
            sprintf(text, "%d/%d", CurrentImage + 1, ImageListSize);
            if (CurrentImage < ImageListSize && ImageListSize > 0) {
                strcat(text, " -- ");
                strcat(text, GetImage(CurrentImage));
            }
            DrawShadedText(ps.hdc, 0, 0, text);

            EndPaint(window, &ps);
            return 0;
        }

        ////////////////////////////////////////////////////////////////////////

        default: {
            return DefWindowProc(window, message, wparam, lparam);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

static bool IsJPEG(const char* filename)
{
    return ((strlen(filename) > 5 && strcmp_ci(filename + strlen(filename) - 5, ".jpeg") == 0) ||
            (strlen(filename) > 4 && strcmp_ci(filename + strlen(filename) - 4, ".jpg") == 0));
}

////////////////////////////////////////////////////////////////////////////////

static bool IsPNG(const char* filename)
{
    return (strlen(filename) > 4 && strcmp_ci(filename + strlen(filename) - 4, ".png") == 0);
}

////////////////////////////////////////////////////////////////////////////////

static bool IsPCX(const char* filename)
{
    return (strlen(filename) > 4 && strcmp_ci(filename + strlen(filename) - 4, ".pcx") == 0);
}

////////////////////////////////////////////////////////////////////////////////

bool DestroyImageList()
{
    StringNode* node = ImageList;
    while (node) {
        StringNode* p = node;
        node = node->next;
        delete p;
    }
    ImageList = NULL;
    ImageListSize = 0;
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool CreateImageList(const char* _directory)
{
    char directory[MAX_PATH];
    strcpy(directory, _directory);
    if (directory[strlen(directory) - 1] == '\\') {
        directory[strlen(directory) - 1] = 0;
    }

    char old_directory[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, old_directory);
    if (!SetCurrentDirectory(directory)) {
      return false;
    }

    WIN32_FIND_DATA ffd;
    HANDLE handle = FindFirstFile("*", &ffd);
    if (handle == INVALID_HANDLE_VALUE) {
        SetCurrentDirectory(old_directory);
        return false;
    }

    do {
        // if the file is a directory and we're supposed to recurse directories
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
            RecurseSubdirectories &&
            strcmp(ffd.cFileName, ".") != 0 &&
            strcmp(ffd.cFileName, "..") != 0) {

            char full_path[MAX_PATH];
            sprintf(full_path, "%s\\%s", directory, ffd.cFileName);
            CreateImageList(full_path);

        // if the file is a JPEG or PNG, add it to the list
        } else {
            if (strlen(ffd.cFileName) > 4) {
//                if (IsJPEG(ffd.cFileName) || IsPNG(ffd.cFileName)) {
                if (IsPCX(ffd.cFileName)) {

                    char path[MAX_PATH];
                    sprintf(path, "%s\\%s", directory, ffd.cFileName);
                    AddImage(path);
                }
            }
        }
    } while (FindNextFile(handle, &ffd));

    FindClose(handle);

    SetCurrentDirectory(old_directory);
    return true;
}

////////////////////////////////////////////////////////////////////////////////

const char* GetImage(int index)
{
    if (index < 0) {
        return NULL;
    }

    StringNode* node = ImageList;
    while (index--) {
        node = node->next;
    }
    return node->str.c_str();
}

////////////////////////////////////////////////////////////////////////////////

void AddImage(const char* image)
{
    StringNode* n = new StringNode;
    n->str = image;
    n->next = NULL;

    if (ImageList == NULL) {
        ImageList = n;
    } else {
        StringNode* p = ImageList;
        while (p->next) {
            p = p->next;
        }
        p->next = n;
    }

    ImageListSize++;
}

////////////////////////////////////////////////////////////////////////////////

bool UpdateImage()
{
    // destroy the old DIB section
    if (ImageBitmap) {
        DeleteObject(ImageBitmap);
        ImageBitmap = NULL;
    }
    ImageWidth = 0;
    ImageHeight = 0;

    if (ImageListSize < 1) {
        return false;
    }

    if (CurrentImage < 0) {
        CurrentImage = 0;
    }

    // load the image
    const char* filename = GetImage(CurrentImage);
    corona::Image* image = corona::OpenImage(filename);
    if (!image) {
        return false;
    }

    // get image dimensions
    ImageWidth   = image->getWidth();
    ImageHeight  = image->getHeight();
    int ImageBPP = image->getFormat() == corona::R8G8B8 ? 24 : 32;

    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    BITMAPINFOHEADER& bmih = bmi.bmiHeader;
    bmih.biSize     = sizeof(bmih);
    bmih.biWidth    = ImageWidth;
    bmih.biHeight   = -ImageHeight;
    bmih.biPlanes   = 1;
    bmih.biBitCount = 24;
    
    // create a new DIB section based on the image
    byte* bits;
    ImageBitmap = CreateDIBSection(ImageDC, &bmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);

    byte* in = (unsigned char*)image->getPixels();
    int row_length = (ImageWidth * 3 + 3) / 4 * 4;
    for (int iy = 0; iy < ImageHeight; ++iy) {
      byte* row = bits + row_length * iy;
      for (int ix = 0; ix < ImageWidth; ++ix) {
        byte red   = *in++;
        byte green = *in++;
        byte blue  = *in++;
        if (ImageBPP == 32) {
          ++in;  // skip alpha
        }

        // switch RGB to BGR here...  DIB sections are in BGR
        *row++ = blue;
        *row++ = green;
        *row++ = red;
      }
    }

    SelectObject(ImageDC, ImageBitmap);

    image->destroy();
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool NextImage()
{
    if (ImageListSize < 1) {
        CurrentImage = -1;
        return 0;
    }

    if (Random) {

        CurrentImage = rand() % ImageListSize;

    } else {

        CurrentImage++;
        if (CurrentImage > ImageListSize - 1) {
            CurrentImage = 0;
        }
    }

    return UpdateImage();
}

////////////////////////////////////////////////////////////////////////////////

bool PrevImage()
{
    if (ImageListSize < 1) {
        CurrentImage = -1;
        return 0;
    }

    if (Random) {

        CurrentImage = rand() % ImageListSize;

    } else {

        CurrentImage--;
        if (CurrentImage < 0) {
            CurrentImage = ImageListSize - 1;
        }
    }

    return UpdateImage();
}

////////////////////////////////////////////////////////////////////////////////

void DrawShadedText(HDC dc, int x, int y, const char* text)
{
    SaveDC(dc);
    SetBkMode(dc, TRANSPARENT);

    SetTextColor(dc, 0x000000);
    TextOut(dc, x + 1, y + 1, text, strlen(text));

    SetTextColor(dc, 0xFFFFFF);
    TextOut(dc, x, y, text, strlen(text));

    RestoreDC(dc, -1);
}

////////////////////////////////////////////////////////////////////////////////

void ResetTimer(HWND window)
{
    KillTimer(window, TIMER_ID);
    if (Timer == 0) {
        SetTimer(window, TIMER_ID, 15, NULL);
    } else {
        SetTimer(window, TIMER_ID, 15, NULL);
    }
}

////////////////////////////////////////////////////////////////////////////////
