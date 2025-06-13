#include "media.h"

AVStream *videoStream{nullptr};
AVPacket *packet {nullptr};
AVFrame *frame {nullptr}; 
AVFrame *cpuFrame{nullptr};
uint8_t *scaled_data[4];
int scaled_linesize[4];
int width {0};
int height {0};
MyImGUI::MediaItem mediaItem;

bool find_video_stream_and_index(const char* filename) {
    if (avformat_open_input(&fmtCtx, filename, nullptr, nullptr) != 0)
        return false;

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0)
        return false;

    for (unsigned int i{0}; i < fmtCtx->nb_streams; i++)
    {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = fmtCtx->streams[i];
            videoStreamIndex = i;
            return true;
        }
    }

    return false;
}

void setup_codec_context() {
    codecCtx = avcodec_alloc_context3(codec);
    codecCtx->hw_device_ctx = av_buffer_ref(hwDeviceCtx);
    avcodec_parameters_to_context(codecCtx, videoStream->codecpar);
    avcodec_open2(codecCtx, codec, nullptr);
}

bool init_ffmpeg_video_params(const char *filename)
{
    if (find_video_stream_and_index(filename))
    {
        width = videoStream->codecpar->width;
        height = videoStream->codecpar->height;

        AVCodecParameters *codecPar = videoStream->codecpar;

        codec = avcodec_find_decoder_by_name("h264_cuvid");
        if (!codec)
            THROW("failed to find decoder!");

        if (av_hwdevice_ctx_create(&hwDeviceCtx, AV_HWDEVICE_TYPE_CUDA, nullptr, nullptr, 0) < 0)
            THROW("failed to create CUDA device context");

        setup_codec_context();

        swsCtx = sws_getContext(width, height, AV_PIX_FMT_NV12,
                                width, height, AV_PIX_FMT_RGBA,
                                SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

        packet = av_packet_alloc();
        frame = av_frame_alloc();
        cpuFrame = av_frame_alloc();

        av_image_alloc(scaled_data, scaled_linesize, 1920, 1080, AV_PIX_FMT_RGBA, 1);
    }
    
    return false;
}

void cleanup_ffmpeg_video_params() {
    av_freep(&scaled_data[0]);
    av_frame_free(&cpuFrame);
    av_frame_free(&frame);
    av_packet_free(&packet);

    sws_freeContext(swsCtx);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&fmtCtx);
}

void check_wether_media_updated() {
    bool updatedMedia { false };
    {
        std::lock_guard<std::mutex> lock(MyImGUI::sharedSettingsMutex);
        if (MyImGUI::sharedSettings.mediaFile.size() > 0)
        {
            mediaItem = MyImGUI::sharedSettings.mediaFile.front();
            updatedMedia = true;
            MyImGUI::sharedSettings.mediaFile.pop();
        }
    }

    if (updatedMedia) {
        if (mediaItem.type == GLINT_THUMBNAIL_FILE_TYPE_VIDEO) {
            avformat_close_input(&fmtCtx);
            avcodec_free_context(&codecCtx);
            find_video_stream_and_index(mediaItem.file.c_str());
            setup_codec_context();
        }
        else if (mediaItem.type == GLINT_THUMBNAIL_FILE_TYPE_IMAGE) {
            int _w, _h, _c;
            FrameData frameData { 
                .pixels = stbi_load(mediaItem.file.c_str(), &_w, &_h, &_c, 4),
                .width = _w,
                .height = _h
            };
            std::unique_lock<std::mutex> lock(frameQueueMutex);
            frameCond.wait(lock, [] { return frameQueue.size() < MAX_QUEUE_SIZE; });
            frameQueue.push(frameData);
        }
    }
}

void media_func(const char *filename) {
    av_log_set_level(AV_LOG_QUIET);
    init_ffmpeg_video_params(filename);

    while (!windowClosed) {
        if (frameQueue.size() > MAX_QUEUE_SIZE * 0.9) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        if (mediaItem.type == GLINT_THUMBNAIL_FILE_TYPE_VIDEO)
        {
            if (av_read_frame(fmtCtx, packet) < 0)
            {
                avcodec_flush_buffers(codecCtx);
                if (av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD) < 0) THROW("failed to seek back to start of the video!");
                continue;
            }

            if (packet->stream_index == videoStreamIndex)
            {
                if (avcodec_send_packet(codecCtx, packet) < 0)
                    continue;

                while (avcodec_receive_frame(codecCtx, frame) == 0)
                {
                    if (frame->format == AV_PIX_FMT_CUDA)
                    {
                        av_hwframe_transfer_data(cpuFrame, frame, 0);
                        sws_scale(swsCtx, cpuFrame->data, cpuFrame->linesize, 0, height, scaled_data, scaled_linesize);

                        FrameData frameData{
                            .pixels = malloc(width * height * 4),
                            .width = width,
                            .height = height
                        };
                        memcpy(frameData.pixels, scaled_data[0], width * height * 4);

                        {
                            std::unique_lock<std::mutex> lock(frameQueueMutex);
                            frameCond.wait(lock, [] { return frameQueue.size() < MAX_QUEUE_SIZE; });
                            frameQueue.push(frameData);
                        }
                    }
                }
            }

            av_packet_unref(packet);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        check_wether_media_updated();
    }

    cleanup_ffmpeg_video_params();
}




constexpr int dstWidth = 174;
constexpr int dstHeight = 97;

uint8_t* getMediaThumbnail(const char *filename)
{
    AVFormatContext* _formatCtx = nullptr;
    if (avformat_open_input(&_formatCtx, filename, nullptr, nullptr) < 0) {
        LOG(fmt::color::red, "Failed to open input file: {}\n", filename);
        return nullptr;
    }
    if (avformat_find_stream_info(_formatCtx, nullptr) < 0) {
        LOG(fmt::color::red, "Failed to find stream info {}\n", filename);
        return nullptr;
    }

    int videoStreamIndex = -1;
    AVStream* videoStream = nullptr;
    for (unsigned int i = 0; i < _formatCtx->nb_streams; i++) {
        if (_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = _formatCtx->streams[i];
            videoStreamIndex = i;
            break;
        }
    }
    if (!videoStream) {
        avformat_close_input(&_formatCtx);
        THROW("No video stream found");
    }

    const AVCodec* codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
    if (!codec) {
        avformat_close_input(&_formatCtx);
        THROW("Failed to find decoder");
    }

    AVCodecContext* _codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(_codecCtx, videoStream->codecpar);
    if (avcodec_open2(_codecCtx, codec, nullptr) < 0) {
        avcodec_free_context(&_codecCtx);
        avformat_close_input(&_formatCtx);
        THROW("Failed to open codec");
    }

    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* rgbFrame = av_frame_alloc();

    int width = _codecCtx->width;
    int height = _codecCtx->height;

    struct SwsContext* _swsCtx = sws_getContext(width, height, _codecCtx->pix_fmt,
                                               dstWidth, dstHeight, AV_PIX_FMT_RGBA,
                                               SWS_BILINEAR, nullptr, nullptr, nullptr);

    uint8_t* buffer = nullptr;
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, dstWidth, dstHeight, 1);
    buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, AV_PIX_FMT_RGBA, dstWidth, dstHeight, 1);

    uint8_t* thumbnail = nullptr;

    while (av_read_frame(_formatCtx, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            if (avcodec_send_packet(_codecCtx, packet) == 0) {
                if (avcodec_receive_frame(_codecCtx, frame) == 0) {
                    sws_scale(_swsCtx, frame->data, frame->linesize, 0, height, rgbFrame->data, rgbFrame->linesize);

                    thumbnail = (uint8_t*)malloc(numBytes);
                    memcpy(thumbnail, rgbFrame->data[0], numBytes);
                    av_packet_unref(packet);
                    break;
                }
            }
        }
        av_packet_unref(packet);
    }

    av_frame_free(&frame);
    av_frame_free(&rgbFrame);
    av_packet_free(&packet);
    sws_freeContext(_swsCtx);
    avcodec_free_context(&_codecCtx);
    avformat_close_input(&_formatCtx);

    if (!thumbnail) THROW("Failed to decode a frame");

    return thumbnail;
}

uint8_t* getImageThumbnail(const char* filepath) {
    int width, height, nrChannels;

    unsigned char* data = stbi_load(filepath, &width, &height, &nrChannels, 4);

    if (!data) return nullptr;

    unsigned char* resized_data = new unsigned char[dstWidth * dstHeight * 4];
    int result = stbir_resize_uint8(data, width, height, 0, resized_data, dstWidth, dstHeight, 0, 4);

    stbi_image_free(data);
    
    if (!result) return nullptr;

    return resized_data;
}