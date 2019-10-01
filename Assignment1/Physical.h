#pragma once

#include <vector>
#include <string>

HANDLE initializeSerialPort(LPCWSTR lpszCommName);

void writeToFile(HWND hWnd, HANDLE hComm, WPARAM wParam);
DWORD WINAPI readFromSerial(LPVOID hWnd);


//Variables
static std::vector<std::string> charHistory;