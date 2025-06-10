#pragma once
#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "imgui.h"

void SetupImGuiStyle();

HWND getWorkerwWindow();

std::string readFileContents(const char* filename);
struct MonitorDimensions {
    int left = INT_MAX;
    int top = INT_MAX;
    int right = INT_MIN;
    int bottom = INT_MIN;
    int count = 0;

    int width() const { return right - left; }
    int height() const { return bottom - top; }
};

MonitorDimensions getMonitorDimensions();

// SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, NULL, SPIF_SENDCHANGE);