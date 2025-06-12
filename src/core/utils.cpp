#include "utils.h"

// make sure to enable under performance settings -> visual effects -> animate controls and elements inside windows otherwise the wallpaper might not be placed behind the icons correctly
WorkerWs getWorkerwWindow() {
    HWND progmanHWND = FindWindow("Progman", 0);
    SendMessageTimeout(progmanHWND, 0x052C, 0, 0, SMTO_NORMAL, 1000, 0);

    WorkerWs workers {};

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        HWND shellView = FindWindowEx(hwnd, 0, "SHELLDLL_DefView", 0);

        if (shellView) {
            WorkerWs *result = reinterpret_cast<WorkerWs*>(lParam);
            result->surW = hwnd;
            result->subW = FindWindowEx(0, hwnd, "WorkerW", 0);
            return FALSE;
        }

        return TRUE;
    }, reinterpret_cast<LPARAM>(&workers));

    return workers;
}

std::string readFileContents(const char *filename)
{
    std::string fileContent;
    std::ifstream file;
    try {
        file.open(filename);
        if (!file) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return "";
        }
        std::stringstream fileStream;
        fileStream << file.rdbuf();
        file.close();
        fileContent = fileStream.str();
    } catch (std::ifstream::failure e) {
        std::cout << "failed to read source" << std::endl;
    }

    return fileContent;
}

std::vector<std::string> readDirectoryContent(const char *directory) {
    std::vector<std::string> content;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (std::filesystem::is_regular_file(entry.status())) {
                content.push_back(entry.path().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        THROW("filesystem error!");
    }
    return content;
}

std::string OpenFolderDialog(HWND owner)
{
    char path[MAX_PATH] = {0};

    BROWSEINFO bi = {0};
    bi.lpszTitle = "Select a folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.hwndOwner = owner;

    PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);
    if (pidl != nullptr)
    {
        if (SHGetPathFromIDList(pidl, path))
        {
            CoTaskMemFree(pidl);
            return std::string(path);
        }
        CoTaskMemFree(pidl);
    }

    return ""; // Canceled or failed
}

MonitorDimensions getMonitorDimensions() {
     MonitorDimensions monitorDimensions;

    EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
        MONITORINFOEX monitorInfo;
        monitorInfo.cbSize = sizeof(monitorInfo);

        MonitorDimensions* pMonitorDimensions = reinterpret_cast<MonitorDimensions*>(lParam);

        if (GetMonitorInfo(hMonitor, &monitorInfo)) {
            RECT rect = monitorInfo.rcMonitor;

            if (rect.left < pMonitorDimensions->left)
                pMonitorDimensions->left = rect.left;
            if (rect.top < pMonitorDimensions->top)
                pMonitorDimensions->top = rect.top;
            if (rect.right > pMonitorDimensions->right)
                pMonitorDimensions->right = rect.right;
            if (rect.bottom > pMonitorDimensions->bottom)
                pMonitorDimensions->bottom = rect.bottom;
        }

        pMonitorDimensions->count++;

        return TRUE;
    }, reinterpret_cast<LPARAM>(&monitorDimensions));

    return monitorDimensions;
}