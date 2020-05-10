#include "xframe.h"

xFrame::xFrame()
{
    dpts = 0.0;
    duration = 0.0;
    pts = 0;

    data_channel = -1;

    memset(linesize, 0, sizeof(int) * X_FRAME_MAX_CHANNEL);
    memset(data_len, 0, sizeof(int) * X_FRAME_MAX_CHANNEL);
    memset(data, 0, sizeof(uint8_t*) * X_FRAME_MAX_CHANNEL);
}
xFrame::~xFrame()
{
    Clear();
}
void xFrame::Clear()
{
    for (size_t i = 0; i < data_channel; i++)
    {
        if (data[i] != NULL)
        {
            free(data[i]);
        }
    }

    dpts = 0.0;
    duration = 0.0;
    pts = 0;

    data_channel = -1;

    memset(linesize, 0, sizeof(int) * X_FRAME_MAX_CHANNEL);
    memset(data_len, 0, sizeof(int) * X_FRAME_MAX_CHANNEL);
    memset(data, 0, sizeof(uint8_t*) * X_FRAME_MAX_CHANNEL);
}
void xFrame::CopyYUV(AVFrame* _data, int _w, int _h)
{
    this->data_channel = 3;
    memcpy(this->linesize, _data->linesize, sizeof(int) * 3);

    this->data_len[0] = _w * _h;
    this->data[0] = (uint8_t*)calloc(this->data_len[0], sizeof(uint8_t));
    if (this->data[0] != NULL)
        memcpy(this->data[0], _data->data[0], this->data_len[0] * sizeof(uint8_t));

    this->data_len[1] = _w * _h / 4;
    this->data[1] = (uint8_t*)calloc(this->data_len[1], sizeof(uint8_t));
    if (this->data[1] != NULL)
        memcpy(this->data[1], _data->data[1], this->data_len[1] * sizeof(uint8_t));

    this->data_len[2] = _w * _h / 4;
    this->data[2] = (uint8_t*)calloc(this->data_len[2], sizeof(uint8_t));
    if (this->data[2] != NULL)
        memcpy(this->data[2], _data->data[2], this->data_len[2] * sizeof(uint8_t));

}
void xFrame::CopyPCM(uint8_t* _buf, int _len)
{
    this->data_channel = 1;

    this->data_len[0] = _len;
    this->data[0] = (uint8_t*)calloc(this->data_len[0], sizeof(uint8_t));
    if (this->data[0] != NULL)
        memcpy(this->data[0], _buf, this->data_len[0] * sizeof(uint8_t));
}