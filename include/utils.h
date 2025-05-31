#pragma once
#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

std::string readFileContents(const char* filename);
HWND getWorkerwWindow();

// SetParent(wallpaperWindow.getWindowHandle(), workerwHWND);
// SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, NULL, SPIF_SENDCHANGE);