#ifndef _X_PLAYER_H_
#define _X_PLAYER_H_

#include <Windows.h>

#include <queue>
#include <string>
#include <thread>
#include <mutex>

#include <SDL.h>

#include "x_ffmpeg_demultiplexer.h"
#include "x_ffmpeg_video_decoder.h"
#include "x_ffmpeg_audio_decoder.h"
#include "x_ffmpeg_pixel_transcoder.h"
#include "x_ffmpeg_sample_transcoder.h"
#include "x_ffmpeg_audio_video_synchronization.h"


#define SDL_IMAGE_SIZE_WDITH 1280
#define SDL_IMAGE_SIZE_HEIGHT 720

#define CONSOLE_IMAGE_SIZE_WDITH 158
#define CONSOLE_IMAGE_SIZE_HEIGHT 85


namespace xM
{
	//typedef struct _X_AUDIO_FRAME_
	//{
	//	uint8_t* buffer_;
	//	int buffer_size_;
	//	int64_t timestamp_;
	//}X_AUDIO_FRAME,* PX_AUDIO_FRAME;

	class xPlayer
	{
	private:
		bool run_flag_;
	private:
		HANDLE console_out_;
	private:
		SDL_AudioSpec sdl_audio_spec_;
		SDL_Window* sdl_window_;
		SDL_Renderer* sdl_renderer_;
		SDL_Texture* sdl_texture_;
		SDL_Rect sdl_rect_;
	private:
		x_ffmpeg::Demultiplexer demultiplexer_;
		x_ffmpeg::VideoDecoder video_decoder_;
		x_ffmpeg::PixelTranscoder sdl_pixel_transcoder_;
		x_ffmpeg::PixelTranscoder console_pixel_transcoder_;
	public:
		x_ffmpeg::AudioDecoder audio_decoder_;
		x_ffmpeg::SampleTranscoder sample_transcoder_;
		x_ffmpeg::AudioVideoSynchronization av_synchronization_;
	private:
		std::queue<AVFrame*> video_frames_;
	public:
		std::queue<AVFrame*> audio_frames_;
		std::mutex audio_frames_mtx_;
	private:
		AVPacket* packet_;
		int pkt_stream_index_;
		int pkt_status_;
		AVMediaType media_type_;
		int vf_status_;
		int af_status_;
		AVFrame* video_frame_;
		AVFrame* audio_frame_;
	private:
		std::thread work_thread_;
	public:
		xPlayer();
		~xPlayer();
	public:
		bool Initialize(std::string _file);
		void Work();
		void Destroy();
	private:
		bool initialize_ffmepg(std::string _file);
		bool initialize_sdl2();
		bool initialize_console();

		void work_function_read_data();

		void work_function_display_sdl(AVFrame* _frame);
		void work_function_display_console(AVFrame* _frame);
		void work_function_display();
		void work_function();
	};
}

#endif // _X_PLAYER_H_