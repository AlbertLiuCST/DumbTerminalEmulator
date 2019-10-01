#pragma once


#include "Physical.h"
#include <vector>
#include <string>

void Draw(HWND hwnd, std::vector<std::string>charHistory);
void connectPort(LPCWSTR lpszCommName, HWND hWnd);
void configPort(HWND hWnd, LPCWSTR com);
