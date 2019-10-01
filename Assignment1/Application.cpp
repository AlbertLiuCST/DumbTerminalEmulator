// Assignment1.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include "framework.h"
#include "Application.h"
#include "Session.h"
#include "Physical.h"
#include "resource.h"

#define MAX_LOADSTRING 100

using namespace std;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Connect Mode Booleans
BOOL threadActive = false;
BOOL connectMode = false;

// Comm port handle
HANDLE hComm;

// Thread Variables
HANDLE rThread;				
DWORD rThreadId;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: wWinMain
--
-- DATE: September 30, 2019 
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Aman Abdulla
--
-- INTERFACE: int APIENTRY wWinMain
--
-- RETURNS: int
--
-- NOTES:
-- winMain gets run on start up of application setsup window then runs msg loop to receive and handle messages
----------------------------------------------------------------------------------------------------------------------*/
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ASSIGNMENT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ASSIGNMENT1));

    MSG msg;
    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}
 


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WndProc
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
--
-- RETURNS: returns 0 on complete of handling of message
--
-- NOTES:
-- WM_CHAR     - handles key presses
-- WM_COMMAND  - process the application menu
-- WM_PAINT    - Paint the main window
-- WM_DESTROY  - post a quit message and return
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
    switch (message)
    {
	case WM_CHAR:	
		{
		if(connectMode)
			writeToFile(hWnd, hComm, wParam);
		break;
		}
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case ID_COM1:
				configPort(hWnd,TEXT("Com1"));
				break;
			case ID_COM2:
				configPort(hWnd, TEXT("Com2"));
				break;
			case ID_COM3:
				configPort(hWnd, TEXT("Com3"));
				break;
			case ID_CONNECT_COM1:

				if (connectMode) 
					MessageBox(NULL, TEXT("Already Connected"), TEXT(""), MB_OK);
				else 
					connectPort(TEXT("Com1"), hWnd);
				
				break;
			case ID_CONNECT_COM2:

				if (connectMode) 
					MessageBox(NULL, TEXT("Already Connected"), TEXT(""), MB_OK);
				else 
					connectPort(TEXT("Com2"), hWnd);
				
				break;
			case ID_CONNECT_COM3:

				if (connectMode) 
					MessageBox(NULL, TEXT("Already Connected"), TEXT(""), MB_OK);
				else 
					connectPort(TEXT("Com3"), hWnd);
				
				break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}





/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MyRegisterClass
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: Albert Liu
--y
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: ATOM MyRegisterClass(HINSTANCE hInstance)
--
-- RETURNS: returns 0 on complete of handling of message
--
-- NOTES: Register
----------------------------------------------------------------------------------------------------------------------*/
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASSIGNMENT1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ASSIGNMENT1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	
	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);

	UpdateWindow(hWnd);

	return TRUE;
}
