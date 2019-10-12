#pragma once


#include "Physical.h"
#include <vector>
#include <string>
#include "SkyeTekAPI.h"
#include "SkyeTekProtocol.h"

void Draw(HWND hwnd, std::vector<std::string>charHistory);
void scanForReceiver(HWND hWnd);
void startScan(HWND hWnd);
void startScanForTags(LPVOID hWnd);
void connectPort(LPCWSTR lpszCommName, HWND hWnd);
unsigned char SelectLoopCallback(LPSKYETEK_TAG lpTag, void* user);

void drawTag(LPSKYETEK_TAG lpTag, HWND hWnd, std::vector<std::string> *vec);


// LPSKYETEK

/*
	Struct used to pass hWnd and tagList vector to SelectCallback Loop
*/
struct tagStruct {
	HWND hWnd;
	std::vector<std::string>* tagList;
};

extern bool scanTags;
extern bool readerFound;

extern unsigned int numDevices;
extern unsigned int numReaders;