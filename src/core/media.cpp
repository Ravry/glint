#include "media.h"

void media_func(const char* filename) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    av_log_set_level(AV_LOG_QUIET);

    if (avformat_open_input(&fmtCtx, filename, nullptr, nullptr) != 0) THROW("could not open file");

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) THROW("could not find stream info");

    AVStream* videoStream {nullptr};

    for (unsigned int i {0}; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = fmtCtx->streams[i];
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStream) {
        int width = videoStream->codecpar->width;
        int height = videoStream->codecpar->height;
        int64_t duration = fmtCtx->duration/AV_TIME_BASE;
        int64_t total_frames = videoStream->nb_frames;

        LOG(fmt::color::brown, "video metadata: width: {}; height: {} | duration: {}s | frames: {}\n", width, height, duration, total_frames);
    
        AVCodecParameters* codecPar = videoStream->codecpar;
        
        codec = avcodec_find_decoder_by_name("h264_cuvid");
        if (!codec) THROW("failed to find decoder!");

        if (av_hwdevice_ctx_create(&hwDeviceCtx, AV_HWDEVICE_TYPE_CUDA, nullptr, nullptr, 0) < 0) THROW("failed to create CUDA device context");

        codecCtx = avcodec_alloc_context3(codec);
        codecCtx->hw_device_ctx = av_buffer_ref(hwDeviceCtx);
        avcodec_parameters_to_context(codecCtx, codecPar);
        avcodec_open2(codecCtx, codec, nullptr);

        AVPacket* packet = av_packet_alloc();
        AVFrame* frame = av_frame_alloc();

        swsCtx = sws_getContext(width, height, AV_PIX_FMT_NV12,
                                width, height, AV_PIX_FMT_RGBA,
                                SWS_BILINEAR, nullptr, nullptr, nullptr);

        AVFrame* cpuFrame = av_frame_alloc();
        uint8_t* scaled_data[4];
        int scaled_linesize[4];

        av_image_alloc(scaled_data, scaled_linesize, 1920, 1080, AV_PIX_FMT_RGBA, 1);


        while (!windowClosed) {
            if (frameQueue.size() > MAX_QUEUE_SIZE * 0.9) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            if(av_read_frame(fmtCtx, packet) < 0) {
                avcodec_flush_buffers(codecCtx);
                if (av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD) < 0) THROW("failed to seek back to start of the video!");
                continue;
            }
            
            if (packet->stream_index == videoStreamIndex) {
                if (avcodec_send_packet(codecCtx, packet) < 0) {
                    THROW("failed sending packet to decoder!");
                    continue;
                } 
                
                while (avcodec_receive_frame(codecCtx, frame) == 0) {
                    if (frame->format == AV_PIX_FMT_CUDA) {
                        av_hwframe_transfer_data(cpuFrame, frame, 0);

                        sws_scale(swsCtx, cpuFrame->data, cpuFrame->linesize, 0, height, scaled_data, scaled_linesize);
                        
                        FrameData frameData {};

                        frameData.pixels = malloc(width * height * 4);
                        memcpy(frameData.pixels, scaled_data[0], width * height * 4);
                        
                        {
                            std::unique_lock<std::mutex> lock(frameQueueMutex);        
                            frameCond.wait(lock, [] { return frameQueue.size() < MAX_QUEUE_SIZE; });
                            frameQueue.push(frameData);
                        }
            
                        // LOG(fmt::color::blanched_almond, "received frame from decoder: width={}; height={} | format={} | frames={}\n", frame->width, frame->height, frame->format, frameQueue.size());
                    }
                }
            }

            av_packet_unref(packet);
        }

        av_freep(&scaled_data[0]);
        av_frame_free(&cpuFrame);
        av_frame_free(&frame);
        av_packet_free(&packet);
    }

    sws_freeContext(swsCtx);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&fmtCtx);
}

uint8_t* getMediaThumbnail(const char* filename) {
    AVFormatContext* _formatCtx = nullptr;
    if (avformat_open_input(&_formatCtx, filename, nullptr, nullptr) < 0)
        THROW("Failed to open input file");
    if (avformat_find_stream_info(_formatCtx, nullptr) < 0)
        THROW("Failed to find stream info");

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
    
    int dstWidth = 174;
    int dstHeight = 97;

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