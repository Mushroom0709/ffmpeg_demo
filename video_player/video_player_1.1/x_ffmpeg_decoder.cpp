#include "x_ffmpeg_decoder.h"

namespace x
{
    namespace ffmpeg
    {
		xDecoder::xDecoder()
		{
			av_dec_ctx_ = NULL;
			av_codec_ = NULL;

			d_timebase_ = 0.0;
			duration_ = 0.0;
			rate_ = 0.0;

			event_ = NULL;
			run_flag_ = false;
		}

		double xDecoder::GetTimebase()
		{
			return d_timebase_;
		}
		double xDecoder::GetDuration()
		{
			return duration_;
		}
		double xDecoder::GetRate()
		{
			return rate_;
		}
    }
}