#pragma once

#include "Map.h"
#include "vector"
#include <Windows.h>

#define OUT		1
#define DES		2
#define FAULT	3
#define SUCCESS 4
#define FAIL	5

#define ROOT	0
#define LEFT	1
#define RIGHT	2
#define UP		3
#define DOWN	4
#define LUP		5
#define RUP		6
#define LDW		7
#define RDW		8

class Node_2D{

public:

	int			m_x;
	int			m_y;
	float		m_Hcost;
	float		m_Gcost;
	float		m_Fcost;
	Node_2D *	m_parent;
	BYTE		m_direction;
	bool		m_used;
};

#define SIZE_TILE	1

class PathFinder{

public:
	PathFinder(Map * pMap);
	~PathFinder();

	Map * m_pRMap;	//유닛 충돌의 경우만
	Map m_map;

	std::vector<Node_2D *>	m_path;	//가는길을 저

	int FindPath(float startX, float startY, float destinationX, float destinationY, std::vector<Node_2D *> * saveVector);
	
	Node_2D * FindNode(int x, int y);	//지금 노드가 만들어 놓은 노드경로중에 지금 노드가 있나 없나를 체크.
	Node_2D * FindPred(void);			//지금 있는 노드에서 주변을 검색 후 갈 노드를 찾는것.
	void InsertNode(Node_2D * newNode);	//노드를 찾은후 집어넣는것.
	void ClearNode(void);
};

/*
//Declare constants
const int mapWidth = SZ_MAP_X, mapHeight = SZ_MAP_Y, tileSize = 1, numberPeople = 1;
const int notfinished = 0, notStarted = 0;// path-related constants
const int found = 1, nonexistent = 2;
const int walkable = 0, unwalkable = 1;// walkability array constants
class PathFinder
{
public:
	PathFinder();
	~PathFinder(void);

	Map m_map;

	bool LoadMapData(LPCWSTR filename);
	int FindPath(int pathfinderID, int startingX, int startingZ, int targetX, int targetZ);
	void ReadPath(int pathfinderID, int currentX, int currentY, int pixelsPerFrame);
	int ReadPathX(int pathfinderID, int pathLocation);
	int ReadPathY(int pathfinderID, int pathLocation);
	void SetStatus(int id, int status) { pathStatus[id] = status; }

	//Create needed arrays
	char walkability[mapWidth][mapHeight];
	int openList[mapWidth*mapHeight + 2]; //1 dimensional array holding ID# of open list items
	int whichList[mapWidth + 1][mapHeight + 1];  //2 dimensional array used to record 
	// 		whether a cell is on the open list or on the closed list.
	int openX[mapWidth*mapHeight + 2]; //1d array stores the x location of an item on the open list
	int openY[mapWidth*mapHeight + 2]; //1d array stores the y location of an item on the open list
	int parentX[mapWidth + 1][mapHeight + 1]; //2d array to store parent of each cell (x)
	int parentY[mapWidth + 1][mapHeight + 1]; //2d array to store parent of each cell (y)
	int Fcost[mapWidth*mapHeight + 2];	//1d array to store F cost of a cell on the open list
	int Gcost[mapWidth + 1][mapHeight + 1]; 	//2d array to store G cost for each cell.
	int Hcost[mapWidth*mapHeight + 2];	//1d array to store H cost of a cell on the open list
	int pathLength[numberPeople + 1];     //stores length of the found path for critter
	int pathLocation[numberPeople + 1];   //stores current position along the chosen path for critter		
	int* pathBank[numberPeople + 1];

	//Path reading variables
	int pathStatus[numberPeople + 1];
	int xPath[numberPeople + 1];
	int yPath[numberPeople + 1];

	int nOnClosedList;
};
*/

