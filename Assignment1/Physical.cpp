/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Physical.cpp - Contains all the terminal logic which takes place inside the physical layer.
--
-- PROGRAM: Dumb Terminal Emulator
--
-- FUNCTIONS:
-- writeToFile(HWND hWnd,HANDLE hComm, WPARAM wParam)
-- HANDLE initializeSerialPort(LPCWSTR lpszCommName,HWND hWnd)
-- DWORD WINAPI readFromSerial(LPVOID hWnd)
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
#include "framework.h"
#include "Application.h"
#include "Session.h"
#include "Physical.h"
#include <string>



/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: writeToFile
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: void writeToFile(HWND hWnd,HANDLE hComm, WPARAM wParam)
--								hWnd:	A unique handle for the current window.
--								hComm:	A handle to the specified connected comm port.
--								wParam: contains the HIWORD (notification message) and LOWORD (control id which sent the message)
--
-- RETURNS: void
--
-- NOTES: 
-- This function is responsible for writing data to the comm port for information transferal.
----------------------------------------------------------------------------------------------------------------------*/
void writeToFile(HWND hWnd,HANDLE hComm, WPARAM wParam) {
	
	OVERLAPPED ov{ 0 };

	// Writes to File
	if (WriteFile(hComm, &wParam, 1, NULL, &ov)) {
		MessageBox(NULL, TEXT("Couldn't write to port"), NULL, MB_ICONERROR);
	};
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initializeSerialPort
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: HANDLE initializeSerialPort(LPCWSTR lpszCommName)
--											lpszCommName:	A long pointer to a zero terminated string containing the name
--															of the serial port to initialize.
--
-- RETURNS: hComm: Handle for the specified serial port once it is successfully started.
--
-- NOTES:
-- This function initializes a serial port. It does this by using CreateFile to return a handle to the specified comm port
-- and then using SetupComm to initialize its settings. Upon successful port initialization, all threads are killed and Connect
-- mode is turned off. Comm timeouts are then set to wait until bytes are available to read.
----------------------------------------------------------------------------------------------------------------------*/
HANDLE initializeSerialPort(LPCWSTR lpszCommName,HWND hWnd) {

	HANDLE hComm = CreateFile(lpszCommName, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	// Error in connecting to Port
	if (hComm == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, TEXT("Could not connect to port."), NULL, MB_ICONERROR);
		return INVALID_HANDLE_VALUE;
	}
	SetupComm(hComm, 1000, 1000);
	

	COMMCONFIG	cc;

	// Kills any active threads and turns Connect mode off
	if (threadActive)
		connectMode = false;

	cc.dwSize = sizeof(COMMCONFIG);
	GetCommConfig(hComm, &cc, &cc.dwSize);
	CommConfigDialog(lpszCommName, hWnd, &cc);
	SetCommState(hComm, &cc.dcb);

	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = 5; // in milliseconds
	timeouts.ReadTotalTimeoutConstant = 5; // in milliseconds
	timeouts.ReadTotalTimeoutMultiplier = 5; // in milliseconds
	timeouts.WriteTotalTimeoutConstant = 5; // in milliseconds
	timeouts.WriteTotalTimeoutMultiplier = 5; // in milliseconds


	SetCommTimeouts(hComm, &timeouts);
	return hComm;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readFromSerial
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: DWORD WINAPI readFromSerial(LPVOID hWnd)
--
-- RETURNS: 0: Indicates the end of the serial port reading thread.
--
-- NOTES:
-- This function contains the core logic that runs during "Connect Mode". It allows the application
-- to read data that enters through the 'connected' comm port and display it on screen.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI readFromSerial(LPVOID hWnd) {
	DWORD dwBytesToRead = 1;
	DWORD dwBytesRead = 0;
	DWORD dwEvent, dwError;
	COMSTAT cs;
	OVERLAPPED ovRead{ 0 };

	// Tries specify a set of events to be monitored for the communication device. If it can't,
	// a message box indicated the error is displayed.
	if (!SetCommMask(hComm, EV_RXCHAR)) {
		MessageBox(NULL, (LPCWSTR)"SetCommMask failed.", (LPCWSTR)"Error", MB_OK);
	}
	std::vector<std::string> charList;
	threadActive = true;

	// The loop containing all the port reading logic.
	while (connectMode) {
		char arr[2] = "";
		
		// Waits for the Event
		if (WaitCommEvent(hComm, &dwEvent, 0)) {
				ClearCommError(hComm, &dwError, &cs);
				if ((dwEvent & EV_RXCHAR) && cs.cbInQue) {
					if (!ReadFile(hComm, arr, dwBytesToRead, &dwBytesRead, &ovRead)) {
						MessageBox(NULL, TEXT("Issue with reading char from Com port"), NULL, MB_ICONERROR);
						continue;
					}
					else {
						if (dwBytesRead) {
							std::string tem(arr);

							if (*arr == '\b' && charList.size() != 0)
								charList.pop_back();
							else
								charList.push_back(tem);

							Draw((HWND)hWnd, charList);

						}
					}
				}
			
		}
		// Discards all chars from the output/input buffer of the comm port.
		PurgeComm(hComm, PURGE_RXCLEAR);
	}
	threadActive = false;
	return 0;
}

