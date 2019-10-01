
#include <windows.h>
#include <stdio.h>
#include "framework.h"
#include "Physical.h"
#include "Application.h"
#include "Session.h"
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
--
-- RETURNS: void
--
-- NOTES: 
-- Writes to Comm port passing in WParam
----------------------------------------------------------------------------------------------------------------------*/
void writeToFile(HWND hWnd,HANDLE hComm, WPARAM wParam) {
	
	char str[2];

	// Convert char to string
	sprintf_s(str, "%c", LPCWSTR(wParam)); 
	
	if (*str == '\x1b') {
		connectMode = false;
	}
	// Writes to File
	if (!WriteFile(hComm, str, strlen(str), NULL, 0)) {
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
--
-- RETURNS: Handle when when serial port is successfully started
--
-- NOTES:
-- Initializes serial port
----------------------------------------------------------------------------------------------------------------------*/
HANDLE initializeSerialPort(LPCWSTR lpszCommName) {

	HANDLE hComm = CreateFile(lpszCommName, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, 0, NULL);

	// Error in connecting to Port
	if (hComm == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, TEXT("Could not connect to port."), NULL, MB_ICONERROR);
		return INVALID_HANDLE_VALUE;
	}
	SetupComm(hComm, 1000, 1000);
	
	DCB dcb;
	dcb.BaudRate = 2400;
	dcb.StopBits = 1;
	dcb.Parity = 0;

	if (SetCommState(hComm, &dcb)) 
		MessageBox(NULL, TEXT("Failed to CreateFile"), TEXT(""), MB_OK);
	

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
-- RETURNS: 0 when thread ends
--
-- NOTES:
-- Function is thrown into thread and constantly reads from port
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI readFromSerial(LPVOID hWnd) {
	DWORD dwBytesToRead = 1;
	DWORD dwBytesRead = 0;

	threadActive = true;
	while (connectMode) {
		char arr[2] = "";

		//TODO Better error Handling
		if (!ReadFile(hComm, arr, dwBytesToRead, &dwBytesRead, NULL)) {
			MessageBox(NULL, TEXT("Issue with reading char from Com port"), NULL, MB_ICONERROR);
			continue;
		};
		
		// Bytes were read, redraw with new character
		if (dwBytesRead) {
			std::string tem(arr);

			if (*arr == '\b' && charHistory.size() != 0) 
				charHistory.pop_back();
			else 
				charHistory.push_back(tem);

			Draw((HWND)hWnd, charHistory);
	
		}
		PurgeComm(hComm, PURGE_RXCLEAR);
	}
	threadActive = false;
	return 0;
}

