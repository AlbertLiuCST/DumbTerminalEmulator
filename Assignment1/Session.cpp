
#include <windows.h>
#include <stdio.h>
#include "framework.h"
#include "Physical.h"
#include "Application.h"
#include "Session.h"


void Draw(HWND hwnd, std::vector<std::string> charHistory) {
	TEXTMETRIC tm;
	HDC hdc = GetDC(hwnd);
	SIZE size;

	InvalidateRect((HWND)hwnd, NULL, TRUE);
	GetTextMetrics(hdc, &tm);
	int x = 0, y = 0;

	for (size_t i = 0; i < charHistory.size(); i++) {

		// String to LPCSTR
		LPCSTR lp = charHistory[i].c_str();

		//Grab size
		GetTextExtentPoint32(hdc, (LPCWSTR)lp, strlen(lp), &size);
		RECT rect;
		// If x is too long is too large reset X and Y before painting to screen
		if (GetWindowRect((HWND)hwnd, &rect) && x < rect.right - rect.left - 30) {
			TextOut(hdc, x, y, (LPCWSTR)lp, strlen(lp));
		}else {
			y = y + tm.tmHeight + tm.tmExternalLeading; // next line 
			x = 0;
			TextOut(hdc, x, y, (LPCWSTR)lp, strlen(lp));
		}
		x += size.cx;
	}
	ReleaseDC(hwnd, hdc); // Release device context
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: connectPort
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: void connectPort(LPCWSTR lpszCommName, HWND hWnd)
--
-- RETURNS: void
--
-- NOTES:
-- Used to start 'Connect Mode State'. Closes any previously open Com ports and re-creates
----------------------------------------------------------------------------------------------------------------------*/
void connectPort(LPCWSTR lpszCommName, HWND hWnd)
{
	connectMode = true;
	CloseHandle(hComm);
	hComm = initializeSerialPort(lpszCommName);
	if (!threadActive) {
		rThread = CreateThread(NULL, 0, readFromSerial, (LPVOID)hWnd, 0, &rThreadId);
	}
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: configPort
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: void configPort(HWND hWnd,LPCWSTR com)
--
-- RETURNS: void
--
-- NOTES:
-- Kills connect mode and enables comm config. 
----------------------------------------------------------------------------------------------------------------------*/
void configPort(HWND hWnd,LPCWSTR com)
{
	COMMCONFIG	cc;
	CloseHandle(hComm);

	// Kills any active threads and turns Connect mode off
	if (threadActive) 
		connectMode = false;

	//TODO - Handle Better if port is invalid
	if ((hComm = initializeSerialPort(com)) == INVALID_HANDLE_VALUE) 
		return;
	
	

	cc.dwSize = sizeof(COMMCONFIG);
	GetCommConfig(hComm, &cc, &cc.dwSize);
	CommConfigDialog(com, hWnd, &cc);
	SetCommState(hComm, &cc.dcb);
}
