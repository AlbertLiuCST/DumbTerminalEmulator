
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
--
-- RETURNS: void
--
-- NOTES: 
-- Writes to Comm port passing in WParam
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
--
-- RETURNS: Handle when when serial port is successfully started
--
-- NOTES:
-- Initializes serial port
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
-- RETURNS: 0 when thread ends
--
-- NOTES:
-- Function is thrown into thread and constantly reads from port
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI readFromSerial(LPVOID hWnd) {
	DWORD dwBytesToRead = 1;
	DWORD dwBytesRead = 0;
	DWORD dwEvent, dwError;
	COMSTAT cs;
	OVERLAPPED ovRead{ 0 };

	if (!SetCommMask(hComm, EV_RXCHAR)) {
		MessageBox(NULL, (LPCWSTR)"SetCommMask failed.", (LPCWSTR)"Error", MB_OK);
	}
	std::vector<std::string> charList;
	threadActive = true;
	while (connectMode) {
		char arr[2] = "";
		
		// Waits for the Event
		if (WaitCommEvent(hComm, &dwEvent, 0)) {
				ClearCommError(hComm, &dwError, &cs);
				if ((dwEvent & EV_RXCHAR) && cs.cbInQue) {
					if (ReadFile(hComm, arr, dwBytesToRead, &dwBytesRead, &ovRead)) {
						MessageBox(NULL, TEXT("Issue with reading char from Com port"), NULL, MB_ICONERROR);
						continue;
					}
					else {
						if (cs.cbInQue) {
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

		PurgeComm(hComm, PURGE_RXCLEAR);
	}
	threadActive = false;
	return 0;
}

