#include <iostream>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <Windows.h>

#define BOUNDINGBOX_NUM 50
#define ALIENSOLDIER_ATTACK_RANGE 200

using namespace std;

#include "protocol.h"
#include "AlienSoldier.h"
#include "BoundingBox.h"

const float PLAYER_SPEED = 15.0f;
const int NUM_THREADS = 6;

BoundingBox box[BOUNDINGBOX_NUM];
BoundingBox loadBox[4];

HANDLE hIOCP;

struct OVERAPPED_EX {
	WSAOVERLAPPED overapped;
	bool is_send;
	WSABUF wsabuf;
 	char IOCPbuf[MAX_BUFF_SIZE];
	char PacketBuf[MAX_PACKET_SIZE];
	unsigned int prev_received;
	unsigned int curr_packet_size;
};

struct PLAYER {
	int x;  
	int y;
	int z;
	int hp;
	int stage;
	float dx;
	float dy;
	float dz;
	int animState;
	bool onLoad;
	SOCKET sock;
	bool in_use;
	OVERAPPED_EX my_overapped;
};

PLAYER players[MAX_USER];
AlienSoldier alien[NUM_OF_MONSTER];

Stand *stand;
TraceEnemy *trace;
AttackEnemy *attack;

bool collisionCheckByMap(D3DXVECTOR3& vec, int stage)
{
	for (int i = 0; i < BOUNDINGBOX_NUM; ++i)
	{
		if (box[i].isPointInside(vec))
			return true;
	}

	return false;
}

bool detectPlayer()

{	
	Vector3 alienPos[NUM_OF_MONSTER];
	Vector3 playerPos[MAX_USER];

	for (auto j = 0; j < NUM_OF_MONSTER; ++j)
	{
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (players[i].in_use)
			{
				alienPos[j].x = alien[j].getPos().x;
				alienPos[j].y = alien[j].getPos().y;
				alienPos[j].z = alien[j].getPos().z;

				if (!alien[j].getAlive())
				{
					alien[j].changeState_dead();
				}
				else if (pow(players[i].x - alienPos[j].x, 2) + pow(players[i].y - alienPos[j].y, 2)
					+ pow(players[i].z - alienPos[j].z, 2) < ALIENSOLDIER_ATTACK_RANGE * ALIENSOLDIER_ATTACK_RANGE)
				{
					if (players[i].hp <= 0)
						alien[j].changeState_idle();
					else
					{
						playerPos[i].x = players[i].x;
						playerPos[i].y = players[i].y;
						playerPos[i].z = players[i].z;
						alien[j].setTracePlayerPos(playerPos[i]);
						alien[j].changeState_attack();
					}
				}
				else if (pow(players[i].x - alienPos[j].x, 2) + pow(players[i].y - alienPos[j].y, 2)
					+ pow(players[i].z - alienPos[j].z, 2) < 1000 * 1000)
				{
					playerPos[i].x = players[i].x;
					playerPos[i].y = players[i].y;
					playerPos[i].z = players[i].z;
					alien[j].setTracePlayerPos(playerPos[i]);
					alien[j].changeState_trace();
					break;
				}
				else
					alien[j].changeState_idle();

			}
		}
	}
	return false;
}

void error_display(char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s :", msg);
	wprintf(L"에러%s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void PlayerInit(int id)
{
	ZeroMemory(&players[id], sizeof(players[id]));
	players[id].in_use = false;
	players[id].my_overapped.is_send = false;
	players[id].my_overapped.wsabuf.buf = players[id].my_overapped.IOCPbuf;
	players[id].my_overapped.wsabuf.len = sizeof(players[id].my_overapped.IOCPbuf);
	players[id].hp = 100;
	players[id].onLoad = false;
	players[id].animState = 8;
	players[id].stage = 1;
}

void MonsterInit()
{
	for (int i = 0; i < NUM_OF_MONSTER; ++i)
	{
		alien[i].setAlive(true);
		alien[i].changeState_idle();
		alien[i].setHp(50);
	}

	for (int i = 0; i < NUM_OF_MONSTER; ++i)
	{
		if (i < 10)
			alien[i].setStagePosition(11);
		else if (i >= 10 && i < 20)
			alien[i].setStagePosition(12);
		else if (i >= 20 && i < 30)
			alien[i].setStagePosition(13);
		else if (i >= 30 && i < 40)
			alien[i].setStagePosition(14);

		if (players[0].stage == 2 && players[0].in_use && !players[1].in_use)
		{
			if (i >= 0 && i < 20)
				alien[i].setStagePosition(21);
			else if (i >= 20 && i < 30)
				alien[i].setStagePosition(22);
			else if (i >= 30 && i < 40)
				alien[i].setStagePosition(23);
		}
		else if (players[0].stage == 3 && players[0].in_use && !players[1].in_use)
		{
			if (i >= 0 && i < 8)
				alien[i].setStagePosition(31);
			else if (i >= 8 && i < 16)
				alien[i].setStagePosition(32);
			else if (i >= 16 && i < 24)
				alien[i].setStagePosition(33);
			else if (i >= 24 && i < 32)
				alien[i].setStagePosition(34);
			else if (i >= 32 && i < 40)
				alien[i].setStagePosition(35);
		}
		else if (players[0].stage == 2 && players[1].stage == 2 && players[0].in_use && players[1].in_use)
		{
			if (i >= 0 && i < 20)
				alien[i].setStagePosition(21);
			else if (i >= 20 && i < 30)
				alien[i].setStagePosition(22);
			else if (i >= 30 && i < 40)
				alien[i].setStagePosition(23);
		}
		else if (players[0].stage == 3 && players[1].stage == 3 && players[0].in_use && players[1].in_use)
		{
			if (i >= 0 && i < 8)
				alien[i].setStagePosition(31);
			else if (i >= 8 && i < 16)
				alien[i].setStagePosition(32);
			else if (i >= 16 && i < 24)
				alien[i].setStagePosition(33);
			else if (i >= 24 && i < 32)
				alien[i].setStagePosition(34);
			else if (i >= 32 && i < 40)
				alien[i].setStagePosition(35);
		}
	}

	for (int i = 0; i < NUM_OF_MONSTER; ++i)
	{
		if (alien[i].getStagePosition() == 11)
			alien[i].setPos(rand() % 2100 - 1050, 0, rand() % 5000 + 1104);
		else if (alien[i].getStagePosition() == 12)
			alien[i].setPos(rand() % 7500 + 2000, 0, rand() % 2000 - 1600);
		else if (alien[i].getStagePosition() == 13)
			alien[i].setPos(rand() % 1900 - 1050, 0, rand() % 7500 - 10000);
		else if (alien[i].getStagePosition() == 14)
			alien[i].setPos(rand() % 5000 - 7000, 0, rand() % 1900 - 1400);
		else if (alien[i].getStagePosition() == 21)
			alien[i].setPos(rand() % 1200 + 5900, 0, rand() % 9300 + 15950);
		else if (alien[i].getStagePosition() == 22)
			alien[i].setPos(rand() % 8000 - 1730, 0, rand() % 1000 + 14360);
		else if (alien[i].getStagePosition() == 23)
			alien[i].setPos(rand() % 1100 - 3560, 0, rand() % 7200 + 15750);
		else if (alien[i].getStagePosition() == 31)
			alien[i].setPos(rand() % 8800 - 4468, 0, rand() % 1000 + 44610);
		else if (alien[i].getStagePosition() == 32)
			alien[i].setPos(rand() % 1000 + 5200, 0, rand() % 6400 + 39339);
		else if (alien[i].getStagePosition() == 33)
			alien[i].setPos(rand() % 6800 - 640, 0, rand() % 1100 + 37488);
		else if (alien[i].getStagePosition() == 34)
			alien[i].setPos(rand() % 1000 - 772, 0, rand() % 2200 + 34508);
		else if (alien[i].getStagePosition() == 35)
			alien[i].setPos(rand() % 3900 - 3629, 0, rand() % 1100 + 32841);
	}
}

void BoundingBoxInit()
{
	box[0]._min = D3DXVECTOR3(-7650.0f, -100.0f, 880.0f);
	box[0]._max = D3DXVECTOR3(-1500.0f, 100.0f, 7100.0f);

	box[1]._min = D3DXVECTOR3(1527.0f, -100.0f, -11320.0f);
	box[1]._max = D3DXVECTOR3(10757.0f, 100.0f, -2077.0f);

	box[2]._min = D3DXVECTOR3(-1527.0f, -100.0f, -13320.0f);
	box[2]._max = D3DXVECTOR3(1527.0f, 100.0f, -11320.0f);

	box[3]._min = D3DXVECTOR3(-7665.0f, -100.0f, -11320.0f);
	box[3]._max = D3DXVECTOR3(-1527.0f, 100.0f, -2090.0f);

	box[4]._min = D3DXVECTOR3(-10000.0f, -100.0f, -2090.0f);
	box[4]._max = D3DXVECTOR3(-7665.0f, 100.0f, -889.0f);

	box[5]._min = D3DXVECTOR3(-1527.0f, -100.0f, 7100.0f);
	box[5]._max = D3DXVECTOR3(1500.0f, 100.0f, 10000.0f);

	box[6]._min = D3DXVECTOR3(1500.0f, -100.0f, 961.0f);
	box[6]._max = D3DXVECTOR3(4594.0f, 100.0f, 7150.0f);

	box[7]._min = D3DXVECTOR3(4594.0f, -100.0f, 1184.0f);
	box[7]._max = D3DXVECTOR3(10730.0f, 100.0f, 3184.0f);

	box[8]._min = D3DXVECTOR3(10730.0f, -100.0f, -2077.0f);
	box[8]._max = D3DXVECTOR3(12000.0f, 100.0f, 1184.0f);

	box[9]._min = D3DXVECTOR3(5408.0f, -100.0f, 28921.0f);
	box[9]._max = D3DXVECTOR3(7846.0f, 100.0f, 31590.0f);

	box[10]._min = D3DXVECTOR3(7663.0f, -100.0f, 13000.0f);
	box[10]._max = D3DXVECTOR3(9849.0f, 100.0f, 28919.0f);

	box[11]._min = D3DXVECTOR3(-4500.0f, -100.0f, 11117.0f);
	box[11]._max = D3DXVECTOR3(8000.0f, 100.0f, 13852.0f);

	box[12]._min = D3DXVECTOR3(-6500.0f, -100.0f, 13000.0f);
	box[12]._max = D3DXVECTOR3(-4100.0f, 100.0f, 25000.0f);

	box[13]._min = D3DXVECTOR3(-2000.0f, -100.0f, 15900.0f);
	box[13]._max = D3DXVECTOR3(2000.0f, 100.0f, 25000.0f);

	box[14]._min = D3DXVECTOR3(1000.0f, -100.0f, 15900.0f);
	box[14]._max = D3DXVECTOR3(5500.0f, 100.0f, 28900.0f);

	box[15]._min = D3DXVECTOR3(-7500.0f, -100.0f, 38000.0f);
	box[15]._max = D3DXVECTOR3(-5100.0f, 100.0f, 47000.0f);

	box[16]._min = D3DXVECTOR3(-5500.0f, -100.0f, 46200.0f);
	box[16]._max = D3DXVECTOR3(8000.0f, 100.0f, 50000.0f);

	box[17]._min = D3DXVECTOR3(6700.0f, -100.0f, 35000.0f);
	box[17]._max = D3DXVECTOR3(10000.0f, 100.0f, 48000.0f);

	box[18]._min = D3DXVECTOR3(900.0f, -100.0f, 30000.0f);
	box[18]._max = D3DXVECTOR3(7500.0f, 100.0f, 36900.0f);

	box[19]._min = D3DXVECTOR3(-5000.0f, -100.0f, 29000.0f);
	box[19]._max = D3DXVECTOR3(2000.0f, 100.0f, 32300.0f);

	box[20]._min = D3DXVECTOR3(-6000.0f, -100.0f, 34400.0f);
	box[20]._max = D3DXVECTOR3(-1200.0f, 100.0f, 40000.0f);

	box[21]._min = D3DXVECTOR3(-2000.0f, -100.0f, 39100.0f);
	box[21]._max = D3DXVECTOR3(4600.0f, 100.0f, 43000.0f);

	box[22]._min = D3DXVECTOR3(-3000.0f, -100.0f, 40000.0f);
	box[22]._max = D3DXVECTOR3(4600.0f, 100.0f, 44100.0f);

	loadBox[0]._min = D3DXVECTOR3(-INFINITY, -100.0f, 520.0f);
	loadBox[0]._max = D3DXVECTOR3(-1100.0f, 100.0f, INFINITY);

	loadBox[1]._min = D3DXVECTOR3(1100.0f, -100.0f, 520.0f);
	loadBox[1]._max = D3DXVECTOR3(INFINITY, 100.0f, INFINITY);

	loadBox[2]._min = D3DXVECTOR3(1100.0f, -100.0f, -INFINITY);
	loadBox[2]._max = D3DXVECTOR3(INFINITY, 100.0f, -1660.0f);

	loadBox[3]._min = D3DXVECTOR3(-INFINITY, -100.0f, -INFINITY);
	loadBox[3]._max = D3DXVECTOR3(-1100.0f, 100.0f, -1660.0f);
}

void SendPacket(int id, void *packet)
{
	int packet_size = reinterpret_cast<unsigned char *>(packet)[0];

	OVERAPPED_EX *send_over = new OVERAPPED_EX;
	ZeroMemory(send_over, sizeof(OVERAPPED_EX));
	send_over->is_send = true;
	send_over->wsabuf.buf = send_over->IOCPbuf;
	send_over->wsabuf.len = packet_size;
	unsigned long io_size;

	memcpy(send_over->IOCPbuf, packet, packet_size);
	WSASend(players[id].sock, &send_over->wsabuf, 1,
		&io_size, NULL, &send_over->overapped, NULL);
}

void ProcessPacket(char packet[], int id)
{	
	sc_packet_pos pos_packet;
	sc_packet_monster mons_packet;
	
	detectPlayer();
	
	if (packet[1] == MON_DAMAGED_BODY || packet[1] == MON_DAMAGED_HEAD)
	{
		cs_packet_shot_monster *shot_monster = reinterpret_cast<cs_packet_shot_monster*>(packet);

		if (alien[shot_monster->id].getAlive())
		{
			int mons_id = shot_monster->id;

			int hp = alien[mons_id].getHp();

			if (shot_monster->type == 6)//대가리
			{
				hp = 0;
				alien[mons_id].setAlive(false);
			}
			else if (shot_monster->type == 7)//몸통
			{
				if (alien[mons_id].getHp() <= 0)
				{
					alien[mons_id].setAlive(false);
				}
				else
				{
					alien[mons_id].decreaseHp(10);
					alien[mons_id].changeState_damaged();
				}
			}
		}
	}

	if (CS_PLAYER_RESPAWN == packet[1])
	{
		players[id].animState = 0;
		players[id].x = 0;
		players[id].y = 0;
		players[id].z = 0;
		players[id].stage = 1;
		players[id].hp = 100;

		sc_packet_respawn_player respawn_packet;
		respawn_packet.size = sizeof(respawn_packet);
		respawn_packet.type = SC_RESPAWN_PLAYER;
		respawn_packet.x = players[id].x;
		respawn_packet.y = players[id].y;
		respawn_packet.z = players[id].z;
		respawn_packet.hp = players[id].hp;
		respawn_packet.animState = players[id].animState;

		for (int i = 0; i < MAX_USER; ++i)
			SendPacket(i, &pos_packet);
	}

	if (CS_PLAYER_DAMAGED == packet[1])
	{
		cs_packet_damaged_player *packet_damaged_player = reinterpret_cast<cs_packet_damaged_player*>(packet);
		players[id].animState = packet_damaged_player->animState;
		players[id].hp -= 10;
	}

	if (CS_MOVE == packet[1])
	{
		cs_packet_move *packet_move = reinterpret_cast<cs_packet_move*>(packet);

		players[id].dx = packet_move->dx;
		players[id].dy = packet_move->dy;
		players[id].dz = packet_move->dz;
		players[id].animState = packet_move->animState;

		if (players[id].x > -1000 && players[id].x < 1000 && players[id].z > 6500 && players[id].z < 7000)
		{
			players[id].stage = 2;
			players[id].x = 6500;
			players[id].z = 28500;
			MonsterInit();
		}

		if (players[id].x > -4500 && players[id].x < -2500 && players[id].z > 23500 && players[id].z < 24000)
		{
			players[id].stage = 1;
			players[id].x = 0;
			players[id].z = 0;
			MonsterInit();
		}

		if (players[id].x > 10000 && players[id].x < 10700 && players[id].z > -2000 && players[id].z < 1000)
		{
			players[id].stage = 3;
			players[id].x = -5000;
			players[id].z = 41000;
			MonsterInit();
		}

		if (players[id].x > -5000 && players[id].x < -4500 && players[id].z > 32000 && players[id].z < 34000)
		{
			players[id].stage = 1;
			players[id].x = 0;
			players[id].z = 0;
			MonsterInit();
		}

		float x = packet_move->dx * PLAYER_SPEED;
		float y = packet_move->dy;
		float z = packet_move->dz * PLAYER_SPEED;

		D3DXVECTOR3 vec;
		vec.x = players[id].x - x;
		vec.y = players[id].y;
		vec.z = players[id].z - z;

		if (!collisionCheckByMap(vec, players[id].stage))
		{
			players[id].x -= x;
			players[id].z -= z;
		}
		vec.x = players[id].x;
		vec.y = players[id].y;
		vec.z = players[id].z;

		if (loadBox[0].isPointInside(vec) || loadBox[1].isPointInside(vec) ||
			loadBox[2].isPointInside(vec) || loadBox[3].isPointInside(vec))
		{
			players[id].onLoad = true;
		}
		else
			players[id].onLoad = false;
	}

	if (CS_STOP == packet[1])
	{
		cs_packet_move *packet_move = reinterpret_cast<cs_packet_move*>(packet);

		players[id].dx = packet_move->dx;
		players[id].dy = packet_move->dy;
		players[id].dz = packet_move->dz;
		players[id].animState = packet_move->animState;
	}
	
	pos_packet.size = sizeof(pos_packet);
	pos_packet.type = SC_POS;
	pos_packet.id = id;
	pos_packet.x = players[id].x;					
	pos_packet.y = players[id].y;
	pos_packet.z = players[id].z;
	pos_packet.dx = players[id].dx;
	pos_packet.dy = players[id].dy;
	pos_packet.dz = players[id].dz;
	pos_packet.hp = players[id].hp;
	pos_packet.animState = players[id].animState;
	pos_packet.onLoad = players[id].onLoad;
	
	for (int i = 0; i < NUM_OF_MONSTER; ++i)
		alien[i].updateStateMachine();
	
	for (int i = 0; i < NUM_OF_MONSTER; ++i)
	{
		mons_packet.size = sizeof(mons_packet);
		mons_packet.type = SC_MONSTER;
		mons_packet.x = alien[i].getPos().x;
		mons_packet.y = alien[i].getPos().y;
		mons_packet.z = alien[i].getPos().z;
		mons_packet.state = alien[i].states();
		mons_packet.tx = alien[i].getTracePlayerPos().x;
		mons_packet.ty = alien[i].getTracePlayerPos().y;
		mons_packet.tz = alien[i].getTracePlayerPos().z;
		mons_packet.alive = alien[i].getAlive();
		mons_packet.hp = alien[i].getHp();
		mons_packet.id = i;
		mons_packet.monsterType = alien[i].getType();

		//접속한 모두가 볼수있도록.
		for (int j = 0; j < MAX_USER; ++j)
		{
			if (true == players[j].in_use)
				SendPacket(j, &mons_packet);
		}
	}
	for (int i = 0; i < MAX_USER; ++i)
		SendPacket(i, &pos_packet);
}

void worker_thread()
{
	while (true)
	{
		unsigned long io_size;
		unsigned long key;
		OVERAPPED_EX *over_ex;
		GetQueuedCompletionStatus(hIOCP, &io_size, &key,
			reinterpret_cast<LPOVERLAPPED *>(&over_ex), INFINITE);
		// ERROR
		// 접속종료
		if (io_size == 0) {
			closesocket(players[key].sock);
			sc_packet_remove_player rem_packet;
			rem_packet.id = key;
			rem_packet.size = sizeof(rem_packet);
			rem_packet.type = SC_REMOVE_PLAYER;
			for (int i = 0; i < MAX_USER; ++i){
				if (key == i) continue;
				if (false == players[i].in_use) continue;
				SendPacket(i, &rem_packet);
			}
			players[key].in_use = false;
			continue;
		}
		if (over_ex->is_send == false){
			// RECV
			int rest_size = io_size;
			char *buf = over_ex->IOCPbuf;
			int packet_size = over_ex->curr_packet_size;
			while (0 < rest_size) {
				if (0 == packet_size) packet_size = buf[0];
				int remain = packet_size - over_ex->prev_received;
				if (remain > rest_size) {
					// 패킷 만들기에는 부족하다.
					memcpy(over_ex->PacketBuf + over_ex->prev_received,
						buf, rest_size);
					over_ex->prev_received += rest_size;
					break;
				}
				else {
					// 패킷을 만들 수 있다.
					memcpy(over_ex->PacketBuf + over_ex->prev_received,
						buf, remain);
					ProcessPacket(over_ex->PacketBuf, key);
					rest_size -= remain;
					packet_size = 0;
					over_ex->prev_received = 0;
				}
			}
			over_ex->curr_packet_size = packet_size;
			unsigned long recv_flag = 0;
			WSARecv(players[key].sock,
				&over_ex->wsabuf, 1, NULL,
				&recv_flag, &over_ex->overapped, NULL);
		}
		else {
			delete over_ex;
			// SEND
		}
	}

}

int GetNewClient_ID()
{
	for (int i = 0; i < MAX_USER; ++i)
	if (players[i].in_use == false){
		PlayerInit(i);
		return i;
	}
	cout << "USER FULL ERROR!!";
	exit(-1);
	return -1;
}

void accept_thread()
{
	struct sockaddr_in listen_addr;
	struct sockaddr_in client_addr;

	SOCKET listen_socket = WSASocket(AF_INET, SOCK_STREAM,
		IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	ZeroMemory(&listen_addr, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(MY_SERVER_PORT);
	ZeroMemory(&listen_addr.sin_zero, 8);
	int ret = ::bind(listen_socket,
		reinterpret_cast<sockaddr *>(&listen_addr),
		sizeof(listen_addr));
	if (SOCKET_ERROR == ret) {
		error_display("BIND", WSAGetLastError());
		exit(-1);
	}
	ret = listen(listen_socket, 10);
	if (SOCKET_ERROR == ret) {
		error_display("LISTEN", WSAGetLastError());
		exit(-1);
	}
	while (true) {
		int addr_size = sizeof(client_addr);
		SOCKET client_socket = WSAAccept(listen_socket,
			reinterpret_cast<sockaddr *>(&client_addr),
			&addr_size, NULL, NULL);
		cout << " 클라이언트 접속. 포트번호: "<< client_addr.sin_port << endl;
		if (INVALID_SOCKET == client_socket) {
			error_display("ACCEPT", WSAGetLastError());
			exit(-1);
		}
		int id = GetNewClient_ID();
		players[id].sock = client_socket;
		players[id].x = STARTING_X;
		players[id].y = STARTING_Y;
		players[id].z = STARTING_Z;
		CreateIoCompletionPort(
			reinterpret_cast<HANDLE>(client_socket),
			hIOCP, id, 0);
		unsigned long recv_flag = 0;
		ret = WSARecv(client_socket,
			&players[id].my_overapped.wsabuf, 1, NULL, &recv_flag,
			&players[id].my_overapped.overapped, NULL);
		if (SOCKET_ERROR == ret) {
			int err_code = WSAGetLastError();
			if (WSA_IO_PENDING != err_code) {
				error_display("ACCEPT(WSARecv):", err_code);
				exit(-1);
			}
		}

		sc_packet_put_player put_player_packet;
		put_player_packet.id = id;
		put_player_packet.type = SC_PUT_PLAYER;
		put_player_packet.size = sizeof(put_player_packet);
		put_player_packet.x = players[id].x;
		put_player_packet.y = players[id].y;
		put_player_packet.z = players[id].z;
		SendPacket(id, &put_player_packet);		

		for (int i = 0; i < MAX_USER; ++i){
			if (false == players[i].in_use) continue;
			if (id == i) continue;
			SendPacket(i, &put_player_packet);			
		}
		for (int i = 0; i < MAX_USER; ++i){
			if (false == players[i].in_use) continue;
			if (id == i) continue;
			put_player_packet.id = i;
			put_player_packet.x = players[i].x;
			put_player_packet.y = players[i].y;
			put_player_packet.z = players[i].z;
			SendPacket(id, &put_player_packet);			
		}
		players[id].in_use = true;

		sc_packet_monster mons_packet;

		for (int i = 0; i < NUM_OF_MONSTER; ++i)
		{
			mons_packet.size = sizeof(mons_packet);
			mons_packet.type = SC_MONSTER;
			mons_packet.x = alien[i].getPos().x;
			mons_packet.y = alien[i].getPos().y;
			mons_packet.z = alien[i].getPos().z;
			mons_packet.state = alien[i].states();
			mons_packet.tx = alien[i].getTracePlayerPos().x;
			mons_packet.ty = alien[i].getTracePlayerPos().y;
			mons_packet.tz = alien[i].getTracePlayerPos().z;
			mons_packet.alive = alien[i].getAlive();
			mons_packet.hp = alien[i].getHp();
			mons_packet.id = i;
			mons_packet.type = alien[i].getType();

			//접속한 모두가 볼수있도록.
			for (int j = 0; j < MAX_USER; ++j)
			{
				if (true == players[j].in_use)
					SendPacket(j, &mons_packet);
			}
		}
	}
}

void NetworkInit()
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
}

int main(){
	vector <thread *> worker_threads;

	_wsetlocale(LC_ALL, L"korean");
	NetworkInit();
	
	for (int i = 0; i < MAX_USER; ++i) 
		PlayerInit(i);
	MonsterInit();
	BoundingBoxInit();

	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	for (int i = 0; i < NUM_THREADS; ++i)
		worker_threads.push_back(new thread{ worker_thread });

	/*Initialize_NPC();*/
	thread accept_threads = thread{ accept_thread };

	while (true) {
		Sleep(1000);
		// if (g_server_shutdown == true) break;
	}
}