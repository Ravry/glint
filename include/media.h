#pragma once 
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "log.h"
#include "imgui_self.h"

inline AVFormatContext* fmtCtx {nullptr};
inline int videoStreamIndex {-1};
inline const AVCodec* codec {nullptr};
inline AVBufferRef* hwDeviceCtx {nullptr};
inline AVCodecContext* codecCtx {nullptr};
inline SwsContext* swsCtx {nullptr};
inline bool windowClosed {false};

struct FrameData {
    void* pixels {nullptr};
};

inline std::mutex frameDataMutex;
inline std::mutex frameQueueMutex;
inline std::queue<FrameData> frameQueue;
inline std::condition_variable frameCond;
constexpr size_t MAX_QUEUE_SIZE = 20;

void media_func(const char* filename);

uint8_t* getMediaThumbnail(const char* filename);