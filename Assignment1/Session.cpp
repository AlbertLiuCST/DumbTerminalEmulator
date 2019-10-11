
#include <windows.h>
#include <stdio.h>
#include "framework.h"
#include "Physical.h"
#include "Application.h"
#include "Session.h"


static LPSKYETEK_DEVICE* devices = NULL;
static LPSKYETEK_READER* readers = NULL;
SKYETEK_STATUS st;

bool scanReader;
bool scanTags;
bool readerFound;

unsigned int numDevices;
unsigned int numReaders;


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

		std::string str(lp);
		if (str == "\n") {
			x = 0;
			y = y + tm.tmHeight + tm.tmExternalLeading;
		}


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

void scanForReceiver(HWND hWnd)
{
	HMENU menu = GetMenu(hWnd);
	EnableMenuItem(menu, ID_SCANTAG_SCANFORRECEIVER, MF_DISABLED);

	// Sets scanning bool
	scanReader = true;
	while (scanReader)
	{
		numDevices = SkyeTek_DiscoverDevices(&devices);
		if (numDevices == 0)
		{
			Sleep(100);
			continue;
		}
		numReaders = SkyeTek_DiscoverReaders(devices, numDevices, &readers);
		if (numReaders == 0)
		{
			SkyeTek_FreeDevices(devices, numDevices);
			Sleep(100);
			continue;
		}
		MessageBox(NULL, TEXT("Reader Found"), NULL, MB_OK);
		scanReader = false;
		EnableMenuItem(menu, ID_SCANTAG_SCANFORTAGS, MF_ENABLED);
		break;
	}
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
--								LPCWSTR lpszCommName - Comm port to initalize 
--								HWND - Current Window 
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
--								HWND hWnd - Current Window
--								LPCWSTR - Com port name to open
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
void startScan(HWND hWnd) {

	connectMode = true;
	if (!threadActive) 
		rThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startScanForTags, (LPVOID)hWnd, 0, &rThreadId);
	
}
void startScanForTags(LPVOID hWnd)
{
	std::vector<std::string> vect;
	tagStruct te;
	te.hWnd = (HWND) hWnd;
	te.tagList = &vect;

	scanTags = true;
	st = SkyeTek_SelectTags(readers[0], AUTO_DETECT, SelectLoopCallback, 0, 1, (void*) &te);
}
unsigned char SelectLoopCallback(LPSKYETEK_TAG lpTag, void* user)
{
		if (scanTags)
			if (lpTag != NULL)
				drawTag(lpTag, ((tagStruct *) user)->hWnd, ((tagStruct*)user)->tagList);

		return(scanTags);
}

void drawTag(LPSKYETEK_TAG lpTag, HWND hWnd, std::vector<std::string> *vec) {

	std::string tagId;
	std::string tagType;
	TCHAR* id = lpTag->friendly;
	while (*id != '\0') {
		tagId = *id++;
		vec->push_back(tagId);
	}
	vec->push_back(" ");
	TCHAR* type = SkyeTek_GetTagTypeNameFromType(lpTag->type);
	while (*type != '\0') {
		tagType = *type++;
		vec->push_back(tagType);
	}
	vec->push_back("\n");
	Draw(hWnd, *vec);
}
