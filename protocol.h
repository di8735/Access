#include <Windows.h>

#define MAX_BUFF_SIZE   4000
#define MAX_PACKET_SIZE  255

#define VIEW_RADIUS   4

#define MAX_USER 2

#define MY_SERVER_PORT  4000

#define MAX_STR_SIZE  100

#define NUM_OF_MONSTER 40
#define NPC_START 500

#define STARTING_X		0
#define STARTING_Y		0
#define STARTING_Z		0

#define SC_POS				1
#define SC_PUT_PLAYER		2
#define SC_REMOVE_PLAYER	3
#define SC_MONSTER			4
#define SC_DEAD_PLAYER		5
#define SC_RESPAWN_PLAYER	6

#define MON_STAND  1
#define MON_TRACE  2
#define MON_ATTACK 3

#define CS_MOVE  1
#define CS_DOWN  2
#define CS_LEFT  3
#define CS_RIGHT 4
#define CS_STOP  5
#define MON_DAMAGED_HEAD		6
#define MON_DAMAGED_BODY		7
#define CS_PLAYER_DAMAGED	    8
#define CS_PLAYER_FIRED			9
#define CS_PLAYER_STOP_FIRE		10
#define CS_PLAYER_RESPAWN		11

#pragma pack (push, 1)

struct cs_packet_move {
	BYTE size;
	BYTE type;
	int animState;
	float time;
	float dx;
	float dy;
	float dz;
	float x;
	float y;
	float z;
};

struct cs_packet_player_fired{
	BYTE size;
	BYTE type;
	bool isFire;
	int animState;
};

struct cs_packet_shot_monster{
	BYTE size;
	BYTE type;
	int id;
};
//1´ë°¡¸® 2 ¸öÅë

struct cs_packet_damaged_player{
	BYTE size;
	BYTE type;
	int animState;
};

struct cs_packet_stop{
	BYTE size;
	BYTE type;
};

struct cs_packet_respawn{
	BYTE size;
	BYTE type;
};

struct sc_packet_pos {
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
	float z;
	float dx;
	float dy;
	float dz;
	int hp;
	int animState;
	bool onLoad;
};

struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
	float z;
};

struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	WORD id;
};

struct sc_packet_dead_player{
	BYTE size;
	BYTE type;
	WORD id;
};

struct sc_packet_respawn_player{
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
	float z;
	int hp;
	int animState;
};

struct sc_packet_monster{
	BYTE size;
	BYTE type;
	float x;
	float y;
	float z;
	float tx;
	float ty;
	float tz;
	int monsterType;
	int state;
	bool alive;
	int hp;
	int id;
};

#pragma pack (pop)