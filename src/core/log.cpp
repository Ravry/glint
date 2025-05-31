#include "log.h"

std::string addContextMessage(const std::string& msg, const char* file, int line, const char* func) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::ostringstream timeStream;
    timeStream << std::put_time(std::localtime(&now_c), "%H:%M:%S");
    
    std::string filename = std::filesystem::path(file).filename().string();

    std::ostringstream oss;
    oss << "[" << timeStream.str() << "|"<< filename << ":" << line << ":" << func << "] " << msg;
    return oss.str();
}