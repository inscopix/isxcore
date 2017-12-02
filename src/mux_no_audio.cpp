#include "isxMovie.h"
#include <iostream>

extern "C"
{
	#include <libavformat/avformat.h>
}

typedef struct VideoOutput
{
	AVStream *avs;
	AVCodecContext *avcc;
	int64_t pts;
	AVFrame *avf;
	AVFrame *avf0;
} VideoOutput;

namespace
{

AVFrame *allocateAVFrame(enum AVPixelFormat pixelFormat, int width, int height)
{
	AVFrame *avf;
	int ret;
	avf = av_frame_alloc();
	if (!avf)
		return NULL;
	avf->format = pixelFormat;
	avf->width = width;
	avf->height = height;

	ret = av_frame_get_buffer(avf, 32);
	if (ret < 0)
	{
		// log: "Could not allocate frame data."
		exit(1);
	}
	return avf;
}

void populatePixels(AVFrame *avf, int tIndex, int width, int height, isx::Image *inImg, const float inMinVal, const float inMaxVal)
{
	const bool rescaleDynamicRange = !(inMinVal == -1 && inMaxVal == -1);

	// Y
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			//avf->data[0][y * avf->linesize[0] + x] = 128 + 64 * (2 * (((x + tIndex) / 10) % 2) - 1)*(2 * (((y - tIndex) / 10) % 2) - 1);
			if (rescaleDynamicRange)
			{
				std::vector<float> val = inImg->getPixelValuesAsF32(isx::isize_t(y), isx::isize_t(x));
				avf->data[0][y * avf->linesize[0] + x] = uint8_t(255 * ((val[0] - inMinVal) / (inMaxVal - inMinVal)));
			}
			else
			{
				std::vector<float> val = inImg->getPixelValuesAsF32(isx::isize_t(y), isx::isize_t(x));
				avf->data[0][y * avf->linesize[0] + x] = uint8_t(255 * val[0]);
			}
		}
	}
	// Cb and Cr
	for (int y = 0; y < height / 2; y++)
	{
		for (int x = 0; x < width / 2; x++)
		{
			avf->data[1][y * avf->linesize[1] + x] = 128;
			avf->data[2][y * avf->linesize[2] + x] = 128;
		}
	}
}

int withinLoopUtility(AVFormatContext *avFmtCnxt, VideoOutput *vOut, bool validFrame = false, isx::Image *inImg = NULL, const float inMinVal = 1, const float inMaxVal = -1)
{
	int ret;
	AVFrame *avf;
	int got_packet = 0;
	AVPacket pkt = { 0 };
	AVCodecContext *avcc = vOut->avcc;

	////

	avf = NULL;

	if (validFrame)
	{
		if (av_frame_make_writable(vOut->avf) < 0)
		{
			exit(1);
		}
		if (vOut->avcc->pix_fmt != AV_PIX_FMT_YUV420P)
		{
			exit(1);
		}
		populatePixels(vOut->avf, int(vOut->pts), vOut->avcc->width, vOut->avcc->height, inImg, inMinVal, inMaxVal);
		vOut->avf->pts = vOut->pts++;
		avf = vOut->avf;
	}

	////

	av_init_packet(&pkt);
	ret = avcodec_encode_video2(avcc, &pkt, avf, &got_packet);
	if (ret < 0)
	{
		// log: "Error encoding video frame: " << ret
		exit(1);
	}
	if (got_packet)
	{
		av_packet_rescale_ts(&pkt, avcc->time_base, vOut->avs->time_base);
		pkt.stream_index = vOut->avs->index;
		ret = av_interleaved_write_frame(avFmtCnxt, &pkt);
	}
	else
	{
		ret = 0;
	}
	if (ret < 0)
	{
		// log: "Error while writing video frame: " << ret;
		exit(1);
	}
	return (avf || got_packet) ? 0 : 1;
}

}

bool preLoop(const char *filename, AVFormatContext * & avFmtCnxt, VideoOutput & vOut, const isx::Image *inImg)
{
	vOut = {0};
	int ret;
	AVDictionary *opt = NULL;
	avformat_alloc_output_context2(&avFmtCnxt, NULL, NULL, filename);
	if (!avFmtCnxt)
	{
		// log (non-err): "Could not deduce output format from file extension: using MPEG."
		avformat_alloc_output_context2(&avFmtCnxt, NULL, "mpeg", filename);
	}
	if (!avFmtCnxt)
	{
		return true;
	}
	AVOutputFormat *avOutFmt = avFmtCnxt->oformat;
	if (avOutFmt->video_codec == AV_CODEC_ID_NONE)
	{
		return true;
	}

	////

	AVCodec *codec = avcodec_find_encoder(avOutFmt->video_codec);
	if (!(codec))
	{
		// log: "Could not find encoder for " << avcodec_get_name(avOutFmt->video_codec)
		exit(1);
	}
	vOut.avs = avformat_new_stream(avFmtCnxt, NULL);
	if (!vOut.avs)
	{
		// log: "Could not allocate stream"
		exit(1);
	}
	vOut.avs->id = avFmtCnxt->nb_streams - 1;
	AVCodecContext *avcc = avcodec_alloc_context3(codec);
	if (avcc == NULL)
	{
		// log: "Could not alloc an encoding context"
		exit(1);
	}

	if (codec->type != AVMEDIA_TYPE_VIDEO)
	{
		exit(1);
	}

	avcc->codec_id = avOutFmt->video_codec;
	avcc->bit_rate = 400000;
	// Resolution must be a multiple of two: FIX THIS!!!
	if (inImg)
	{
		avcc->width = int(inImg->getWidth());
		avcc->height = int(inImg->getHeight());
		avcc->width -= (avcc->width % 2);
		avcc->height -= (avcc->height % 2);
	}
	else
	{
		// PLACEHOLDER: FIX THIS!!!
	}

	avcc->time_base.num = 1;
	avcc->time_base.den = 15; // frame-rate: FIX THIS!!!
	
	avcc->gop_size = 12; // max intra frame period: FIX THIS!!!
	avcc->pix_fmt = AV_PIX_FMT_YUV420P;
	if (avcc->codec_id == AV_CODEC_ID_MPEG2VIDEO)
	{
		avcc->max_b_frames = 2;
	}
	if (avcc->codec_id == AV_CODEC_ID_MPEG1VIDEO)
	{
		avcc->mb_decision = 2;
	}

	if (avFmtCnxt->oformat->flags & AVFMT_GLOBALHEADER)
	{
		avcc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	vOut.avcc = avcc;
	vOut.avs->time_base = vOut.avcc->time_base;

	////

	AVDictionary *opt2 = NULL;
	av_dict_copy(&opt2, opt, 0);
	ret = avcodec_open2(vOut.avcc, codec, &opt2);
	av_dict_free(&opt2);
	if (ret < 0)
	{
		// log: "Could not open video codec: " << ret;
		exit(1);
	}

	vOut.avf = allocateAVFrame(vOut.avcc->pix_fmt, vOut.avcc->width, vOut.avcc->height);
	if (vOut.avf == NULL)
	{
		// log: "Could not allocate video frame"
		exit(1);
	}

	vOut.avf0 = NULL;
	if (vOut.avcc->pix_fmt != AV_PIX_FMT_YUV420P)
	{
		vOut.avf0 = allocateAVFrame(AV_PIX_FMT_YUV420P, vOut.avcc->width, vOut.avcc->height);
		if (vOut.avf0 == NULL)
		{
			// log: "Could not allocate temporary picture"
			exit(1);
		}
	}

	ret = avcodec_parameters_from_context(vOut.avs->codecpar, vOut.avcc);
	if (ret < 0)
	{
		// log: "Could not copy the stream parameters"
		exit(1);
	}

	////

	av_dump_format(avFmtCnxt, 0, filename, 1);
	if (!(avOutFmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&avFmtCnxt->pb, filename, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			// log: "Could not open: " << filename << ret
			return true;
		}
	}
	ret = avformat_write_header(avFmtCnxt, &opt);
	if (ret < 0)
	{
		// log: "Error occurred when opening output file: " << ret
		return true;
	}
	return false;
}

bool withinLoop(AVFormatContext *avFmtCnxt, VideoOutput *vOut, isx::Image *inImg, const float inMinVal, const float inMaxVal)
{
	withinLoopUtility(avFmtCnxt, vOut, true, inImg, inMinVal, inMaxVal);
	return false;
}

bool postLoop(AVFormatContext *avFmtCnxt, VideoOutput & vOut)
{
	int done = 0;
	while (!done)
	{
		done = withinLoopUtility(avFmtCnxt, &vOut);
	}

	AVOutputFormat *avOutFmt = avFmtCnxt->oformat;

	av_write_trailer(avFmtCnxt);
	avcodec_free_context(&(vOut.avcc));
	av_frame_free(&(vOut.avf));
	av_frame_free(&(vOut.avf0));

	if (!(avOutFmt->flags & AVFMT_NOFILE))
	{
		avio_closep(&(avFmtCnxt->pb));
	}
	avformat_free_context(avFmtCnxt);
	return false;
}

int outputMp4Movie()
{
	const char *filename = "dan99.mp4";
	VideoOutput vOut;
	AVFormatContext *avFmtCnxt;

	if (preLoop(filename, avFmtCnxt, vOut, NULL))
	{
		return 1;
	}

	int done = 0;
	int indFrame = 0;
	int numFrames = 100;
	while (!done)
	{
		AVRational temp;
		temp.num = 1;
		temp.den = 1;
		bool validFrame = indFrame < numFrames; // av_compare_ts(vOut.pts, vOut.avcc->time_base, 20, temp) < 0;
		done = withinLoopUtility(avFmtCnxt, &vOut, validFrame);
		indFrame++;
	}

	if (postLoop(avFmtCnxt, vOut))
	{
		return 1;
	}

	return 0;
}
