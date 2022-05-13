#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <libavcodec/avcodec.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}


typedef struct io_network_context {
    int fd;
} io_network_context;


static int recv_cb(void *buf, size_t buf_size, int flags, void *opaque) {
    io_network_context *ctx = (io_network_context*)opaque;
    return recv(ctx->fd, buf, buf_size, flags);
}


static int send_cb(const void *buf, size_t buf_size, int flags, void *opaque) {
    io_network_context *ctx = (io_network_context*)opaque;
    return send(ctx->fd, buf, buf_size, flags);
}


static void close_cb(void *opaque) {
    io_network_context *ctx = (io_network_context*)opaque;
    close(ctx->fd);
}


static void open_cb(int fd, AVIONetAdapter *adapter, void *opaque) {
    io_network_context *ctx = (io_network_context*)opaque;
    ctx->fd = fd;
    avio_init_net_adapter(adapter, &recv_cb, &send_cb, &close_cb);
}


int main(int argc, char **argv)
{
    io_network_context io_ctx;
    AVFormatContext *ifmt_ctx = NULL;
    AVPacket *pkt = NULL;
    const char *in_filename = "rtsp://oper:2m9846gUyB@10.1.0.17:554/Streaming/Channels/101?transportmode=unicast&profile=Profile_1";
    AVDictionary *opts = NULL;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    int ret = 0;

    pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "Could not allocate AVPacket\n");
        return 1;
    }

    if (!(ifmt_ctx = avformat_alloc_context())) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    ifmt_ctx->open_callback.callback = &open_cb;
    ifmt_ctx->open_callback.opaque = &io_ctx;

    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, &opts)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
        goto end;
    }



    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        goto end;
    }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

//    while (1) {
//        ret = av_read_frame(ifmt_ctx, pkt);
//        if (ret < 0)
//            break;
//
//        log_packet(ifmt_ctx, pkt, "in");
//    }

    end:
    av_packet_free(&pkt);

    avformat_close_input(&ifmt_ctx);

    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;
}
