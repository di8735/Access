#define WIN32_LEAN_AND_MEAN 
#pragma comment (lib, "ws2_32.lib")

#include "d3dUtility.h"
#include "..\\..\\Server\\Access Server\\protocol.h"
#define	BUF_SIZE				1024
#define	WM_SOCKET				WM_USER + 1