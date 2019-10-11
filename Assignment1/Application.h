#pragma once

#include "resource.h"
#include "Physical.h"

#include <vector>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <windows.h>
#include <stdio.h>

#define IDM_TX			100
#define IDM_RX			101
#define IDM_Port		102
#define IDM_Speed		103
#define IDM_Word		104
#define IDM_ZETA		105
#define IDM_Save		106
#define IDM_THETA		107
#define IDM_HELP		108
#define IDM_9600		111
#define IDM_32000		112
#define IDM_110000		113

#define IDM_COM1		201
#define IDM_COM2		202
#define IDM_COM3		203

#define ID_SCANTAG_SCANFORRECEIVER 301
#define ID_SCANTAG_SCANFORTAGS 302
#define ID_SCANTAG_STOPSCANFORTAGS 303

extern HANDLE hComm;
extern BOOL threadActive;
extern BOOL connectMode;

extern HANDLE rThread;
extern DWORD rThreadId;
