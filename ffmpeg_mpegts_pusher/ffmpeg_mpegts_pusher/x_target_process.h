#ifndef _X_TARGET_PROCESS_H_
#define _X_TARGET_PROCESS_H_

#include "x_common.h"

class xTargetProcess
{
private:
	std::string tgt_url_;
	AVFormatContext* tgt_fmt_ctx_;
	std::map<int, int> inout_index_mapping_;

	int clock_stream_index_;
	double last_pts_;
	double last_update_time_;
public:
	xTargetProcess();
	~xTargetProcess();
public:
	bool NewOutput(const char* _url, const char* _type);
	bool BuildStreamByInput(AVFormatContext* _input_fmt_ctx, std::map<int, AVMediaType>& _input_stream_index, bool _dump_flg = false);

	bool WriteHeader();
	bool WriteTrailer();
	bool WritePacket(AVFormatContext* _input_fmt_ctx, AVPacket* _pkt, int _index);

	void Destroy();
};


#endif //_X_TARGET_PROCESS_H_