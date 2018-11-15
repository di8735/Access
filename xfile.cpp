//////////////////////////////////////////////////////////////////////////////////////////////////
// 
// File: xfile.cpp
// 
// Author: Frank Luna (C) All Rights Reserved
//
// System: AMD Athlon 1800+ XP, 512 DDR, Geforce 3, Windows XP, MSVC++ 7.0 
//
// Desc: Demonstrates how to load and render an XFile.
//          
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include "camera.h"
#include "CModel.h"
#include "Player.h"
#include "network.h"
#include "AlienSoldier.h"
#include "SkyBox.h"
#include "Effect.h"
#include "Billboard.h"
#include <fstream>

using namespace std;
#define SEND_PACKET_TIME		0.03
#define ROAD_HEIGHT				15
#define START_TEXTURE_WIDTH		800
#define START_TEXTURE_HEIGHT	480

//
// Globals
//

IDirect3DDevice9* Device = 0; 
LPDIRECTINPUT8 g_pDInput;
LPDIRECTINPUTDEVICE8 g_pDIKeyboard;
LPDIRECTINPUTDEVICE8 g_pDIMouse;
LPD3DXSPRITE pSprite;
LPDIRECT3DTEXTURE9 g_pTexture;
LPDIRECT3DTEXTURE9 g_pStartTexture;
LPDIRECT3DTEXTURE9 g_pHpBar;
LPDIRECT3DTEXTURE9 g_pHp;
LPDIRECT3DTEXTURE9 g_pRespawn;
LPDIRECT3DTEXTURE9 g_pRespawnBar;
LPDIRECT3DTEXTURE9 g_pRespawnFont;
LPD3DXFONT pFonts = NULL;
CAllocateHierarchy Alloc;
LPD3DXFRAME g_pFrameRoot;
LPD3DXANIMATIONCONTROLLER g_pAnimController;
CPlayer player;
CPlayer skelaton[MAX_USER];
Effect effect;
CModel startMap;
CModel stageMap1;
CModel stageMap2;
CModel stageMap3;
CModel stageMap4;
AlienSoldier alien[NUM_OF_MONSTER];
HWND main_window_handle;
HINSTANCE hInst;
Billboard blood;
Billboard nextStage;
Billboard lightEffect;
CModel stage1UI;
CModel stage2UI;
CModel stage3UI;
CModel stage4UI;

ifstream file;

const int Width  = 1068;
const int Height = 768;

float sendTime = 0.0f;

Camera TheCamera(Camera::LANDOBJECT);
SkyBox skyBox;

SOCKET	g_mysocket;
WSABUF	send_wsabuf;
char 	send_buffer[BUF_SIZE];
WSABUF	recv_wsabuf;
char	recv_buffer[BUF_SIZE];
char	packet_buffer[BUF_SIZE];
DWORD	in_packet_size = 0;
int		saved_packet_size = 0;
int		g_myid;

DIMOUSESTATE mouseState;
BYTE g_KeyboardState[256];

POINT windowPt;
bool start = false;

//
// Framework functions
//

bool IsTimePassed(float duringTime, float timeDelta)
{
	static float startTime = 0.0f;
	startTime += timeDelta;

	if (startTime > duringTime)
	{
		startTime = 0.0f;
		return true;
	}
	else
		return false;
}

bool connectServer(HWND main_window_handle)
{
	file.open("ip.txt");
	char ipBuf[64];

	if (file.is_open())
	{
		while (!file.eof())
			file >> ipBuf;
	}

	file.close();

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_mysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr(ipBuf);

	int Result = WSAConnect(g_mysocket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	Result = WSAAsyncSelect(g_mysocket, main_window_handle, WM_SOCKET, FD_CLOSE | FD_READ);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;

	return 0;
}

void ProcessPacket(char *ptr)
{
	static bool first_time = true;

	sc_packet_put_player *put_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
	sc_packet_pos *pos_packet = reinterpret_cast<sc_packet_pos *>(ptr);
	sc_packet_monster *mons_packet = reinterpret_cast<sc_packet_monster *>(ptr);
	sc_packet_dead_player *dead_player_packet = reinterpret_cast<sc_packet_dead_player *>(ptr);
	sc_packet_respawn_player *respawn_packet = reinterpret_cast<sc_packet_respawn_player*>(ptr);
	int id;

	switch (ptr[1])
	{
	case SC_PUT_PLAYER:
		id = put_packet->id;
		if (first_time)
		{
			first_time = false;
			g_myid = id;
		}
		if (id == g_myid)
		{
			player.setPos(D3DXVECTOR3(put_packet->x, put_packet->y, put_packet->z));
			player.setDrawPlayer(true);
		}
		else if (id < NPC_START)
		{
			skelaton[id].setPos(D3DXVECTOR3(put_packet->x, put_packet->y, put_packet->z));
			skelaton[id].loadModel(L"suit\\suit_A_v3.X", Device);
			skelaton[id].setDrawPlayer(true);
		}
		break;
	case SC_POS:
		id = pos_packet->id;
		if (id == g_myid)
		{
			/* D3DXVECTOR3 vec;
			 vec.x = my_packet->x;
			 vec.y = my_packet->y;
			 vec.z = my_packet->z;

			 vec -= player.getPos();
			 if (D3DXVec3Length(&vec) > 1.0f)
			 player.setPos(D3DXVECTOR3(my_packet->x, my_packet->y, my_packet->z));*/
			player.setPos(D3DXVECTOR3(pos_packet->x, pos_packet->y, pos_packet->z));

			player.setHp(pos_packet->hp);

			if (pos_packet->onLoad)
			{
				D3DXVECTOR3 pos(player.getPos());
				pos.y += ROAD_HEIGHT;
				player.setPos(pos);
			}
		}
		else if (id < NPC_START)
		{
			skelaton[id].setPos(D3DXVECTOR3(pos_packet->x, pos_packet->y, pos_packet->z));
			skelaton[id].setSkelatonDirection(D3DXVECTOR3(pos_packet->dx, pos_packet->dy, pos_packet->dz));
			skelaton[id].setState(pos_packet->animState);
			if (pos_packet->onLoad)
			{
				D3DXVECTOR3 pos(skelaton[id].getPos());
				pos.y += ROAD_HEIGHT;
				skelaton[id].setPos(pos);
			}
		}
		//	else {
		//		npc[other_id - NPC_START].x = my_packet->x;
		//		npc[other_id - NPC_START].y = my_packet->y;
		//	}
		break;
	case SC_MONSTER:
		alien[mons_packet->id].setPos(D3DXVECTOR3(mons_packet->x, mons_packet->y, mons_packet->z));
		alien[mons_packet->id].setTraceEnemyPos(D3DXVECTOR3(mons_packet->tx, mons_packet->ty, mons_packet->tz));
		alien[mons_packet->id].setState(mons_packet->state);
		alien[mons_packet->id].setAlive(mons_packet->alive);
		alien[mons_packet->id].setType(mons_packet->monsterType);
		break;
	case SC_DEAD_PLAYER:
		id = dead_player_packet->id;
		if (id == g_myid)
			player.setState(PLAYER_DEAD);
		else if (id < NPC_START)
			skelaton[id].setState(PLAYER_DEAD);
		break;
	case SC_RESPAWN_PLAYER:
		id = respawn_packet->id;
		if (id == g_myid)
		{
			player.setPos(D3DXVECTOR3(respawn_packet->x, respawn_packet->y, respawn_packet->z));
			player.setHp(respawn_packet->hp);
			player.setState(respawn_packet->animState);
			player.initPlayerModel();
		}
		else if (id < NPC_START)
		{
			skelaton[id].setPos(D3DXVECTOR3(respawn_packet->x, respawn_packet->y, respawn_packet->z));
			skelaton[id].setHp(respawn_packet->hp);
			skelaton[id].setState(respawn_packet->animState);
			skelaton[id].initPlayerModel();
		}
		break;
		//case SC_REMOVE_PLAYER:
		//	sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
		//	int other_id = my_packet->id;
		//	if (other_id == g_myid) {
		//		player.attr &= ~BOB_ATTR_VISIBLE;
		//	}
		//	else if (other_id < NPC_START) {
		//		skelaton[other_id].attr &= ~BOB_ATTR_VISIBLE;
		//	}
		//	else {
		//		npc[other_id - NPC_START].attr &= ~BOB_ATTR_VISIBLE;
		//	}
		//	break;
		//case SC_CHAT:
		//	sc_packet_chat *my_packet = reinterpret_cast<sc_packet_chat *>(ptr);
		//	int other_id = my_packet->id;
		//	if (other_id == g_myid) {
		//		wcsncpy(player.message, my_packet->message, 256);
		//		player.message_time = GetTickCount();
		//	}
		//	else if (other_id < NPC_START) {
		//		wcsncpy(skelaton[other_id].message, my_packet->message, 256);
		//		skelaton[other_id].message_time = GetTickCount();
		//	}
		//	else {
		//		wcsncpy(npc[other_id - NPC_START].message, my_packet->message, 256);
		//		npc[other_id - NPC_START].message_time = GetTickCount();
		//	}
		//	break;

		//default:
		//printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void ReadPacket(SOCKET sock)
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(sock, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) {
		int err_code = WSAGetLastError();
		printf("Recv Error [%d]\n", err_code);
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);

	while (0 != iobyte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (iobyte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}

d3d::Ray CalcPickingRay(int x, int y)
{
	float px = 0.0f;
	float py = 0.0f;

	D3DVIEWPORT9 vp;
	Device->GetViewport(&vp);

	D3DXMATRIX proj;
	Device->GetTransform(D3DTS_PROJECTION, &proj);

	px = (((2.0f*x) / vp.Width) - 1.0f) / proj(0, 0);
	py = (((-2.0f*y) / vp.Height) + 1.0f) / proj(1, 1);

	d3d::Ray ray;
	ray._origin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	ray._direction = D3DXVECTOR3(px, py, 1.0f);

	return ray;
}

void TransformRay(d3d::Ray* ray, D3DXMATRIX* T)
{
	// transform the ray's origin, w = 1.
	D3DXVec3TransformCoord(
		&ray->_origin,
		&ray->_origin,
		T);

	// transform the ray's direction, w = 0.
	D3DXVec3TransformNormal(
		&ray->_direction,
		&ray->_direction,
		T);

	// normalize the direction
	D3DXVec3Normalize(&ray->_direction, &ray->_direction);
}

bool RaySphereIntTest(d3d::Ray* ray, d3d::BoundingSphere* sphere)
{
	D3DXVECTOR3 v = ray->_origin - sphere->_center;

	float b = 2.0f * D3DXVec3Dot(&ray->_direction, &v);
	float c = D3DXVec3Dot(&v, &v) - (sphere->_radius * sphere->_radius);

	// find the discriminant
	float discriminant = (b * b) - (4.0f * c);

	// test for imaginary number
	if (discriminant < 0.0f)
		return false;

	discriminant = sqrtf(discriminant);

	float s0 = (-b + discriminant) / 2.0f;
	float s1 = (-b - discriminant) / 2.0f;

	// if a solution is >= 0, then we intersected the sphere
	if (s0 >= 0.0f || s1 >= 0.0f)
		return true;

	return false;
}

void clienterror(void)
{
	exit(-1);
}

bool Setup()
{
	HRESULT hr = 0;

	connectServer(main_window_handle);

	//
	// Load the XFile data.
	//

	blood.loadTexture(L"effect\\blood.png", Device);
	blood.createQuad(Device, 16, 4, 4);
	//blood.scaleBillboard(2.0f, 2.0f, 2.0f);
	blood.setAnimTime(0.2f);

	nextStage.loadTexture(L"effect\\nextStage.png", Device);
	nextStage.createQuad(Device, 16, 4, 4);
	nextStage.setAnimTime(1.0f);
	


	player.loadModel(L"suit\\suit_A_v3.X", Device);
	alien[0].loadModel(L"mob\\AlienSoldier_v8_b.X", Device);
	alien[1].loadModel(L"mob\\AlienSoldier_v8_w.X", Device);
	for (int i = 2; i < NUM_OF_MONSTER; ++i)
	{
		if (alien[i].getType() == 0)
			alien[i].setMesh(alien[0].getFrame(), alien[0].getAnimController());
		else if (alien[i].getType() == 1)
			alien[i].setMesh(alien[1].getFrame(), alien[1].getAnimController());
	}
	
	skyBox.loadModel(L"etc\\sky_v1.X", Device);
	startMap.LoadXFile(L"map\\stage1.X", Device);
	stageMap1.LoadXFile(L"map\\stage2.X", Device);
	stageMap2.LoadXFile(L"map\\stage3.X", Device);
	stageMap3.LoadXFile(L"map\\stage4.X", Device);
	stageMap4.LoadXFile(L"map\\stage5.X", Device);
	D3DXCreateTextureFromFile(Device, L"etc\\crosshairs1.tga", &g_pTexture);
	D3DXCreateTextureFromFile(Device, L"etc\\startTexture.png", &g_pStartTexture);
	D3DXCreateTextureFromFile(Device, L"ui\\hp_bar.tga", &g_pHpBar);
	D3DXCreateTextureFromFile(Device, L"ui\\hp.tga", &g_pHp);
	D3DXCreateTextureFromFile(Device, L"ui\\respawn.png", &g_pRespawn);
	D3DXCreateTextureFromFile(Device, L"ui\\respawnbar.png", &g_pRespawnBar);
	D3DXCreateTextureFromFile(Device, L"ui\\respawnFont.png", &g_pRespawnFont);
	effect.loadEffect(L"effect\\muzzle_3.X", Device);
	lightEffect.loadTexture(L"effect\\Tex_Roiling_Blast2_SubUV_1024.tga", Device);
	lightEffect.createQuad(Device, 9, 3, 3);
	lightEffect.setAnimTime(0.5f);
	lightEffect.setPos(D3DXVECTOR3(0.0f, 200.0f, 0.0f));

	stage1UI.LoadXFile(L"ui\\stage_A.X", Device);
	stage1UI.setScale(D3DXVECTOR3(1.5f, 1.5f, 1.5f));
	stage1UI.setModelPosition(D3DXVECTOR3(0.0f, 30.0f, 500.0f));
	D3DXMATRIX rotateMatrix;
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixRotationYawPitchRoll(&rotateMatrix, -PI / 2, PI / 2, 0.0f);
	stage1UI.setworldMatrix(rotateMatrix * stage1UI.getworldMatrix());

	stage2UI.LoadXFile(L"ui\\stage_A2.X", Device);
	stage2UI.setScale(D3DXVECTOR3(1.5f, 1.5f, 1.5f));
	stage2UI.setModelPosition(D3DXVECTOR3(500.0f, 30.0f, -500.0f));
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixRotationYawPitchRoll(&rotateMatrix, 0, PI / 2, 0.0f);
	stage2UI.setworldMatrix(rotateMatrix * stage2UI.getworldMatrix());

	stage3UI.LoadXFile(L"ui\\stage_A3.X", Device);
	stage3UI.setScale(D3DXVECTOR3(1.5f, 1.5f, 1.5f));
	stage3UI.setModelPosition(D3DXVECTOR3(0.0f, 30.0f, -500.0f));
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixRotationYawPitchRoll(&rotateMatrix, PI / 2, PI / 2, 0.0f);
	stage3UI.setworldMatrix(rotateMatrix * stage3UI.getworldMatrix());

	stage4UI.LoadXFile(L"ui\\stage_A4.X", Device);
	stage4UI.setScale(D3DXVECTOR3(1.5f, 1.5f, 1.5f));
	stage4UI.setModelPosition(D3DXVECTOR3(500.0f, 30.0f, -500.0f));
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixRotationYawPitchRoll(&rotateMatrix, PI, PI / 2, 0.0f);
	stage4UI.setworldMatrix(rotateMatrix * stage4UI.getworldMatrix());

	for (int i = 0; i < NUM_OF_MONSTER; ++i)
		alien[i].createSphere(Device);

	//
	// DirectInput Init
	//

	if(FAILED(DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDInput, NULL)))
		return E_FAIL;
	if (FAILED(g_pDInput->CreateDevice(GUID_SysMouse, &g_pDIMouse, NULL)))
		return E_FAIL;
	if (FAILED(g_pDInput->CreateDevice(GUID_SysKeyboard, &g_pDIKeyboard, NULL)))
		return E_FAIL;

	if (FAILED(g_pDIMouse->SetDataFormat(&c_dfDIMouse)))
		return E_FAIL;
	if (FAILED(g_pDIKeyboard->SetDataFormat(&c_dfDIKeyboard)))
		return E_FAIL;

	if (FAILED(g_pDIMouse->SetCooperativeLevel(main_window_handle, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
		return E_FAIL;
	if (FAILED(g_pDIKeyboard->SetCooperativeLevel(main_window_handle, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
		return E_FAIL;

	//
	// Set texture filters.
	//

	Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	// 
	// Set Lights.
	//

	//TheTerrain = new Terrain(Device, "Map\\map256.raw", 256, 256, 20, 0.5f);
	//TheTerrain->loadTexture("Map\\tile_.dds");

	D3DXVECTOR3 dir(1.0f, -1.0f, 1.0f);
	D3DXCOLOR col(0.8f, 0.8f, 0.8f, 0.8f);
	D3DLIGHT9 light = d3d::InitDirectionalLight(&dir, &col);

	D3DXVECTOR3 posLight(0.0f, 10000.0f, 1.0f);
	col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	light = d3d::InitPointLight(&posLight, &col);
	Device->SetLight(0, &light);
	Device->LightEnable(0, true);

	/*Device->SetLight(0, &light);
	Device->LightEnable(0, true);*/

	dir = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	col = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	light = d3d::InitDirectionalLight(&dir, &col);
	Device->SetLight(1, &light);
	Device->LightEnable(1, true);


	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	Device->SetRenderState(D3DRS_SPECULARENABLE, true);

	//
	// Set Fonts.
	//

	pFonts = d3d::InitFont(Device, pSprite);

	//
	// Set camera.
	//

	TheCamera.yaw(2 * 3.14f / 3);
	//TheCamera.yaw(-3.14f / 3);

	//
	// Set projection matrix.
	//

	D3DXMATRIX proj;

	D3DXMatrixPerspectiveFovLH(
			&proj,
			D3DX_PI * 0.5f, // 90 - degree
			(float)Width / (float)Height,
			1.0f,
			500000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	return true;
}

void Cleanup()
{
}

bool Display(float timeDelta)
{
	HRESULT hr;
	if(start)
	{
		sendTime += timeDelta;
		ShowCursor(false);
		RECT windowRect;
		GetWindowRect(main_window_handle, &windowRect);
		SetCursorPos(((windowRect.right - windowRect.left) / 2) + windowRect.left, ((windowRect.bottom - windowRect.top) / 2) + windowRect.top);

		DWORD iobyte;
		D3DXMATRIX world;
		D3DXVECTOR3 playerDirection;
		world = player.getWorldMatrix();
		playerDirection.x = world._31;
		playerDirection.y = world._32;
		playerDirection.z = world._33;
		D3DXVec3Normalize(&playerDirection, &playerDirection);

		if (FAILED(g_pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState)))
		{
			if (FAILED(g_pDIMouse->Acquire()))
				return E_FAIL;
		}
		if (FAILED(g_pDIKeyboard->GetDeviceState(sizeof(g_KeyboardState), &g_KeyboardState)))
		{
			if (FAILED(g_pDIKeyboard->Acquire()))
				return E_FAIL;
		}

		static int lastKeyDown = UP_KEY;

		if (g_KeyboardState[DIK_NUMPADENTER] & 0x80)
		{
		}

		if (g_KeyboardState[DIK_W] & 0x80 && player.getHp() > 0)
		{
			player.movePlayerPos(timeDelta);
			D3DXVECTOR3 lookVec;
			TheCamera.getLook(&lookVec);
			player.rotateYPlayer(-lookVec);
			player.setState(PLAYER_FRONT_RUN);

			if (sendTime > SEND_PACKET_TIME)
			{
				cs_packet_move *my_packet = reinterpret_cast<cs_packet_move *>(send_buffer);
				my_packet->size = sizeof(cs_packet_move);
				send_wsabuf.buf = send_buffer;
				send_wsabuf.len = sizeof(cs_packet_move);
				int ret = 0;

				my_packet->dx = playerDirection.x;
				my_packet->dy = playerDirection.y;
				my_packet->dz = playerDirection.z;
				my_packet->type = CS_MOVE;
				my_packet->time = timeDelta;
				my_packet->animState = player.getCurrAnimNum();
				memcpy(&send_buffer, my_packet, my_packet->size);
				ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

				if (ret)
				{
					int error_code = WSAGetLastError();
					::MessageBox(NULL, L"Error while sending packet", NULL, MB_OK);
				}
				sendTime = 0.0f;
			}
		}
		else if (g_KeyboardState[DIK_S] & 0x80 && player.getHp() > 0)
		{
			player.movePlayerPos(timeDelta);
			D3DXVECTOR3 lookVec;
			TheCamera.getLook(&lookVec);
			player.rotateYPlayer(lookVec);

			if (sendTime > SEND_PACKET_TIME)
			{
				cs_packet_move *my_packet = reinterpret_cast<cs_packet_move *>(send_buffer);
				my_packet->size = sizeof(cs_packet_move);
				send_wsabuf.buf = send_buffer;
				send_wsabuf.len = sizeof(cs_packet_move);
				int ret = 0;

				my_packet->dx = playerDirection.x;
				my_packet->dy = playerDirection.y;
				my_packet->dz = playerDirection.z;
				my_packet->type = CS_MOVE;
				my_packet->time = timeDelta;
				my_packet->animState = player.getCurrAnimNum();
				memcpy(&send_buffer, my_packet, my_packet->size);
				ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

				if (ret)
				{
					int error_code = WSAGetLastError();
					::MessageBox(NULL, L"Error while sending packet", NULL, MB_OK);
				}
				sendTime = 0.0f;
			}
		}
		else if (g_KeyboardState[DIK_A] & 0x80 && player.getHp() > 0)
		{
			player.movePlayerPos(timeDelta);
			D3DXVECTOR3 rightVec;
			TheCamera.getRight(&rightVec);
			player.rotateYPlayer(rightVec);
			player.setState(PLAYER_LEFT_RUN);

			if (sendTime > SEND_PACKET_TIME)
			{
				cs_packet_move *my_packet = reinterpret_cast<cs_packet_move *>(send_buffer);
				my_packet->size = sizeof(cs_packet_move);
				send_wsabuf.buf = send_buffer;
				send_wsabuf.len = sizeof(cs_packet_move);
				int ret = 0;

				my_packet->dx = playerDirection.x;
				my_packet->dy = playerDirection.y;
				my_packet->dz = playerDirection.z;
				my_packet->type = CS_MOVE;
				my_packet->time = timeDelta;
				my_packet->animState = player.getCurrAnimNum();
				memcpy(&send_buffer, my_packet, my_packet->size);
				ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

				if (ret)
				{
					int error_code = WSAGetLastError();
					::MessageBox(NULL, L"Error while sending packet", NULL, MB_OK);
				}
				sendTime = 0.0f;
			}
		}
		else if (g_KeyboardState[DIK_D] & 0x80 && player.getHp() > 0)
		{
			player.movePlayerPos(timeDelta);
			D3DXVECTOR3 rightVec;
			TheCamera.getRight(&rightVec);
			player.rotateYPlayer(-rightVec);
			player.setState(PLAYER_RIGHT_RUN);

			if (sendTime > SEND_PACKET_TIME)
			{
				cs_packet_move *my_packet = reinterpret_cast<cs_packet_move *>(send_buffer);
				my_packet->size = sizeof(cs_packet_move);
				send_wsabuf.buf = send_buffer;
				send_wsabuf.len = sizeof(cs_packet_move);
				int ret = 0;

				my_packet->dx = playerDirection.x;
				my_packet->dy = playerDirection.y;
				my_packet->dz = playerDirection.z;
				my_packet->type = CS_MOVE;
				my_packet->time = timeDelta;
				my_packet->animState = player.getCurrAnimNum();
				memcpy(&send_buffer, my_packet, my_packet->size);
				ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

				if (ret)
				{
					int error_code = WSAGetLastError();
					::MessageBox(NULL, L"Error while sending packet", NULL, MB_OK);
				}
				sendTime = 0.0f;
			}
		}
		else
		{
			if (sendTime > SEND_PACKET_TIME)
			{
				cs_packet_move *my_packet = reinterpret_cast<cs_packet_move *>(send_buffer);
				my_packet->size = sizeof(cs_packet_move);
				send_wsabuf.buf = send_buffer;
				send_wsabuf.len = sizeof(cs_packet_move);
				int ret = 0;

				my_packet->dx = playerDirection.x;
				my_packet->dy = playerDirection.y;
				my_packet->dz = playerDirection.z;
				my_packet->type = CS_STOP;
				my_packet->time = timeDelta;
				my_packet->animState = player.getCurrAnimNum();
				memcpy(&send_buffer, my_packet, my_packet->size);
				ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

				if (ret)
				{
					int error_code = WSAGetLastError();
					::MessageBox(NULL, L"Error while sending packet", NULL, MB_OK);
				}
				sendTime = 0.0f;
			}
			player.stopPlayer(lastKeyDown);
			player.setState(PLAYER_STOP);
		}

		D3DXVECTOR3 cameralook;
		TheCamera.getLook(&cameralook);

		static float shootDelayTime = 0.0f;

		if (mouseState.rgbButtons[0] & 0x80 && player.getHp() > 0)
		{
			player.fireGun(true, -cameralook);
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(main_window_handle, &pt);
			effect.setVisible(true);

			if (player.getState() == PLAYER_STOP)
			{
				D3DXVECTOR3 lookVec;
				TheCamera.getLook(&lookVec);
				player.rotateYPlayer(-lookVec);
			}

			// compute the ray in view space given the clicked screen point
			d3d::Ray ray = CalcPickingRay(pt.x + 10, pt.y + 20);

			// transform the ray to world space
			D3DXMATRIX view;
			Device->GetTransform(D3DTS_VIEW, &view);

			D3DXMATRIX viewInverse;
			D3DXMatrixInverse(&viewInverse, 0, &view);

			TransformRay(&ray, &viewInverse);

			for (int i = 0; i < NUM_OF_MONSTER; ++i)
			{
				if (RaySphereIntTest(&ray, &alien[i].getBodyBoundingSphere()) && alien[i].getAlive())
				{
					if (shootDelayTime > SHOOT_DELAY_TIME || shootDelayTime == 0.0f)
					{
						float bloodX, bloodY, bloodZ;
						bloodX = alien[i].getModel().getworldMatrix()._41;
						bloodY = alien[i].getModel().getworldMatrix()._42 + 100.0f;
						bloodZ = alien[i].getModel().getworldMatrix()._43;
						blood.setPos(D3DXVECTOR3(bloodX, bloodY, bloodZ));
						blood.setVisible(true);

						cs_packet_shot_monster *my_packet = reinterpret_cast<cs_packet_shot_monster *>(send_buffer);
						my_packet->size = sizeof(cs_packet_shot_monster);
						send_wsabuf.buf = send_buffer;
						send_wsabuf.len = sizeof(cs_packet_shot_monster);
						int ret = 0;

						my_packet->type = MON_DAMAGED_BODY;
						my_packet->id = i;
						memcpy(&send_buffer, my_packet, my_packet->size);
						ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

						if (ret)
						{
							int error_code = WSAGetLastError();
							::MessageBox(NULL, L"Error while sending packet", NULL, MB_OK);
						}
						shootDelayTime = 0.0f;
					}
				}
				else if (RaySphereIntTest(&ray, &alien[i].getHeadBoundingSphere()) && alien[i].getAlive())
				{
					if (shootDelayTime > SHOOT_DELAY_TIME || shootDelayTime == 0.0f)
					{
						float bloodX, bloodY, bloodZ;
						bloodX = alien[i].getModel().getworldMatrix()._41;
						bloodY = alien[i].getModel().getworldMatrix()._42 + 150.0f;
						bloodZ = alien[i].getModel().getworldMatrix()._43;
						blood.setPos(D3DXVECTOR3(bloodX, bloodY, bloodZ));
						blood.setVisible(true);

						cs_packet_shot_monster *my_packet = reinterpret_cast<cs_packet_shot_monster *>(send_buffer);
						my_packet->size = sizeof(cs_packet_shot_monster);
						send_wsabuf.buf = send_buffer;
						send_wsabuf.len = sizeof(cs_packet_shot_monster);
						int ret = 0;

						my_packet->type = MON_DAMAGED_HEAD;
						my_packet->id = i;
						memcpy(&send_buffer, my_packet, my_packet->size);
						ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

						if (ret)
						{
							int error_code = WSAGetLastError();
							::MessageBox(NULL, L"Error while sending packet", NULL, MB_OK);
						}
						shootDelayTime = 0.0f;
					}
				}
			}
		}
		else
		{
			player.setIsFire(false);
			effect.setVisible(false);
			player.setState(PLAYER_STOP);
		}

		shootDelayTime += timeDelta;

		for (int i = 0; i < NUM_OF_MONSTER; ++i)
		{
			if (ATTACK_STATE == alien[i].getState())
			{
				static float startTime = 0.0f;
				startTime += timeDelta;
				if (startTime > 0.35f)
				{
					player.setState(PLAYER_DAMAGED);
					if (startTime > 0.5f)
					{
						cs_packet_damaged_player *my_packet = reinterpret_cast<cs_packet_damaged_player *>(send_buffer);
						my_packet->size = sizeof(cs_packet_damaged_player);
						send_wsabuf.buf = send_buffer;
						send_wsabuf.len = sizeof(cs_packet_damaged_player);
						int ret = 0;

						my_packet->type = CS_PLAYER_DAMAGED;
						my_packet->animState = PLAYER_DAMAGED;
						memcpy(&send_buffer, my_packet, my_packet->size);
						ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

						if (ret)
						{
							int error_code = WSAGetLastError();
							::MessageBox(NULL, L"Error while sending packet", NULL, MB_OK);
						}
						startTime = 0.0f;
					}
				}
			}
		}

		static float mouseMove = 0.0f;
		mouseMove -= (float)mouseState.lX * timeDelta / 2;

		D3DXVECTOR3 pos;
		TheCamera.getPosition(&pos);
		pos.x = player.getPos().x + 150.0f * cos(mouseMove);
		pos.y = player.getPos().y + 200.0f;
		pos.z = player.getPos().z + 150.0f * sin(mouseMove);
		TheCamera.setPosition(&pos);
		TheCamera.yaw(mouseState.lX * timeDelta / 2);
		TheCamera.pitch(mouseState.lY * timeDelta / 2);
		// Update the view matrix representing the cameras 
		// new position/orientation.

		D3DXMATRIX V;
		TheCamera.getViewMatrix(&V);
		Device->SetTransform(D3DTS_VIEW, &V);

		//
		// Update: Rotate the mesh.
		//

		//
		// Render
		//

		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
		Device->BeginScene();

		Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

		//////////////////////////////////////////////////////////////////////////////////////
		//draw
		player.drawPlayer(Device, timeDelta);

		for (int i = 0; i < NUM_OF_MONSTER; ++i)
		{
			alien[i].moveAlienPos(timeDelta);
			alien[i].rotateAlien();
			alien[i].drawSphere(Device);
			alien[i].drawAlien(Device, timeDelta);
		}
		skyBox.drawSkyBox(Device, timeDelta);
		startMap.Draw(Device, timeDelta);
		stageMap1.Draw(Device, timeDelta);
		stageMap2.Draw(Device, timeDelta);
		stageMap3.Draw(Device, timeDelta);
		stageMap4.Draw(Device, timeDelta);
	
		////////////////////////////////////////////////////////////////////////////////////////

		for (auto i = 0; i < MAX_USER; ++i)
		{
			if (skelaton[i].getDrawPlayer())
			{
				skelaton[i].setSkelatonPosition(timeDelta);
				skelaton[i].skelatonRotate();
				skelaton[i].drawSkelaton(Device, timeDelta);
			}
		}

		D3DXVECTOR3 cameraPos;
		D3DXVECTOR3 playerPos;
		D3DXMATRIX playerWorld;
		D3DXMATRIX temp;

		TheCamera.getPosition(&cameraPos);
		cameraPos.y = 0.0f;

		D3DXMatrixIdentity(&playerWorld);
		D3DXMatrixRotationY(&temp, PI);
		playerWorld *= temp;

		playerPos.x = -25.0f;
		playerPos.y = 140.0f;
		playerPos.z = -100.0f;

		D3DXMatrixTranslation(&temp, playerPos.x, playerPos.y, playerPos.z);
		playerWorld *= temp;

		if (player.getState() == PLAYER_LEFT_RUN)
		{
			D3DXMatrixRotationY(&temp, PI / 2.15);
			playerWorld *= temp;
		}
		else if (player.getState() == PLAYER_RIGHT_RUN)
		{
			D3DXMatrixRotationY(&temp, -PI / 2.15);
			playerWorld *= temp;
		}

		temp = player.getWorldMatrix();
		playerWorld *= temp;

		D3DXMATRIX camera;
		D3DXVECTOR3 tempVec;
		D3DXMatrixIdentity(&camera);
		TheCamera.getRight(&tempVec);
		camera._11 = tempVec.x;
		camera._21 = tempVec.y;
		camera._31 = tempVec.z;
		TheCamera.getLook(&tempVec);
		camera._13 = tempVec.x;
		camera._23 = tempVec.y;
		camera._33 = tempVec.z;
		D3DXMatrixInverse(&camera, NULL, &camera);
		blood.setMatrix(&camera);

		//ÀÌÆåÆ® ºí·»µù
		Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		Device->SetRenderState(D3DRS_NORMALIZENORMALS, false);
		Device->SetRenderState(D3DRS_SPECULARENABLE, false);
		effect.setWorldMatrix(playerWorld);
		if(player.getState() != PLAYER_DEAD)
			effect.drawEffect(Device, timeDelta);
		blood.drawBillboardOnce(Device, timeDelta);
		//lightEffect.drawBillboard(Device, timeDelta);
		//nextStage.setPos(D3DXVECTOR3(0.0f, 200.0f, 0.0f));
		//nextStage.drawBillboard(Device, timeDelta);
		stage1UI.Draw(Device, timeDelta);
		stage2UI.Draw(Device, timeDelta);
		stage3UI.Draw(Device, timeDelta);
		stage4UI.Draw(Device, timeDelta);
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
		Device->SetRenderState(D3DRS_SPECULARENABLE, true);

		////////////////////////////////////////////////////////////////////////////////////
		//Print Frame Rate
		pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);

		//wchar_t buf[128];
		//RECT tempRect;
		//int frameRate = 0;
		//static int maxFrameRate = 0;
		//static int minFrameRate = 50000;
		//frameRate = d3d::CountFrame(timeDelta);
		//if (frameRate > maxFrameRate)
		//	maxFrameRate = frameRate;
		//if (frameRate < minFrameRate && frameRate > 0)
		//	minFrameRate = frameRate;

		//tempRect.left = 20;
		//tempRect.right = 120;
		//tempRect.top = 500;
		//tempRect.bottom = 520;
		//pFonts->DrawTextW(pSprite, L"Frame Rate :", -1, &tempRect, 0, D3DCOLOR_ARGB(255, 0, 0, 0));

		//tempRect.left = 130;
		//tempRect.right = 200;
		//tempRect.top = 500;
		//tempRect.bottom = 520;
		//_itow_s(frameRate, buf, 10);
		//pFonts->DrawTextW(pSprite, buf, -1, &tempRect, 0, D3DCOLOR_ARGB(255, 0, 0, 0));

		//tempRect.left = 20;
		//tempRect.right = 100;
		//tempRect.top = 530;
		//tempRect.bottom = 550;
		//pFonts->DrawTextW(pSprite, L"Max FPS :", -1, &tempRect, 0, D3DCOLOR_ARGB(255, 0, 0, 0));

		//tempRect.left = 115;
		//tempRect.right = 160;
		//tempRect.top = 530;
		//tempRect.bottom = 550;
		//_itow_s(maxFrameRate, buf, 10);
		//pFonts->DrawTextW(pSprite, buf, -1, &tempRect, 0, D3DCOLOR_ARGB(255, 0, 0, 0));

		//tempRect.left = 20;
		//tempRect.right = 100;
		//tempRect.top = 560;
		//tempRect.bottom = 580;
		//pFonts->DrawTextW(pSprite, L"Min FPS :", -1, &tempRect, 0, D3DCOLOR_ARGB(255, 0, 0, 0));

		//tempRect.left = 115;
		//tempRect.right = 160;
		//tempRect.top = 560;
		//tempRect.bottom = 580;
		//_itow_s(minFrameRate, buf, 10);
		//pFonts->DrawTextW(pSprite, buf, -1, &tempRect, 0, D3DCOLOR_ARGB(255, 0, 0, 0));

		//tempRect.left = 200;
		//tempRect.right = 250;
		//tempRect.top = 560;
		//tempRect.bottom = 580;
		////itoa(player.getPos().x, buf, 10);
		//_itow_s(player.getHp(), buf, 10);
		//pFonts->DrawTextW(pSprite, buf, -1, &tempRect, 0, D3DCOLOR_ARGB(255, 0, 0, 0));

		//tempRect.left = 260;
		//tempRect.right = 320;
		//tempRect.top = 560;
		//tempRect.bottom = 580;
		//_itow_s(player.getPos().z, buf, 10);
		//pFonts->DrawTextW(pSprite, buf, -1, &tempRect, 0, D3DCOLOR_ARGB(255, 0, 0, 0));
		//////////////////////////////////////////////////////////////////////////////////////

		D3DXMATRIX transMatrix;
		D3DXMATRIX scaleMatrix;
		D3DXMatrixIdentity(&transMatrix);
		D3DXMatrixTranslation(&transMatrix, -32, -32, 0.0f);

		D3DXMatrixIdentity(&scaleMatrix);

		POINT p;
		GetCursorPos(&p);
		RECT hpRect;
		hpRect.top = 0.0f;
		hpRect.bottom = 33.0f;
		hpRect.left = 0.0f;
		if (player.getHp() < 0)
			hpRect.right = 0.0f;
		else
			hpRect.right = 255.0f * ((float)player.getHp() / 100);

		static float elapseTime = 0.0f;

		RECT respawnRect;
		respawnRect.top = 0.0f;
		respawnRect.left = 0.0f;
		respawnRect.bottom = 32.0f;
		if (elapseTime < RESPAWN_TIME)
			respawnRect.right = 256.0f * (elapseTime / RESPAWN_TIME);
		else
			respawnRect.right = 256.0f;

		D3DXMatrixScaling(&scaleMatrix, 0.95f, 0.60f, 1.0f);
		pSprite->SetTransform(&scaleMatrix);
		pSprite->Draw(g_pHpBar, &hpRect, NULL, &D3DXVECTOR3(Width / 35, Height / 28, 0.0f), D3DCOLOR_RGBA(255, 255, 255, 255));
		D3DXMatrixIdentity(&scaleMatrix);
		pSprite->SetTransform(&scaleMatrix);

		D3DXMatrixScaling(&scaleMatrix, 1.0f, 0.5f, 1.0f);
		pSprite->SetTransform(&scaleMatrix);
		pSprite->Draw(g_pHp, NULL, NULL, &D3DXVECTOR3(Width / 40, Height / 40, 0.0f), D3DCOLOR_RGBA(255, 255, 255, 255));
		D3DXMatrixIdentity(&scaleMatrix);
		pSprite->SetTransform(&scaleMatrix);

		pSprite->SetTransform(&transMatrix);
		pSprite->Draw(g_pTexture, NULL, NULL, &D3DXVECTOR3(Width / 2, Height / 2, 0.0f), D3DCOLOR_RGBA(255, 255, 255, 255));
		D3DXMatrixIdentity(&transMatrix);
		pSprite->SetTransform(&transMatrix);

		switch(player.startRespawn(timeDelta))
		{
		case RESPAWNING:
			elapseTime += timeDelta;
			D3DXMatrixScaling(&scaleMatrix, 1.5f, 0.5f, 1.0f);
			pSprite->SetTransform(&scaleMatrix);
			pSprite->Draw(g_pRespawn, NULL, NULL, &D3DXVECTOR3(Width / 4.2, Height / 0.7, 0.01f), D3DCOLOR_RGBA(255, 255, 255, 255));
			D3DXMatrixIdentity(&scaleMatrix);
			pSprite->SetTransform(&scaleMatrix);

			D3DXMatrixScaling(&scaleMatrix, 1.48f, 0.82f, 1.0f);
			pSprite->SetTransform(&scaleMatrix);
			pSprite->Draw(g_pRespawnBar, &respawnRect, NULL, &D3DXVECTOR3(Width / 4.12, Height / 1.142, 0.0f), D3DCOLOR_RGBA(255, 255, 255, 255));
			D3DXMatrixIdentity(&scaleMatrix);
			pSprite->SetTransform(&scaleMatrix);

			D3DXMatrixScaling(&scaleMatrix, 0.5f, 0.3f, 1.0f);
			pSprite->SetTransform(&scaleMatrix);
			pSprite->Draw(g_pRespawnFont, NULL, NULL, &D3DXVECTOR3(Width / 1.25, Height / 0.5, 0.0f), D3DCOLOR_RGBA(255, 255, 255, 255));
			D3DXMatrixIdentity(&scaleMatrix);
			pSprite->SetTransform(&scaleMatrix);
			break;
		case RESPAWN_NOW:
			elapseTime = 0.0f;
			cs_packet_respawn *my_packet = reinterpret_cast<cs_packet_respawn *>(send_buffer);
			my_packet->size = sizeof(cs_packet_respawn);
			my_packet->type = CS_PLAYER_RESPAWN;
			send_wsabuf.buf = send_buffer;
			send_wsabuf.len = sizeof(cs_packet_respawn);
			int ret = 0;

			memcpy(&send_buffer, my_packet, my_packet->size);
			ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

			if (ret)
			{
				int error_code = WSAGetLastError();
				::MessageBox(NULL, L"Error while sending packet", NULL, MB_OK);
			}
		}

		pSprite->End();

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
	}
	else
	{
		if (FAILED(g_pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState)))
		{
			if (FAILED(g_pDIMouse->Acquire()))
				return E_FAIL;
		}

		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
		Device->BeginScene();

		pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);

		RECT rect;

		GetWindowRect(main_window_handle, &rect);

		D3DXMATRIX mat;
		D3DXMatrixScaling(&mat, ((float)rect.right / (float)START_TEXTURE_WIDTH), ((float)rect.bottom / (float)START_TEXTURE_HEIGHT), 1.0f);

		pSprite->SetTransform(&mat);

		pSprite->Draw(g_pStartTexture, NULL, NULL, NULL, D3DCOLOR_RGBA(255, 255, 255, 255));

		pSprite->End();

		POINT p;
		GetCursorPos(&p);
		GetWindowRect(main_window_handle, &rect);

		if (p.x > rect.right / 20 && p.x < rect.right / 1.8 && 
			p.y > rect.bottom / 3.5 && p.y < rect.bottom / 2.5 && mouseState.rgbButtons[1] & 0x80)
		{
			start = true;
		}
		if (p.x > rect.right / 20 && p.x < rect.right / 2.8 &&
			p.y > rect.bottom / 1.63 && p.y < rect.bottom / 1.45 && mouseState.rgbButtons[0] & 0x80)
		{
			::DestroyWindow(main_window_handle);
		}

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
	}
	return true;
}

//
// WndProc
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	case WM_SOCKET:
		if (WSAGETSELECTERROR(lParam))
		{
			closesocket((SOCKET)wParam);
			clienterror();
			break;
		}
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_READ:
			ReadPacket((SOCKET)wParam);
			break;
		case FD_CLOSE:
			closesocket((SOCKET)wParam);
			clienterror();
			break;
		}
		break;
	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE )
			::DestroyWindow(hwnd);
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
	main_window_handle = d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device, &pSprite);

	hInst = hinstance;
		
	if(!Setup())
	{
		::MessageBox(0, L"Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop( Display );

	Cleanup();

	Device->Release();

	return 0;
}


