#ifndef __COOKER_PROTOCOL_H__
#define __COOKER_PROTOCOL_H__

#include <stdint.h>

enum
{
    COOKER_CHAN_0,
    COOKER_CHAN_1,
    COOKER_CHAN_2,
    COOKER_CHAN_3,
    COOKER_CHAN_4,
};

enum
{
    STATE_CH_DEBUG,
    STATE_CH_PAN_TEMP,
    STATE_CH_IGBT_TEMP,
    STATE_CH_VOLTAGE,
    STATE_CH_CURRENT,
};

#pragma pack(push, 1)
typedef struct
{
    uint8_t serial  :3;
    uint8_t fault   :5;
} cooker_fault_t;

typedef struct
{
    uint8_t pot_state   :1;
    uint8_t pot_timeout :1;
    uint8_t spec_pot    :1;
    uint8_t afterheat   :1;
    uint8_t pot_chan    :4;
} cooker_state_t;

typedef struct
{
    uint8_t serial  :3;
    uint8_t onoff   :1;
    uint8_t gear    :4;
} cooker_resp_t;

#pragma pack(pop)








#endif
