#include "utils.h"

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
    } catch (const std::filesystem::filesystem_error& e) {}
    return content;
}

std::string OpenFolderDialog(HWND owner)
{
    std::string folderPath;

    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileDialog *pFileDialog = nullptr;

        // Create the FileOpenDialog object
        hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&pFileDialog));

        if (SUCCEEDED(hr))
        {
            // Set options to select folders
            DWORD dwOptions;
            pFileDialog->GetOptions(&dwOptions);
            pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

            // Show the dialog
            hr = pFileDialog->Show(owner);

            if (SUCCEEDED(hr))
            {
                IShellItem *pItem = nullptr;
                hr = pFileDialog->GetResult(&pItem);

                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath = nullptr;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    if (SUCCEEDED(hr))
                    {
                        // Convert to std::string
                        char path[MAX_PATH];
                        wcstombs(path, pszFilePath, MAX_PATH);
                        folderPath = path;
                        CoTaskMemFree(pszFilePath);
                    }

                    pItem->Release();
                }
            }

            pFileDialog->Release();
        }

        CoUninitialize();
    }

    return folderPath;
}

bool isVideoFile(const std::string &filepath)
{
    static const std::set<std::string> videoExts = {
        ".mp4", ".avi", ".mov", ".mkv", ".webm", ".flv", ".wmv", ".m4v"
    };

    std::string ext = std::filesystem::path(filepath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    return videoExts.find(ext) != videoExts.end();
}

bool isImageFile(const std::string &filepath) {
    static const std::set<std::string> imageExts = {
        ".jpg", ".jpeg", ".png", ".bmp", ".tga", ".psd",
        ".gif", ".hdr", ".pic", ".ppm", ".pgm", ".qoi"}
    ;

    std::string ext = std::filesystem::path(filepath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    return imageExts.find(ext) != imageExts.end();
}

MonitorDimensions getMonitorDimensions()
{
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