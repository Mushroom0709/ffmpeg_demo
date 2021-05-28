#include "x_player.h"


void sdl_audio_callback(void* userdata, Uint8* stream, int len)
{
	uint8_t* buf;
	int buf_size;
	int64_t timestamp;
	int write_len;
	xM::xPlayer* ptr_player = (xM::xPlayer*)userdata;

	AVFrame* frame = NULL;
	ptr_player->audio_frames_mtx_.lock();
	if (ptr_player->audio_frames_.size() > 0)
	{
		frame = ptr_player->audio_frames_.front();
		ptr_player->audio_frames_.pop();
	}
	ptr_player->audio_frames_mtx_.unlock();

	if (frame != NULL)
	{
		ptr_player->sample_transcoder_.Convert(frame, buf, buf_size, timestamp);
		write_len = len < buf_size ? len : buf_size;
		SDL_memset(stream, 0, len);
		SDL_MixAudio(stream, buf, write_len, SDL_MIX_MAXVOLUME);
		ptr_player->av_synchronization_.SetAudioClock(frame->pts * ptr_player->audio_decoder_.d_timebase_);
		ptr_player->sample_transcoder_.FreeBuffer(buf, buf_size);
		av_frame_free(&frame);
	}
	else
	{
		//如果没有声音缓冲，则静音
		SDL_memset(stream, 0, len);
	}
}

namespace xM
{
	xPlayer::xPlayer()
	{
		run_flag_ = false;

		memset(&sdl_audio_spec_, 0, sizeof(SDL_AudioSpec));
		sdl_window_ = NULL;
		sdl_renderer_ = NULL;
		sdl_texture_ = NULL;
		memset(&sdl_rect_, 0, sizeof(SDL_Rect));

		packet_ = NULL;
		pkt_stream_index_ = -1;
		pkt_status_ = -1;
		media_type_ = AVMEDIA_TYPE_UNKNOWN;
		vf_status_ = X_FFMPEG_STATUS_SEND_PKT;
		af_status_ = X_FFMPEG_STATUS_SEND_PKT;
		video_frame_ = NULL;
		audio_frame_ = NULL;
	}
	xPlayer::~xPlayer()
	{
		Destroy();
	}

	bool xPlayer::initialize_ffmepg(std::string _file)
	{
		int nb_streams = 0;
		if (false == demultiplexer_.Initialize(_file.c_str(), nb_streams))
			return false;
		if (nb_streams < 2)
			return false;

		if (false == video_decoder_.Initialize(
			demultiplexer_.FormatContext(),
			demultiplexer_.VideoIndexs()[0]))
			return false;

		if (false == audio_decoder_.Initialize(
			demultiplexer_.FormatContext(),
			demultiplexer_.AudioIndexs()[0]))
			return false;

		if (false == sdl_pixel_transcoder_.Initialize(
			video_decoder_.format_, video_decoder_.height_, video_decoder_.width_,
			AV_PIX_FMT_YUV420P, SDL_IMAGE_SIZE_HEIGHT, SDL_IMAGE_SIZE_WDITH))
			return false;

		if (false == console_pixel_transcoder_.Initialize(
			video_decoder_.format_, video_decoder_.height_, video_decoder_.width_,
			AV_PIX_FMT_GRAY8, CONSOLE_IMAGE_SIZE_HEIGHT, CONSOLE_IMAGE_SIZE_WDITH))
			return false;

		if (false == sample_transcoder_.Initialize(audio_decoder_.format_, audio_decoder_.ch_layout_, audio_decoder_.rate_, audio_decoder_.frame_size_,
			AVSampleFormat::AV_SAMPLE_FMT_S16, audio_decoder_.ch_layout_, audio_decoder_.rate_))
			return false;

		packet_ = av_packet_alloc();
		video_frame_ = av_frame_alloc();
		audio_frame_ = av_frame_alloc();

		return true;
	}
	bool xPlayer::initialize_sdl2()
	{
		SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

		sdl_window_ = SDL_CreateWindow(
			"xMushroom",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			SDL_IMAGE_SIZE_WDITH, SDL_IMAGE_SIZE_HEIGHT,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

		if (sdl_window_ == NULL)
			return false;

		sdl_renderer_ = SDL_CreateRenderer(sdl_window_, -1, SDL_RENDERER_SOFTWARE);
		if (sdl_renderer_ == NULL)
			return false;

		sdl_rect_.x = 0;
		sdl_rect_.y = 0;
		sdl_rect_.w = SDL_IMAGE_SIZE_WDITH;
		sdl_rect_.h = SDL_IMAGE_SIZE_HEIGHT;

		sdl_texture_ = SDL_CreateTexture(
			sdl_renderer_,
			SDL_PIXELFORMAT_IYUV,
			SDL_TEXTUREACCESS_STREAMING,
			SDL_IMAGE_SIZE_WDITH, SDL_IMAGE_SIZE_HEIGHT);
		if (sdl_texture_ == NULL)
			return false;

		sdl_audio_spec_.freq = sample_transcoder_.dst_sample_rate_;
		sdl_audio_spec_.format = AUDIO_S16SYS;
		sdl_audio_spec_.channels = av_get_channel_layout_nb_channels(sample_transcoder_.dst_ch_layout_);
		sdl_audio_spec_.silence = 0;
		sdl_audio_spec_.samples = sample_transcoder_.dst_nb_samples_;
		sdl_audio_spec_.userdata = this;
		sdl_audio_spec_.callback = sdl_audio_callback;

		if (SDL_OpenAudio(&sdl_audio_spec_, NULL) < 0)
			return false;

		return true;
	}
	bool xPlayer::initialize_console()
	{
		COORD size;
		CONSOLE_FONT_INFOEX font_info;
		COORD font_size = { 5,5 };
		console_out_ = GetStdHandle(STD_OUTPUT_HANDLE);
		HWND hwnd = GetForegroundWindow();

		font_info.cbSize = sizeof(CONSOLE_FONT_INFOEX);
		font_info.nFont = 0;
		font_info.dwFontSize.X = font_size.X;
		font_info.dwFontSize.Y = font_size.Y;
		font_info.FontFamily = TMPF_VECTOR;
		font_info.FontWeight = 400;
		size.X = 300;
		size.Y = 150;
		size.X = size.X > GetSystemMetrics(SM_CXMIN) ? size.X : GetSystemMetrics(SM_CXMIN);
		size.Y = size.Y > GetSystemMetrics(SM_CYMIN) ? size.Y : GetSystemMetrics(SM_CYMIN);

		BOOL RES = SetCurrentConsoleFontEx(console_out_, FALSE, &font_info);
		SetConsoleScreenBufferSize(console_out_, size);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, size.X * font_size.X, size.Y * font_size.Y, 0);

		return true;
	}

	void xPlayer::work_function_read_data()
	{
		int cnt = 0;
		while (run_flag_ && cnt < 5 && video_frames_.size() < 10)
		{
			if (true == demultiplexer_.GetPacket(packet_, media_type_, pkt_stream_index_, pkt_status_))
			{
				if (pkt_status_ == 0)
				{
					if (media_type_ == AVMEDIA_TYPE_VIDEO && pkt_stream_index_ == demultiplexer_.VideoIndexs()[0])
					{
						while (video_decoder_.GetFrame(packet_, video_frame_, vf_status_))
						{
							if (vf_status_ == X_FFMPEG_STATUS_SEND_PKT)
								break;
							else if (vf_status_ == X_FFMPEG_STATUS_READ_FRAME)
							{
								video_frames_.push(av_frame_clone(video_frame_));
								av_frame_unref(video_frame_);
							}
						}
					}
					else if (media_type_ == AVMEDIA_TYPE_AUDIO && pkt_stream_index_ == demultiplexer_.AudioIndexs()[0])
					{
						while (audio_decoder_.GetFrame(packet_, audio_frame_, af_status_))
						{
							if (af_status_ == X_FFMPEG_STATUS_SEND_PKT)
								break;
							else if (af_status_ == X_FFMPEG_STATUS_READ_FRAME)
							{
								audio_frames_mtx_.lock();
								audio_frames_.push(av_frame_clone(audio_frame_));
								audio_frames_mtx_.unlock();

								av_frame_unref(audio_frame_);
							}
						}
					}

					av_packet_unref(packet_);
					cnt++;
				}
			}
			else
			{
				run_flag_ = false;
			}
		}
	}

	void xPlayer::work_function_display_sdl(AVFrame* _frame)
	{
		uint8_t* dst_buf = NULL;
		AVFrame* dst = NULL;
		sdl_pixel_transcoder_.FillFrame(dst, dst_buf);

		sdl_pixel_transcoder_.Scale(_frame, dst);

		SDL_UpdateYUVTexture(sdl_texture_, NULL,
			dst->data[0], dst->linesize[0],
			dst->data[1], dst->linesize[1],
			dst->data[2], dst->linesize[2]);
		SDL_RenderClear(sdl_renderer_);
		SDL_RenderCopy(sdl_renderer_, sdl_texture_, NULL, &sdl_rect_);
		SDL_RenderPresent(sdl_renderer_);
		sdl_pixel_transcoder_.FreeFrame(dst, dst_buf);
	}
	void xPlayer::work_function_display_console(AVFrame* _frame)
	{
		const char* ascii_buf = " `'""^.,-*<>CO()l!][ivaLVPqohSmM$@#";
		uint8_t* dst_buf = NULL;
		AVFrame* dst = NULL;
		console_pixel_transcoder_.FillFrame(dst, dst_buf);

		console_pixel_transcoder_.Scale(_frame, dst);
		int buf_len = dst->width * dst->height;

		COORD w_pos = { 0, 0 };
		DWORD res_len = 0;
		uint8_t* console_buf = (uint8_t*)calloc((buf_len), sizeof(uint8_t));
		for (int i = 0; i < buf_len; i++)
		{
			console_buf[i] = ascii_buf[dst->data[0][i] / strlen(ascii_buf)];
		}

		for (int i = 0; i < dst->height; i++)
		{
			w_pos.Y = i;
			WriteConsoleOutputCharacterA(
				console_out_,
				(LPCSTR)(console_buf + i * dst->linesize[0]),
				dst->linesize[0],
				w_pos,
				&res_len);
		}
		free(console_buf);
		console_pixel_transcoder_.FreeFrame(dst, dst_buf);
	}
	void xPlayer::work_function_display()
	{
		if (video_frames_.size() <= 0)
			return;
		AVFrame* frame = video_frames_.front();
		video_frames_.pop();

		work_function_display_console(frame);
		work_function_display_sdl(frame);

		int64_t delay = av_synchronization_.ComputationalDelay(frame->pts * video_decoder_.d_timebase_);
		if (delay > 0)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(delay));
		}

		av_synchronization_.SetCycleStart();

		av_frame_free(&frame);
	}
	void xPlayer::work_function()
	{
		while (run_flag_)
		{
			work_function_read_data();
			if (run_flag_ == true)
				work_function_display();
		}
	}

	bool xPlayer::Initialize(std::string _file)
	{
		if (false == initialize_ffmepg(_file))
			return false;
		if (false == initialize_console())
			return false;
		if (false == initialize_sdl2())
			return false;
		return true;
	}
	void xPlayer::Work()
	{
		run_flag_ = true;
		work_thread_ = std::thread(&xPlayer::work_function, this);
		SDL_PauseAudio(0);
		SDL_Event s_event = { 0 };
		while (run_flag_)
		{
			memset(&s_event, 0, sizeof(SDL_Event));
			SDL_WaitEventTimeout(&s_event, 50);

			switch (s_event.type)
			{
			case SDL_QUIT:
			{
				run_flag_ = false;
			}
			break;

			default:
				break;
			}
		}

		Destroy();
	}
	void xPlayer::Destroy()
	{
		run_flag_ = false;

		if (work_thread_.joinable())
		{
			work_thread_.join();
		}
		// thread join

		if (sdl_window_ != NULL)
		{
			SDL_CloseAudio();
			SDL_DestroyTexture(sdl_texture_);
			SDL_DestroyRenderer(sdl_renderer_);
			SDL_DestroyWindow(sdl_window_);
		}


		sample_transcoder_.Destroy();
		sdl_pixel_transcoder_.Destroy();
		audio_decoder_.Destroy();
		video_decoder_.Destroy();
		demultiplexer_.Destroy();

		memset(&sdl_audio_spec_, 0, sizeof(SDL_AudioSpec));
		sdl_window_ = NULL;
		sdl_renderer_ = NULL;
		sdl_texture_ = NULL;
		memset(&sdl_rect_, 0, sizeof(SDL_Rect));

		if (packet_ != NULL)
			av_packet_free(&packet_);
		packet_ = NULL;
		pkt_stream_index_ = -1;
		pkt_status_ = -1;
		media_type_ = AVMEDIA_TYPE_UNKNOWN;
		vf_status_ = X_FFMPEG_STATUS_SEND_PKT;
		af_status_ = X_FFMPEG_STATUS_SEND_PKT;

		if (video_frame_ != NULL)
			av_frame_free(&video_frame_);
		if (audio_frame_ != NULL)
			av_frame_free(&audio_frame_);

		while (audio_frames_.size() > 0)
		{
			av_frame_free(&audio_frames_.front());
			audio_frames_.pop();
		}

		while (video_frames_.size() > 0)
		{
			av_frame_free(&video_frames_.front());
			video_frames_.pop();
		}
	}
}