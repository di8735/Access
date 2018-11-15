#pragma once

#include <math.h>
#include <Windows.h>

#define MAX_X_SIZE 1000//예 맵이 5000이면 노드를 10으로 잡는다면 5000/10
#define MAX_Y_SIZE 1000

#define DABLE 1
#define ABLE 2
#define USED 3

class Map{
public:
	BYTE m_tile[MAX_X_SIZE][MAX_Y_SIZE];

	Map(){

		for (int i = 0; i < MAX_X_SIZE; ++i){
			for (int j = 0; j < MAX_Y_SIZE; ++j){
				m_tile[MAX_X_SIZE][MAX_Y_SIZE] = rand() % 2 + 1;
			}
		}
	
	};
};