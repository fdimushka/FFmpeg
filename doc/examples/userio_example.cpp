#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavformat/url.h"
#include "libavformat/version.h"


typedef struct TCPContext {

//    int fd;
    int open_timeout;
    int rw_timeout;
//    int listen_timeout;
//    int recv_buffer_size;
//    int send_buffer_size;
//    int tcp_nodelay;
//#if !HAVE_WINSOCK2_H
//    int tcp_mss;
//#endif /* !HAVE_WINSOCK2_H */
} TCPContext;

#define OFFSET(x) offsetof(TCPContext, x)
#define D AV_OPT_FLAG_DECODING_PARAM
#define E AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
//    { "listen",          "Listen for incoming connections",  OFFSET(listen),         AV_OPT_TYPE_INT, { .i64 = 0 },     0,       2,       .flags = D|E },
        {"timeout", "set timeout (in microseconds) of socket I/O operations", OFFSET(rw_timeout), AV_OPT_TYPE_INT, {.i64 = -1}, -1, INT_MAX, .flags =
        D | E},
//    { "listen_timeout",  "Connection awaiting timeout (in milliseconds)",      OFFSET(listen_timeout), AV_OPT_TYPE_INT, { .i64 = -1 },         -1, INT_MAX, .flags = D|E },
//    { "send_buffer_size", "Socket send buffer size (in bytes)",                OFFSET(send_buffer_size), AV_OPT_TYPE_INT, { .i64 = -1 },         -1, INT_MAX, .flags = D|E },
//    { "recv_buffer_size", "Socket receive buffer size (in bytes)",             OFFSET(recv_buffer_size), AV_OPT_TYPE_INT, { .i64 = -1 },         -1, INT_MAX, .flags = D|E },
//    { "tcp_nodelay", "Use TCP_NODELAY to disable nagle's algorithm",           OFFSET(tcp_nodelay), AV_OPT_TYPE_BOOL, { .i64 = 0 },             0, 1, .flags = D|E },
//#if !HAVE_WINSOCK2_H
//    { "tcp_mss",     "Maximum segment size for outgoing TCP packets",          OFFSET(tcp_mss),     AV_OPT_TYPE_INT, { .i64 = -1 },         -1, INT_MAX, .flags = D|E },
//#endif /* !HAVE_WINSOCK2_H */
        {NULL}
};

static const AVClass tcp_class = {
        .class_name = "tcp",
        .item_name  = av_default_item_name,
        .option     = options,
        .version    = LIBAVUTIL_VERSION_INT,
};

/* return non zero if error */
static int tcp_open(URLContext *h, const char *uri, int flags) {
    std::cout << uri << std::endl;
    TCPContext *s = (TCPContext *)h->priv_data;
    s->open_timeout = 10;
    s->rw_timeout = 10;
    return 0;
}

static int tcp_accept(URLContext *s, URLContext **c) {
    return 0;
}

static int tcp_read(URLContext *h, uint8_t *buf, int size) {
    TCPContext *s = (TCPContext *)h->priv_data;
    return AVERROR_EOF;
}

static int tcp_write(URLContext *h, const uint8_t *buf, int size) {
    return AVERROR_EOF;
}

static int tcp_shutdown(URLContext *h, int flags) {
    return 0;
}

static int tcp_close(URLContext *h) {
    return 0;
}

static int tcp_get_file_handle(URLContext *h) {
    return 0;
}

static int tcp_get_window_size(URLContext *h) {
    return 0;
}

const URLProtocol ff_async_tcp_protocol = {
        .name                = "tcp",
        .url_open            = tcp_open,
        .url_accept          = NULL,
        .url_read            = tcp_read,
        .url_write           = tcp_write,
        .url_close           = tcp_close,
        .url_get_file_handle = tcp_get_file_handle,
        .url_get_short_seek  = tcp_get_window_size,
        .url_shutdown        = tcp_shutdown,
        .priv_data_class     = &tcp_class,
        .priv_data_size      = sizeof(TCPContext),
        .flags               = URL_PROTOCOL_FLAG_NETWORK,
};
}

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

//    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
//           tag,
//           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
//           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
//           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
//           pkt->stream_index);
}


typedef struct io_network_context {

} io_network_context;


static const struct URLProtocol * find_url_protocol(const char* filename, void *opaque) {
    //return &ff_async_tcp_protocol;
    return NULL;
}

int main(int argc, char **argv)
{
    io_network_context io_ctx;
    AVFormatContext *ifmt_ctx = NULL;
    AVPacket *pkt = NULL;
    const char *in_filename = "rtsp://oper:2m9846gUyB@10.1.0.17:554/Streaming/Channels/101";
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

    ifmt_ctx->find_protocol_callback.callback = &find_url_protocol;
    ifmt_ctx->find_protocol_callback.opaque = &io_ctx;

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
        //fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;
}
