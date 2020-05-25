#ifndef _X_FFMPEG_SCREEN_PROCESS_H_
#define _X_FFMPEG_SCREEN_PROCESS_H_

#include <thread>

#include "x_ffmpeg_common.h"

namespace xM
{
    namespace ffmepg
    {
        typedef class ScreenEvent
        {
        public:
            virtual void HeaderInfo(int _pix_width, int _pix_height) {};
            virtual void FrametData(AVFrame* _frame) {};
            virtual void x264NalData(x264_nal_t* _nal,int _nal_num) {};
        }*PtrScreenEvent;

        class ScreenProcess
        {
        private:
            volatile bool run_flag_;
            PtrScreenEvent event_;
            std::thread process_stream_thread_;
        private:
            AVFormatContext* in_fmt_ctx_;
            AVCodecContext* in_cdc_ctx_;
            int video_index_;
        private:
            SwsContext* sws_ctx_;

            x264_param_t* ptr_param_;
            x264_t* ptr_encoder_;
            x264_nal_t* ptr_nal_;
            x264_picture_t* ptr_in_pic_;
            x264_picture_t* ptr_out_pic_;
        public:
            ScreenProcess();
            ~ScreenProcess();
        private:
            bool init_screen_stream();
            bool init_h264_encode();

            bool process_x264_encoder(AVFrame* _frame);
            void process_stream_function();
        public:
            bool Open(PtrScreenEvent _event);
            void Close();
        };
    }
}


#endif // _X_FFMPEG_SCREEN_PROCESS_H_