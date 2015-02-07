#pragma once

#include <stdlib.h>
#include <stdint.h>

#include <lh_arr.h>

// special data types used in the MC packets

typedef uint8_t uuid_t[16];
typedef int32_t fixp;
typedef uint8_t angle_t;

// single metadata key-value pair
typedef struct {
    union {
        struct {
            unsigned char key  : 5;
            unsigned char type : 3;
        };
        uint8_t h;
    };
    union {
        int8_t  b;
        int16_t s;
        int32_t i;
        float   f;
        char    str[256];
        //TODO: slot;
        struct {
            int32_t x;
            int32_t y;
            int32_t z;
        };
        struct {
            float   pitch;
            float   yaw;
            float   roll;
        };
    };
} metadata;

#define META_BYTE    0
#define META_SHORT   1
#define META_INT     2
#define META_FLOAT   3
#define META_STRING  4
#define META_SLOT    5
#define META_COORD   6
#define META_ROT     7

////////////////////////////////////////////////////////////////////////////////
// Server -> Client

// 0x01
typedef struct {
    uint32_t eid;
    uint8_t  gamemode;
    int8_t   dimension;
    uint8_t  difficulty;
    uint8_t  maxplayers;
    char     leveltype[32];
    uint8_t  reduced_debug_info;
} SP_JoinGame_pkt;

// 0x02
typedef struct {
    char     json[32768];
    uint8_t  pos;
} SP_ChatMessage_pkt;

// 0x03
typedef struct {
    uint64_t worldage;
    uint64_t time;
} SP_TimeUpdate_pkt;

// 0x08
typedef struct {
    double x,y,z;
    float  yaw,pitch;
    char   flags;
} SP_PlayerPositionLook_pkt;

// 0x0c
typedef struct {
    uint32_t eid;
    uuid_t   uuid;
    fixp     x;
    fixp     y;
    fixp     z;
    angle_t  yaw;
    angle_t  pitch;
    int16_t  item;
    metadata *meta;
} SP_SpawnPlayer_pkt;

// 0x0f
typedef struct {
    uint32_t eid;
    uint8_t  mobtype;
    fixp     x;
    fixp     y;
    fixp     z;
    angle_t  yaw;
    angle_t  pitch;
    angle_t  headpitch;
    int16_t  vx;
    int16_t  vy;
    int16_t  vz;
    metadata *meta;
} SP_SpawnMob_pkt;

// 0x13
typedef struct {
    uint32_t count;
    uint32_t *eids;
} SP_DestroyEntities_pkt;

// 0x14
typedef struct {
    uint32_t eid;
} SP_Entity_pkt;

// 0x15
typedef struct {
    uint32_t eid;
    int8_t   dx;
    int8_t   dy;
    int8_t   dz;
    uint8_t  onground;
} SP_EntityRelMove_pkt;

// 0x16
typedef struct {
    uint32_t eid;
    angle_t  yaw;
    angle_t  pitch;
    uint8_t  onground;
} SP_EntityLook_pkt;

// 0x17
typedef struct {
    uint32_t eid;
    int8_t   dx;
    int8_t   dy;
    int8_t   dz;
    angle_t  yaw;
    angle_t  pitch;
    uint8_t  onground;
} SP_EntityLookRelMove_pkt;

// 0x18
typedef struct {
    uint32_t eid;
    int8_t   x;
    int8_t   y;
    int8_t   z;
    angle_t  yaw;
    angle_t  pitch;
    uint8_t  onground;
} SP_EntityTeleport_pkt;

// 0x46
typedef struct {
    int32_t     threshold;
} SP_SetCompression_pkt;

////////////////////////////////////////////////////////////////////////////////
// Client -> Server

// 0x01
typedef struct {
    char str[256];
} CP_ChatMessage_pkt;

// 0x0a
typedef struct {
} CP_Animation_pkt;

////////////////////////////////////////////////////////////////////////////////

#define PKT(name) name##_pkt _##name

typedef struct {
    union {
        int32_t pid;                 // for use as Cx_ Sx_ constants defined in mcp_ids.h
        struct {
            unsigned int type  : 24; // raw packet type from the protocol
            unsigned int mode  : 2;  // protocol mode: 0:Idle, 1:Status, 2:Login, 3:Play
            unsigned int _mode : 2;
            unsigned int cl    : 1;  // whether this packet comes from client
            unsigned int _cl   : 3;
        };
    };

    int32_t   ver;      // decoder will mark this with its version, so in
                        // difficult cases you can differentiate between
                        // changed interpretation
    int modified;

    uint8_t * raw;      // raw packet data
    ssize_t   rawlen;

    // various packet types depending on pid
    union {
        PKT(SP_JoinGame);
        PKT(SP_ChatMessage);
        PKT(SP_TimeUpdate);
        PKT(SP_PlayerPositionLook);
        PKT(SP_SpawnPlayer);
        PKT(SP_SpawnMob);
        PKT(SP_DestroyEntities);
        PKT(SP_Entity);
        PKT(SP_EntityRelMove);
        PKT(SP_EntityLook);
        PKT(SP_EntityLookRelMove);
        PKT(SP_EntityTeleport);
        PKT(SP_SetCompression);

        PKT(CP_ChatMessage);
        PKT(CP_Animation);
    };
} MCPacket;

typedef struct {
    lh_arr_declare(MCPacket *,queue);
} MCPacketQueue;

////////////////////////////////////////////////////////////////////////////////

MCPacket * decode_packet(int is_client, uint8_t *p, ssize_t len);
ssize_t    encode_packet(MCPacket *pkt, uint8_t *buf);
void       dump_packet(MCPacket *pkt);
void       free_packet  (MCPacket *pkt);
void       queue_packet (MCPacket *pkt, MCPacketQueue *q);

////////////////////////////////////////////////////////////////////////////////

