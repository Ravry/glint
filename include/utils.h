#pragma once
#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <windows.h>
#include <shlobj.h>
#include <commdlg.h>
#include <set>
#include <algorithm>
#include "imgui.h"
#include "log.h"

struct WorkerWs {
    HWND surW;
    HWND subW;
    HWND focus;
};

WorkerWs getWorkerwWindow();

std::string readFileContents(const char* filename);
std::vector<std::string> readDirectoryContent(const char* directory);
std::string OpenFolderDialog(HWND owner = nullptr);
bool isVideoFile(const std::string &filepath);
bool isImageFile(const std::string &filepath);

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