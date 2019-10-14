/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Application.c - Hosts the core terminal runtime processes.
--
-- PROGRAM: Dumb Terminal Emulator
--
-- FUNCTIONS:
-- LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wparam, LPARAM lparam);
-- VOID StartTerminal(HINSTANCE& hInst, int nCmdShow, HWND& hwnd, WNDCLASSEX& Wcl);
-- VOID SetupTerminalConfiguration(WNDCLASSEX& Wcl, HINSTANCE& hInst);
--
--
-- DATE: 2019-10-01
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Albert Liu
--
-- NOTES:
-- This file was designed to hold the general windows desktop application
-- runtime procedures.
----------------------------------------------------------------------------------------------------------------------*/

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
-- INTERFACE: int APIENTRY wWinMain( HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int nCmdShow)
--									hInstance:		Integer which identifies the internal data structure of the process.
--									hPrevInstance:	Should always be null, in 16-bit windows it was used to gather previous application instance data to make start up processes quicker.
--									lpCmdLine:		A string pointer which holds command line arguments held when the applications begins.
--									nCmdShow:		Specifies how the applications windows should be displayed to the OS
--													if no value is specified by the programmer/user.
--
-- RETURNS:		FALSE:				If the window instance is unable to be initialized.
--				(int) msg.wParam:	Represent the successful exit code of the process.
--
-- NOTES:
-- This function is responsible for running the functions responsible for setting up and initializing the application window
-- as well as starting and running the main message loop which receives and interprets all information directed towards the 
-- application before relaying the appropriate response.
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
--									hWnd: a unique handle for identifying the current window
--									message: the message being received by the window
--									wParam: contains the HIWORD (notification message) and LOWORD (control id which sent the message)
--									lParam: the window handle to the control which sent the message
--
-- RETURNS: returns 0 on complete of handling of message
--
-- NOTES:
-- This function receives all input directed at the window. Windows recognizes this function as a
-- callback function due to the pointer to this function in the class structure.
--
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
		// If a Comm port has been connected to, write the char to the port
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

			// Menu commands which allow for comm port connection. First checks if
			// an existing connection is active before connecting to the selected port.
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
			// Menu command to check to see if a receiver is connected to the comm port
			case ID_SCANTAG_SCANFORRECEIVER:
				scanForReceiver(hWnd);
				break;
			// Menu command to check if tags are being placed near the receiver.
			case ID_SCANTAG_SCANFORTAGS:
				startScan(hWnd);
				break;
			
			// Menu command to disconnect the receiver and ends all scanning processes.
			case ID_SCANTAG_STOPSCANFORTAGS:
				scanTags = false;
				break;
			// Menu command to close the window and end all application processes.
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	// Activates when another applications attempts to draw on the current application window.
    case WM_PAINT:
        {
			// Handling of window drawing logic
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
--
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: ATOM MyRegisterClass(HINSTANCE hInstance)
--									hInstance: Integer which identifies the internal data structure of the process.
--
-- RETURNS: ATOM: RegisterClassExW returns a class atom that uniquely identifies the class
--				being registered. If the function fails, the return value is zero.
--
-- NOTES: 
-- This function sets all the specified visual settings for the application interface prior to initialization.
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


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: InitInstance(HINSTANCE, int)
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
--									hInstance:	Integer which identifies the internal data structure of the process.
--									nCmdShow:	Specifies how the applications windows should be displayed to the OS
--												if no value is specified by the programmer/user.
--
-- RETURNS: BOOL:	TRUE: If the function is successfully able to create and display the application window.
					FALSE: If the a window is unable to be created.
--
-- NOTES:
-- In this function, we save the instance handle in a global variable and create and display the main program window.----------------------------------------------------------------------------------------------------------------------*/
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

