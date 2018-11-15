#pragma once
#include "d3dUtility.h"
#include "CModel.h"

#define IDLE_ANIMATION				15
#define FRONT_WALK_ANIMATION		14
#define FIRE_ANIMATION				13
#define RUN_ANIMATION				12
#define RUN_FRONT_FIRE_ANIMATION	11
#define RUN_LEFT_FIRE_ANIMATION		10
#define RUN_RIGHT_FIRE_ANIMATION	9
#define DAMAGED_ANIMATION			8
#define DYING_ANIMATION				7

#define SHOOT_DELAY_TIME			0.3

#define PLAYER_STOP					0
#define PLAYER_FRONT_RUN			1
#define PLAYER_LEFT_RUN				2
#define PLAYER_RIGHT_RUN			3
#define PLAYER_DAMAGED				4
#define PLAYER_DEAD					5

#define NOT_RESPAWN					0
#define RESPAWNING					1
#define RESPAWN_NOW					2

const float RESPAWN_TIME = 8.0f;
const float RESPAWN_WAITING_TIME = 3.0f;

class CPlayer
{
public:
	CPlayer();
	~CPlayer();

	void		setPos(D3DXVECTOR3 setValue) { pos = setValue; mModel.setModelPosition(pos); }
	D3DXVECTOR3 getPos() { return pos; }
	void		setSpeed(float setValue) { charSpeed = setValue; }
	float		getSpeed() { return charSpeed; }
	void		setDrawPlayer(bool setValue) { isDrawPlayer = setValue; }
	bool		getDrawPlayer() { return isDrawPlayer; }
	void		setSkelatonKeyDown(int setValue) { skelatonKeyDown = setValue; }
	int			getSkelatonKeyDown() { return skelatonKeyDown; }
	void		setSkelatonDirection(D3DXVECTOR3 setValue) { mSkelatonDirection = setValue; }
	D3DXVECTOR3	getSkelatonDirection() { return mSkelatonDirection; }
	void		setState(int setValue) { mState = setValue; }
	int			getState() { return mState; }
	void		setIsFire(bool setValue) { mIsFire = setValue; }
	bool		getIsFire() { return mIsFire; }
	int			getHp() { return mHp; }
	void		setHp(int setValue) { mHp = setValue; }
	int			getCurrAnimNum() { return mModel.getCurrAnimNum(); }
	
	void loadModel(wchar_t* strFileName, LPDIRECT3DDEVICE9 pd3dDevice);
	void stopPlayer(int lastKey);
	void drawPlayer(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta);
	void drawSkelaton(LPDIRECT3DDEVICE9 pd3dDevice, float timeDelta);
	void movePlayerPos(float timeDelta);
	float rotateYPlayer(D3DXVECTOR3 cameraVec);
	void setSkelatonPosition(float timeDelta);
	void skelatonRotate();
	void setMoveAnim(char animNum);
	void fireGun(bool fire, D3DXVECTOR3 cameraLookVec);
	int startRespawn(float timeDelta);
	void initPlayerModel();
	D3DXMATRIX getWorldMatrix() { return mModel.getworldMatrix(); }

private:
	CModel mModel;
	D3DXVECTOR3 pos;
	float charSpeed;
	bool isDrawPlayer;
	int skelatonKeyDown;
	D3DXVECTOR3 mSkelatonDirection;
	int mState;
	bool mIsFire;
	int mHp;
};

