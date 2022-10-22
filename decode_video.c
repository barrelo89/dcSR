/*
 * Copyright (c) 2001 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * video decoding with libavcodec API example
 *
 * @example decode_video.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <tensorflow/c/c_api.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image/stb_image.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image/stb_image_write.h"

#define INBUF_SIZE 4096

//ppm: color image format
void ppm_save(unsigned char *buf, int wrap, int ysize, char *filename)
{
  FILE *f;
  int i;

  f = fopen(filename, "w");

  //Unlike pgm, ppm contains the information about the three channels (R, G, B).
  //Thus, the wrap (linesize) should be three times of pgm, which makes the width of images linesize / 3.
  fprintf(f,"P6\n%d %d\n255\n", wrap / 3, ysize);
  for (i = 0; i < ysize; i++)
      fwrite(buf + i * wrap, 1, wrap, f);
  fclose(f);

}

//pgm: gray-scale image format
void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
    FILE *f;
    int i;

    f = fopen(filename,"w");

    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}

//function to convert yuv format into rgb format
//in this function, we save the decoded rgb frame as ppm format file
AVFrame* yuv420p_to_rgb24(AVFrame *frame, AVFrame *rgb_frame, char* filename){

  //rgb_frame = av_frame_alloc();
  int ret;

  int m_buffersize = avpicture_get_size(AV_PIX_FMT_RGB24, frame->width, frame->height);
  uint8_t *buffer = (uint8_t *)av_malloc(m_buffersize);

  //printf("Size: %d\n", m_buffersize);

  //init once
  struct SwsContext *imgConvertCtxYUVToRGB = sws_getContext(frame->width, frame->height, AV_PIX_FMT_YUV420P, frame->width, frame->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

  av_image_alloc(rgb_frame->data, rgb_frame->linesize, frame->width, frame->height, AV_PIX_FMT_RGB24, 32);

  //printf("Pixel Foramt: %d\n", rgb_frame->format);

  ret = sws_scale(imgConvertCtxYUVToRGB, frame->data, frame->linesize, 0, frame->height, rgb_frame->data, rgb_frame->linesize);

  av_free(buffer);

  //
  //printf("%d\t %d\t %d\n", rgb_frame->linesize[0], rgb_frame->linesize[1], rgb_frame->linesize[2]);
  //printf("Pixel Foramt: %d\n", rgb_frame->format);
  //printf("Result: %d\n", ret);

  //to write ppm file, we need to have rgb image format, not YUV
  ppm_save(rgb_frame->data[0], rgb_frame->linesize[0], ret, filename);

  return rgb_frame;

}

//decode function
static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt,
                   const char *filename)
{
    char buf[1024], buf_1[1024], buf_2[1024], buf_3[1024];
    int ret;

    ret = avcodec_send_packet(dec_ctx, pkt);

    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }


    while (ret >= 0) {

        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }

        //printf("Saving frame %3d\n", dec_ctx->frame_number);
        fflush(stdout);

        /* the picture is allocated by the decoder. no need to
           free it */

        /*Decoded Pixel Format: YUV 420P (AV_PIX_FMT_YUV420P = 0)
        in YUV 420P, the size of U, V field is a fourth of Y (both width and height is a half of Y)
        */

        snprintf(buf, sizeof(buf), "%s-%d-Y.pgm", filename, dec_ctx->frame_number);
        snprintf(buf_1, sizeof(buf), "%s-%d-Cr.pgm", filename, dec_ctx->frame_number);
        snprintf(buf_2, sizeof(buf), "%s-%d-Cb.pgm", filename, dec_ctx->frame_number);

        //linesize represents the width of each field (Y, U, and V)
        //printf("%d\t %d\t %d\n", frame->linesize[0], frame->linesize[1], frame->linesize[2]);

        //frame->data[0] contains the luma component
        pgm_save(frame->data[0], frame->linesize[0], frame->width, frame->height, buf);
        //frame->data[1] contains the blue component
        pgm_save(frame->data[1], frame->linesize[1], frame->width / 2, frame->height / 2, buf_1);
        //frame->data[2] contains the red component
        pgm_save(frame->data[2], frame->linesize[2], frame->width / 2, frame->height / 2, buf_2);

        snprintf(buf_3, sizeof(buf), "%s-%06d.ppm", filename, dec_ctx->frame_number);

        //we create a new avframe and then fill it with the decoded rgb frame and save it
        AVFrame *rgb_frame;
        rgb_frame = av_frame_alloc();
        rgb_frame = yuv420p_to_rgb24(frame, rgb_frame, buf_3);

    }
}

int main(int argc, char **argv)
{
    const char *filename, *outfilename;
    const AVCodec *codec;
    AVCodecParserContext *parser;
    AVCodecContext *c= NULL;
    FILE *f;
    AVFrame *frame;
    uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *data;
    size_t   data_size;
    int ret;
    AVPacket *pkt;

    if (argc <= 2) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(0);
    }
    filename    = argv[1]; //input_file_name: xxxx.h264
    outfilename = argv[2]; //output_img_name (prefix): ppm or outputfile_name

    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);

    /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
    memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    /* find the MPEG-1 video decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    parser = av_parser_init(codec->id);
    if (!parser) {
        fprintf(stderr, "parser not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    while (!feof(f)) {
        /* read raw data from the input file */
        data_size = fread(inbuf, 1, INBUF_SIZE, f);
        if (!data_size){
          break;
        }

        /* use the parser to split the data into frames */
        data = inbuf;
        while (data_size > 0) {
            ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                                   data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0) {
                fprintf(stderr, "Error while parsing\n");
                exit(1);
            }
            data      += ret;
            data_size -= ret;
            //printf("%d\n", pkt->size);

            if (pkt->size){
                //printf("Decoding Packets");
                decode(c, frame, pkt, outfilename);
                }
        }
    }

    /* flush the decoder */
    decode(c, frame, NULL, outfilename);

    fclose(f);

    av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    return 0;
}
