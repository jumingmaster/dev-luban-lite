/*
 * Copyright (C) 2020-2025 ArtInChip Technology Co. Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *  Author: <jun.ma@artinchip.com>
 *  Desc: mini_audio_player
 */

#include <string.h>
#include "aic_audio_decoder.h"
#include "aic_audio_render_manager.h"
#include "aic_parser.h"
#include "mini_audio_player.h"

#define _MINI_AUDIO_PLAYER_DEBUG_

#ifdef _MINI_AUDIO_PLAYER_DEBUG_
#define MINI_AUDIO_PLAYER_DEBUG(fmt,arg...)\
        printf("[%s:%d]"fmt"\n",__FUNCTION__,__LINE__,##arg)
#else
#define MINI_AUDIO_PLAYER_DEBUG(...)
#endif

#define MINI_AUDIO_PLAYER_ERROR(fmt,arg...)\
        printf("\033[40;31m[%s:%d]"fmt"\033[0m \n",__FUNCTION__,__LINE__,##arg)

enum MINI_AUDIO_PLAYER_MSG_TYPE {
    MINI_AUDIO_PLAYER_MSG_NONE = 0,
    MINI_AUDIO_PLAYER_MSG_START = 1,
    MINI_AUDIO_PLAYER_MSG_STOP = 2,
    MINI_AUDIO_PLAYER_MSG_PAUSE = 3,
    MINI_AUDIO_PLAYER_MSG_RESUME = 4,
    MINI_AUDIO_PLAYER_MSG_DESTROY = 5,
};

enum MINI_AUDIO_PLAYER_EVENT {
    MINI_AUDIO_PLAYER_EVENT_NONE = 0,
    MINI_AUDIO_PLAYER_EVENT_PLAY = 1,
    MINI_AUDIO_PLAYER_EVENT_STOP = 2,
    MINI_AUDIO_PLAYER_EVENT_PAUSE = 3,
    MINI_AUDIO_PLAYER_EVENT_RESUME = 4,
    MINI_AUDIO_PLAYER_EVENT_DESTROY = 5,
};

enum PLAYER_FILE_TYPE {
    PLAYER_FILE_MP3 = 0,
    PLAYER_FILE_WAV = 1,
    PLAYER_FILE_FLAC = 2,
    PLAYER_FILE_WMA = 3,
};

#define MINI_AUDIO_PLAYER_WAVE_BUFF_SIZE (4*1024)
#define MAX_URI_LEN 128
struct mini_audio_player {
    char uri[MAX_URI_LEN];
    int type;
    int fd;
    int state;
    int force_stop;
    int volume;
    char *wav_buff;
    int wav_buff_size;
    struct aic_audio_frame frame_info;
    struct aic_audio_decoder *decoder;
    struct aic_audio_decode_config dec_cfg;
    struct aic_audio_render *render;
    struct aic_parser *parser;
    struct mini_player_audio_info audio_info;
    aicos_thread_t tid;
    aicos_queue_t mq;
    aicos_mutex_t lock;
    aicos_sem_t sem_thread_exit;
    aicos_sem_t sem_ack;
    aicos_sem_t stop_ack;
    int loop;
};

struct mini_audio_player_msg {
    int type;
    void *data;
};

static int mini_audio_player_msg_send(struct mini_audio_player *player, int type, void *data)
{
    struct mini_audio_player_msg msg;

    msg.type = type;
    msg.data = data;

    return aicos_queue_send(player->mq, &msg);
}

static int play_start(struct mini_audio_player *player, char *uri)
{
    int result = 0;
    int len = strlen(uri)+1;
    if (len > MAX_URI_LEN)
        len = MAX_URI_LEN;

    aicos_mutex_take(player->lock, AICOS_WAIT_FOREVER);
    if (player->state != MINI_AUDIO_PLAYER_STATE_STOPED && player->state != MINI_AUDIO_PLAYER_STATE_INIT) {
        mini_audio_player_stop(player);
    }
    player->force_stop = 0;
    strncpy(player->uri, uri, len);
    result = mini_audio_player_msg_send(player, MINI_AUDIO_PLAYER_MSG_START, NULL);
    if (result != 1) {
        result = -1;
    }
    aicos_sem_take(player->sem_ack, AICOS_WAIT_FOREVER);
    aicos_mutex_give(player->lock);
    return result;
}

int mini_audio_player_play(struct mini_audio_player *player, char *uri)
{
    int result = 0;

    player->loop = 0;
    result = play_start(player, uri);

    return result;
}

int mini_audio_player_play_loop(struct mini_audio_player *player, char *uri)
{
    int result = 0;

    player->loop = 1;
    result = play_start(player, uri);

    return result;
}

int mini_audio_player_stop(struct mini_audio_player *player)
{
    int result = 0;

    aicos_mutex_take(player->lock, AICOS_WAIT_FOREVER);
    if (player->state != MINI_AUDIO_PLAYER_STATE_STOPED && player->state != MINI_AUDIO_PLAYER_STATE_INIT) {
        result = mini_audio_player_msg_send(player, MINI_AUDIO_PLAYER_MSG_STOP, NULL);
        if (result != 1) {
            result = -1;
        }
        player->force_stop = 1;
        aicos_sem_take(player->stop_ack, AICOS_WAIT_FOREVER);
    }
    aicos_mutex_give(player->lock);

    return result;
}

int mini_audio_player_pause(struct mini_audio_player *player)
{
    int result = 0;

    aicos_mutex_take(player->lock, AICOS_WAIT_FOREVER);
    if (player->state == MINI_AUDIO_PLAYER_STATE_PLAYING) {
        result = mini_audio_player_msg_send(player, MINI_AUDIO_PLAYER_MSG_PAUSE, NULL);
        if (result != 1) {
            result = -1;
        }
        aicos_sem_take(player->sem_ack, AICOS_WAIT_FOREVER);
    }
    aicos_mutex_give(player->lock);

    return result;
}

int mini_audio_player_resume(struct mini_audio_player *player)
{
    int result = 0;

    aicos_mutex_take(player->lock, AICOS_WAIT_FOREVER);
    if (player->state  == MINI_AUDIO_PLAYER_STATE_PAUSED)
    {
        result = mini_audio_player_msg_send(player, MINI_AUDIO_PLAYER_MSG_RESUME, NULL);
        if (result != 1) {
            result = -1;
        }
        aicos_sem_take(player->sem_ack,AICOS_WAIT_FOREVER);
    }
    aicos_mutex_give(player->lock);

    return result;
}

int mini_audio_player_get_media_info(struct mini_audio_player *player, struct mini_player_audio_info *audio_info)
{
    if (player == NULL || audio_info == NULL) {
        return -1;
    }
    memcpy(audio_info,&player->audio_info,sizeof(struct mini_player_audio_info));
    return 0;
}

int mini_audio_player_set_volume(struct mini_audio_player *player, int vol)
{
    int volume;

    volume = vol;
    if (volume < 0) {
        volume = 0;
    }
    if (volume > 100) {
        volume = 100;
    }
    player->volume = volume;
    if (player->render) {
        aic_audio_render_control(player->render, AUDIO_RENDER_CMD_SET_VOL, &player->volume);
    }
    return 0;
}

int mini_audio_player_get_volume(struct mini_audio_player *player,int *vol)
{
    if (vol == NULL) {
        return -1;
    }
    *vol = player->volume;
    return 0;
}

int mini_audio_player_get_state(struct mini_audio_player *player)
{
    return player->state;
}

static int mini_audio_player_open(struct mini_audio_player *player)
{
    int ret = 0;
    char* ptr = NULL;
    struct aic_parser_av_media_info media_info;
    struct aic_audio_render_attr attr;
    struct aic_audio_decode_params decode_params;
    struct audio_render_create_params create_params;

    if (player == NULL) {
        MINI_AUDIO_PLAYER_ERROR("para error\n");
        goto _exit;
    }
    ptr = strrchr(player->uri, '.');
    if (ptr == NULL) {
        MINI_AUDIO_PLAYER_ERROR("Invalid file uri\n");
        goto _exit;
    }
    if (!strncmp(ptr+1, "mp3", 3)) {
        player->type = PLAYER_FILE_MP3;
    } else if (!strncmp(ptr+1, "wav", 3)) {
        player->type = PLAYER_FILE_WAV;
        if (player->wav_buff == NULL) {
            player->wav_buff_size = MINI_AUDIO_PLAYER_WAVE_BUFF_SIZE;
            player->wav_buff = aicos_malloc(MEM_DEFAULT,player->wav_buff_size);
            if (player->wav_buff == NULL) {
                MINI_AUDIO_PLAYER_ERROR("aicos_malloc error\n");
                goto _exit;
            }
        }
    } else if (!strncmp(ptr+1, "flac", 4)) {
        player->type = PLAYER_FILE_FLAC;
    } else if (!strncmp(ptr+1, "wma", 3)) {
        player->type = PLAYER_FILE_WMA;
    } else {
        MINI_AUDIO_PLAYER_ERROR("unsupport file type\n");
        goto _exit;
    }

    aic_parser_create((unsigned char *)player->uri, &player->parser);
    if (player->parser == NULL) {
        MINI_AUDIO_PLAYER_ERROR("aic_parser_create fail\n");
        goto _exit;
    }
    if (aic_parser_init(player->parser)) {
        MINI_AUDIO_PLAYER_ERROR("aic_parser_init fail\n");
        goto _exit;
    }
    if (aic_parser_get_media_info(player->parser, &media_info)) {
        MINI_AUDIO_PLAYER_ERROR("aic_parser_get_media_info fail\n");
        goto _exit;
    }
    player->audio_info.file_size = media_info.file_size;
    player->audio_info.duration = media_info.duration;
    player->audio_info.sample_rate = media_info.audio_stream.sample_rate;
    player->audio_info.nb_channel = media_info.audio_stream.nb_channel;
    player->audio_info.bits_per_sample = media_info.audio_stream.bits_per_sample;

    if (player->type == PLAYER_FILE_MP3 ||
        player->type == PLAYER_FILE_FLAC ||
        player->type == PLAYER_FILE_WMA) {
        if (player->type == PLAYER_FILE_MP3)
            player->decoder = aic_audio_decoder_create(MPP_CODEC_AUDIO_DECODER_MP3);
        else if (player->type == PLAYER_FILE_FLAC)
            player->decoder = aic_audio_decoder_create(MPP_CODEC_AUDIO_DECODER_FLAC);
        else if (player->type == PLAYER_FILE_WMA)
            player->decoder = aic_audio_decoder_create(MPP_CODEC_AUDIO_DECODER_WMA);

        if (player->decoder == NULL) {
            MINI_AUDIO_PLAYER_ERROR("aic_audio_decoder_create fail\n");
            goto _exit;
        }
        player->dec_cfg.packet_buffer_size = 4*1024;
        player->dec_cfg.packet_count = 2;
        player->dec_cfg.frame_count = 2;
        ret = aic_audio_decoder_init(player->decoder, &player->dec_cfg);
        if (ret) {
            MINI_AUDIO_PLAYER_ERROR("aic_audio_decoder_init fail %d\n", ret);
            goto _exit;
        }

        decode_params.sample_rate = media_info.audio_stream.sample_rate;
        decode_params.bit_rate = media_info.audio_stream.bit_rate;
        decode_params.bits_per_sample = media_info.audio_stream.bits_per_sample;
        decode_params.channels = media_info.audio_stream.nb_channel;
        decode_params.block_align = media_info.audio_stream.block_align;
        decode_params.extradata = media_info.audio_stream.extra_data;
        decode_params.extradata_size = media_info.audio_stream.extra_data_size;
        ret = aic_audio_decoder_control(player->decoder, MPP_DEC_INIT_CMD_SET_PARAMS,
                                        &decode_params);
        if (ret) {
            MINI_AUDIO_PLAYER_ERROR("aic_audio_decoder_control set params fail %d\n", ret);
            goto _exit;
        }
    }

    create_params.dev_id = 0;
    create_params.scene_type = AUDIO_RENDER_SCENE_DEFAULT;
    aic_audio_render_create(&player->render, &create_params);
    if (player->render == NULL) {
        MINI_AUDIO_PLAYER_ERROR("aic_audio_render_create fail\n");
        goto _exit;
    }
    if (aic_audio_render_init(player->render)) {
        MINI_AUDIO_PLAYER_ERROR("aic_audio_render_init fail\n");
        goto _exit;
    }

    // set volume
    if (player->volume != 0) {
        aic_audio_render_control(player->render, AUDIO_RENDER_CMD_SET_VOL, &player->volume);
    } else {
        aic_audio_render_control(player->render, AUDIO_RENDER_CMD_GET_VOL, &player->volume);
    }

    // set attr
    attr.bits_per_sample = player->audio_info.bits_per_sample;
    attr.channels = player->audio_info.nb_channel;
    attr.sample_rate = player->audio_info.sample_rate;
    if (aic_audio_render_control(player->render, AUDIO_RENDER_CMD_SET_ATTR, &attr)) {
        MINI_AUDIO_PLAYER_ERROR("aic_audio_render_set_attr fail\n");
        goto _exit;
    }

    return 0;
_exit:
    if (player->parser) {
        aic_parser_destroy(player->parser);
        player->parser = NULL;
    }
    if (player->decoder) {
        aic_audio_decoder_destroy(player->decoder);
        player->decoder = NULL;
    }
    if (player->render) {
        aic_audio_render_destroy(player->render);
        player->render = NULL;
    }
    return -1;
}

static int mini_audio_player_close(struct mini_audio_player *player)
{
    if (player->parser) {
        aic_parser_destroy(player->parser);
        player->parser = NULL;
    }
    if (player->decoder) {
        aic_audio_decoder_destroy(player->decoder);
        player->decoder = NULL;
    }
    if (player->render) {
        aic_audio_render_destroy(player->render);
        player->render = NULL;
    }
    return 0;
}

static int mini_audio_playe_event_handler(struct mini_audio_player *player, int timeout)
{
    int event;
    int  result;
    struct mini_audio_player_msg msg;
#ifdef _MINI_AUDIO_PLAYER_DEBUG_
    int last_state;
#endif
    //1-ok,other-fail
    result = aicos_queue_receive(player->mq, &msg, timeout);
    if (result != 1) {
        event = MINI_AUDIO_PLAYER_MSG_NONE;
        return event;
    }
#ifdef _MINI_AUDIO_PLAYER_DEBUG_
    last_state = player->state;
#endif
    switch (msg.type) {
    case MINI_AUDIO_PLAYER_MSG_START:
        event = MINI_AUDIO_PLAYER_EVENT_PLAY;
        player->state = MINI_AUDIO_PLAYER_STATE_PLAYING;
        break;
    case MINI_AUDIO_PLAYER_MSG_STOP:
        event = MINI_AUDIO_PLAYER_EVENT_STOP;
        player->force_stop = 1;
        break;

    case MINI_AUDIO_PLAYER_MSG_PAUSE:
        event = MINI_AUDIO_PLAYER_EVENT_PAUSE;
        player->state = MINI_AUDIO_PLAYER_STATE_PAUSED;
        break;

    case MINI_AUDIO_PLAYER_MSG_RESUME:
        event = MINI_AUDIO_PLAYER_EVENT_RESUME;
        player->state = MINI_AUDIO_PLAYER_STATE_PLAYING;
        break;

    case MINI_AUDIO_PLAYER_MSG_DESTROY:
        event = MINI_AUDIO_PLAYER_EVENT_DESTROY;
        break;

    default:
        event = MINI_AUDIO_PLAYER_EVENT_NONE;
        break;
    }
    aicos_sem_give(player->sem_ack);

    MINI_AUDIO_PLAYER_DEBUG("%d %d->%d\n", event, last_state, player->state);

    return event;
}

// return val: 0 - break; 1 - continue
static int audio_decode(struct mini_audio_player *player)
{
    int eos = 0;
    int need_peek = 1;
    int parser_ret = 0;
    int decoder_ret = 0;
    struct aic_parser_packet parser_pkt;
    struct mpp_packet decoder_pkt;
    struct aic_audio_frame audio_frame = {0};

    if (!eos) {
        if (need_peek) {
            parser_ret = aic_parser_peek(player->parser, &parser_pkt);
        }
        if (parser_ret != PARSER_EOS) {
            decoder_pkt.size =  parser_pkt.size;
            decoder_ret = aic_audio_decoder_get_packet(player->decoder,&decoder_pkt,decoder_pkt.size);
            if (decoder_ret == DEC_OK) {
                parser_pkt.data = decoder_pkt.data;
                parser_pkt.flag = 0;
                aic_parser_read(player->parser,&parser_pkt);
                decoder_pkt.flag = parser_pkt.flag;
                aic_audio_decoder_put_packet(player->decoder, &decoder_pkt);
                need_peek = 1;
            } else {
                need_peek = 0;
            }
        } else {
            if (player->loop && (aic_parser_seek(player->parser, 0) >= 0)) {
                need_peek = 1;
                return 1;
            }
            eos = 1;
        }
    }
    if (player->force_stop)
        return 0;
    decoder_ret = aic_audio_decoder_decode(player->decoder);
    if (decoder_ret == DEC_NO_EMPTY_PACKET) {
        /*Decoder in multi-thread conditions, the decoder internal
            *packet buf may be fulled and returned no empty packet,
            *so should sleep a minute */
        usleep(5000);
    }
    decoder_ret = aic_audio_decoder_get_frame(player->decoder, &audio_frame);
    if (decoder_ret == DEC_OK) {
        aic_audio_render_rend(player->render, audio_frame.data, audio_frame.size);
        aic_audio_decoder_put_frame(player->decoder, &audio_frame);
        if (audio_frame.flag & PARSER_EOS) {
            if (player->loop)
                return 1;
            return 0;
        }
    } else if (eos && DEC_NO_RENDER_FRAME == decoder_ret &&
        !player->force_stop ) {
        /*Decoder in multithread conditions, should sleep until all data
            *decoding to complete*/
        usleep(5000);
    }
    return 1;
}

static void mini_audio_player_entry(void *parameter)
{
    int result;
    int event;
    struct aic_parser_packet parser_pkt;
    struct mini_audio_player *player = (struct mini_audio_player *)parameter;
    int parser_ret = 0;

    while(1) {
        event = mini_audio_playe_event_handler(player, AICOS_WAIT_FOREVER);
        if (event == MINI_AUDIO_PLAYER_EVENT_DESTROY) {
            goto _exit;
        }
        if (event != MINI_AUDIO_PLAYER_EVENT_PLAY) {
            continue;
        }

        /* open mp3 player */
        result = mini_audio_player_open(player);
        if (result != 0) {
            player->state = MINI_AUDIO_PLAYER_STATE_STOPED;
            MINI_AUDIO_PLAYER_ERROR("mini_audio_player_open failed\n");
            continue;
        }

        player->state = MINI_AUDIO_PLAYER_STATE_PLAYING;
        while(1) {
            event = mini_audio_playe_event_handler(player, 0);
            if (event == MINI_AUDIO_PLAYER_EVENT_NONE) {
                if (player->type != PLAYER_FILE_WAV) {
                    result = audio_decode(player);
                    if (result == 0)
                        break;
                    else
                        continue;
                } else {// wav
                    parser_ret = aic_parser_peek(player->parser, &parser_pkt);
                    if (parser_ret == PARSER_EOS) {
                        if (player->loop && (aic_parser_seek(player->parser, 0) >= 0)) {
                            continue;
                        }
                        break;
                    }
                    if (parser_pkt.size > player->wav_buff_size) {
                        MINI_AUDIO_PLAYER_ERROR("pkt size[%d] larger than wav_buf_size[%d]\n",parser_pkt.size,player->wav_buff_size);
                        break;
                    }
                    if (player->force_stop)
                        break;
                    parser_pkt.data = player->wav_buff;
                    aic_parser_read(player->parser, &parser_pkt);
                    if (player->force_stop)
                        break;
                    aic_audio_render_rend(player->render, parser_pkt.data, parser_pkt.size);
                }
            } else if (event == MINI_AUDIO_PLAYER_EVENT_PAUSE) {
                event = mini_audio_playe_event_handler(player, AICOS_WAIT_FOREVER);
                if (player->force_stop)
                        break;
            }
        }

        mini_audio_player_close(player);

        player->state = MINI_AUDIO_PLAYER_STATE_STOPED;
        if (player->force_stop) {
            aicos_sem_give(player->stop_ack);
        }
    }
_exit:
    aicos_sem_give(player->sem_thread_exit);
    return;
}

struct mini_audio_player* mini_audio_player_create(void)
{
    struct mini_audio_player *player = NULL;

    player = (struct mini_audio_player *)aicos_malloc(MEM_DEFAULT,sizeof(struct mini_audio_player));
    if (player == NULL) {
        MINI_AUDIO_PLAYER_ERROR("aicos_malloc error\n");
        goto _exit;
    }

    memset(player,0x00,sizeof(struct mini_audio_player));
    player->state = MINI_AUDIO_PLAYER_STATE_INIT;

    player->mq = aicos_queue_create(sizeof(struct mini_audio_player_msg), 10);

    if (player->mq == NULL) {
        MINI_AUDIO_PLAYER_ERROR("aicos_queue_create error\n");
        goto _exit;
    }

    player->lock = aicos_mutex_create();
    if (player->lock == NULL) {
        MINI_AUDIO_PLAYER_ERROR("aicos_mutex_create error\n");
        goto _exit;
    }

    player->sem_thread_exit = aicos_sem_create(0);
    if (player->sem_thread_exit == NULL) {
        MINI_AUDIO_PLAYER_ERROR("aicos_sem_create error\n");
        goto _exit;
    }

    player->sem_ack = aicos_sem_create(0);
    if (player->sem_ack == NULL) {
        MINI_AUDIO_PLAYER_ERROR("aicos_sem_create error\n");
        goto _exit;
    }

    player->stop_ack = aicos_sem_create(0);
    if (player->stop_ack == NULL) {
        MINI_AUDIO_PLAYER_ERROR("stop_ack error\n");
        goto _exit;
    }

    player->tid = aicos_thread_create("mini_audio_player", 32 * 1024, 20,
                                      mini_audio_player_entry, player);
    if (player->tid == NULL) {
        MINI_AUDIO_PLAYER_ERROR("aicos_thread_create error\n");
        goto _exit;
    }
    return player;
_exit:
    if (player) {
        if (player->mq) {
            aicos_queue_delete(player->mq);
            player->mq = NULL;
        }

        if (player->lock) {
            rt_mutex_delete(player->lock);
            player->lock = NULL;
        }

        if (player->sem_thread_exit) {
            aicos_sem_delete(player->sem_thread_exit);
            player->sem_thread_exit = NULL;
        }

        if (player->sem_ack) {
            aicos_sem_delete(player->sem_ack);
            player->sem_ack = NULL;
        }

        if (player->stop_ack) {
            aicos_sem_delete(player->stop_ack);
            player->stop_ack = NULL;
        }

        aicos_free(MEM_DEFAULT, player);
    }
    return NULL;
}

int mini_audio_player_destroy(struct mini_audio_player *player)
{
    aicos_mutex_take(player->lock, AICOS_WAIT_FOREVER);
    if (player->state != MINI_AUDIO_PLAYER_STATE_STOPED && player->state != MINI_AUDIO_PLAYER_STATE_INIT) {
        MINI_AUDIO_PLAYER_DEBUG("player->state: %d ", player->state);
        mini_audio_player_stop(player);
    }
    aicos_mutex_give(player->lock);
    MINI_AUDIO_PLAYER_DEBUG(" ");
    if (player->tid) {
        mini_audio_player_msg_send(player, MINI_AUDIO_PLAYER_MSG_DESTROY, NULL);
        MINI_AUDIO_PLAYER_DEBUG(" ");
        aicos_sem_take(player->sem_thread_exit, AICOS_WAIT_FOREVER);
    }
    MINI_AUDIO_PLAYER_DEBUG(" ");

    if (player->mq) {
        aicos_queue_delete(player->mq);
        player->mq = NULL;
    }

    if (player->lock) {
        rt_mutex_delete(player->lock);
        player->lock = NULL;
    }

    if (player->sem_thread_exit) {
        aicos_sem_delete(player->sem_thread_exit);
        player->sem_thread_exit = NULL;
    }

    if (player->sem_ack) {
        aicos_sem_delete(player->sem_ack);
        player->sem_ack = NULL;
    }

    if (player->stop_ack) {
        aicos_sem_delete(player->stop_ack);
        player->stop_ack = NULL;
    }

    if (player->wav_buff) {
        aicos_free(MEM_DEFAULT, player->wav_buff);
        player->wav_buff = NULL;
        player->wav_buff_size = 0;
    }

    if (player->tid) {
        aicos_thread_delete(player->tid);
        player->tid = NULL;
    }

    aicos_free(MEM_DEFAULT, player);

    return 0;
}

