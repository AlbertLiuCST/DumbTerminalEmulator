/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Session.c - Contains all the terminal logic which takes place inside the session layer.
--
-- PROGRAM: Dumb Terminal Emulator
--
-- FUNCTIONS:
-- void Draw(HWND hwnd, std::vector<std::string> charHistory)
-- void scanForReceiver(HWND hWnd)
-- void connectPort(LPCWSTR lpszCommName, HWND hWnd)
-- void startScan(HWND hWnd)
-- void startScanForTags(LPVOID hWnd)
-- unsigned char SelectLoopCallback(LPSKYETEK_TAG lpTag, void* user)
-- void drawTag(LPSKYETEK_TAG lpTag, HWND hWnd, std::vector<std::string> *vec)
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
-- This file contains functions pertaining to 
----------------------------------------------------------------------------------------------------------------------*/
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
bool scanStop = false;

unsigned int numDevices;
unsigned int numReaders;


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Draw
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: 
--
-- PROGRAMMER: 
--
-- INTERFACE: Draw(HWND hwnd, std::vector<std::string> charHistory)
--									hwnd:			
--									charHistory:	
--
-- RETURNS:		VOID
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: scanForReceiver
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER:
--
-- PROGRAMMER:
--
-- INTERFACE: scanForReceiver(HWND hWnd)
--									hwnd:									
--
-- RETURNS:		VOID
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/
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
		MessageBox(NULL, TEXT("Reader Found"), TEXT("Found"), MB_OK);
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
--								LPCWSTR lpszCommName - Comm port to initialize 
--								HWND - Current Window 
--
-- RETURNS: VOID
--
-- NOTES:
-- Used to start 'Connect Mode State'. Closes any previously open Com ports and re-creates
----------------------------------------------------------------------------------------------------------------------*/
void connectPort(LPCWSTR lpszCommName, HWND hWnd)
{
	connectMode = true;
	CloseHandle(hComm);
	hComm = initializeSerialPort(lpszCommName,hWnd);
	if (!threadActive) {
		rThread = CreateThread(NULL, 0, readFromSerial, (LPVOID)hWnd, 0, &rThreadId);
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startScan
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: startScan(HWND hWnd)
--									hwnd:									
--
-- RETURNS:		VOID
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/
void startScan(HWND hWnd) {

	connectMode = true;
	scanStop = false;
	if (!threadActive) 
		rThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startScanForTags, (LPVOID)hWnd, 0, &rThreadId);
	
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startScanForTags
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: void startScanForTags(LPVOID hWnd)
--									hwnd:
--
-- RETURNS:		VOID
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/
void startScanForTags(LPVOID hWnd)
{
	std::vector<std::string> vect;
	tagStruct te;
	te.hWnd = (HWND) hWnd;
	te.tagList = &vect;
	te.map = new std::unordered_map<std::string, int>();

	scanTags = true;
	st = SkyeTek_SelectTags(readers[0], AUTO_DETECT, SelectLoopCallback, 0, 1, (void*) &te);

}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: SelectLoopCallback
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: 
--
-- PROGRAMMER: 
--
-- INTERFACE: unsigned char SelectLoopCallback(LPSKYETEK_TAG lpTag, void* user)
--									lpTag:
--									user:
--
-- RETURNS:		VOID
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/
unsigned char SelectLoopCallback(LPSKYETEK_TAG lpTag, void* user)
{
		if (scanTags)
			if (lpTag != NULL)
				if(!tagExists(lpTag->friendly,user))
					drawTag(lpTag, ((tagStruct *) user)->hWnd, ((tagStruct*)user)->tagList);
		return(scanTags);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: drawTag
--
-- DATE: September 30, 2019
--
-- REVISIONS: September 30,2019
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: void drawTag(LPSKYETEK_TAG lpTag, HWND hWnd, std::vector<std::string> *vec)
--									lpTag:	
--									hWnd:	
--									*vec:	
--
-- RETURNS:		VOID
--
-- NOTES:
--
----------------------------------------------------------------------------------------------------------------------*/
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
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: tagExists
--
-- DATE: October 14, 2019
--
-- REVISIONS: October 14,2019
--
-- DESIGNER: Albert Liu
--
-- PROGRAMMER: Albert Liu
--
-- INTERFACE: boolean tagExists(TCHAR* id, void* user)
--									id: Id of tag to check
--									user: tagStruct to pass with tag reading session data
--
-- RETURNS:		boolean
--					true: if tag exists within tagStruct map
--					false: if tag doesn't exist within tagStruct map
--
-- NOTES: This function checks to see if key exists within unordered map contained in passed in tagStruct.
--		  If key exists returns true ,else false.
----------------------------------------------------------------------------------------------------------------------*/
boolean tagExists(TCHAR* id, void* user) {
	std::string tagId;
	while (*id != '\0') {
		tagId += *id++;
	}
	std::unordered_map<std::string, int> *tempMap = ((tagStruct*)user)->map;
	
	if (tempMap->find(tagId)== tempMap->end()){
		std::pair<std::string, int>tag(tagId, 1);
		tempMap->insert(tag);
		return false;
	}
	return true;
}
