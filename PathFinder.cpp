#include "PathFinder.h"

PathFinder::PathFinder(Map * pMap){

	
	m_pRMap = pMap;
	for (int y = 0; y < MAX_X_SIZE; ++y){
		for (int x = 0; x < MAX_Y_SIZE; ++x){
			m_map.m_tile[x][y] = pMap->m_tile[x][y];
		}
	}

}

PathFinder::~PathFinder(){

}

int PathFinder::FindPath(float startX, float startY, float destinationX, float destinationY, std::vector<Node_2D *> * saveVector){

	// FindNearlist Tile
	int stx = startX / SIZE_TILE;
	int sty = startY / SIZE_TILE;
	int desx = destinationX / SIZE_TILE;
	int desy = destinationY / SIZE_TILE;

	// Check Range
	if (stx < 0 || stx > MAX_X_SIZE || sty < 0 || sty > MAX_Y_SIZE
		|| desx < 0 || desx > MAX_X_SIZE || desy < 0 || desy > MAX_Y_SIZE)
		return OUT;

	// Check Destination Able
	if (m_map.m_tile[desx][desy] == DABLE){
		return FAULT;
	}

	// Check Need PathFind
	if (abs(stx - desx) < 1.0f && abs(sty - desy) < 1.0f){
		return DES;
	}

	// Ready PathFind
	int hCost = 0;

	Node_2D * pCurr;
	Node_2D * pPred;

	pPred = new Node_2D();
	pPred->m_x = stx;
	pPred->m_y = sty;
	pPred->m_Hcost = hCost;
	pPred->m_Gcost = (float) (abs(pPred->m_x - desx) * abs(pPred->m_x - desx)) + (abs(pPred->m_y - desy) * abs(pPred->m_y - desy));
	pPred->m_Fcost = pPred->m_Hcost + pPred->m_Gcost;
	pPred->m_parent = NULL;
	pPred->m_direction = ROOT;
	pPred->m_used = true;

	m_path.push_back(pPred);

	pCurr = NULL;

	int bestx = stx;
	int besty = sty;

	while (pPred->m_x != desx || pPred->m_y != desy){

		//**  팔방을 확인해서 배열에 Cost값대로 넣는다. **//

		// 좌측 확인 //
		if (0 < pPred->m_x) {

			if (m_map.m_tile[pPred->m_x - 1][pPred->m_y] != DABLE){

				pCurr = new Node_2D();
				pCurr->m_x = pPred->m_x - 1;
				pCurr->m_y = pPred->m_y;
				pCurr->m_parent = pPred;
				pCurr->m_direction = LEFT;
				pCurr->m_used = false;
				pCurr->m_Hcost = pPred->m_Hcost + 1;
				pCurr->m_Gcost = (float)(abs(pCurr->m_x - desx) * abs(pCurr->m_x - desx)) + (abs(pCurr->m_y - desy) * abs(pCurr->m_y - desy));
				pCurr->m_Fcost = pCurr->m_Hcost + pCurr->m_Gcost;

				if (m_map.m_tile[pCurr->m_x][pCurr->m_y] == USED){
					Node_2D * pNode = FindNode(pCurr->m_x, pCurr->m_y);
					if (!pNode){
						InsertNode(pCurr);
					}
					else if (pCurr->m_Fcost < pNode->m_Fcost){
						pNode->m_Hcost = pCurr->m_Hcost;
						pNode->m_Gcost = pCurr->m_Gcost;
						pNode->m_Fcost = pCurr->m_Fcost;
						pNode->m_parent = pCurr->m_parent;
						pNode->m_direction = pCurr->m_direction;
						pNode->m_used = false;
						delete pCurr;
					}
				}
				else{
					InsertNode(pCurr);
					m_map.m_tile[pCurr->m_x][pCurr->m_y] = USED;
				}
			}
		}

		// 우측 확인 //
		if (MAX_X_SIZE > pPred->m_x) {

			if (m_map.m_tile[pPred->m_x + 1][pPred->m_y] != DABLE){

				pCurr = new Node_2D();
				pCurr->m_x = pPred->m_x + 1;
				pCurr->m_y = pPred->m_y;
				pCurr->m_parent = pPred;
				pCurr->m_direction = RIGHT;
				pCurr->m_used = false;
				pCurr->m_Hcost = pPred->m_Hcost + 1;
				pCurr->m_Gcost = (float)(abs(pCurr->m_x - desx) * abs(pCurr->m_x - desx)) + (abs(pCurr->m_y - desy) * abs(pCurr->m_y - desy));
				pCurr->m_Fcost = pCurr->m_Hcost + pCurr->m_Gcost;

				if (m_map.m_tile[pCurr->m_x][pCurr->m_y] == USED){
					Node_2D * pNode = FindNode(pCurr->m_x, pCurr->m_y);
					if (!pNode){
						InsertNode(pCurr);
					}
					else if (pCurr->m_Fcost < pNode->m_Fcost){
						pNode->m_Hcost = pCurr->m_Hcost;
						pNode->m_Gcost = pCurr->m_Gcost;
						pNode->m_Fcost = pCurr->m_Fcost;
						pNode->m_parent = pCurr->m_parent;
						pNode->m_direction = pCurr->m_direction;
						pNode->m_used = false;
						delete pCurr;
					}
				}
				else{
					InsertNode(pCurr);
					m_map.m_tile[pCurr->m_x][pCurr->m_y] = USED;
				}
			}
		}

		// 상단 확인
		if (0 < pPred->m_y) {

			if (m_map.m_tile[pPred->m_x][pPred->m_y - 1] != DABLE){

				pCurr = new Node_2D();
				pCurr->m_x = pPred->m_x;
				pCurr->m_y = pPred->m_y - 1;
				pCurr->m_parent = pPred;
				pCurr->m_direction = UP;
				pCurr->m_used = false;
				pCurr->m_Hcost = pPred->m_Hcost + 1;
				pCurr->m_Gcost = (float)(abs(pCurr->m_x - desx) * abs(pCurr->m_x - desx)) + (abs(pCurr->m_y - desy) * abs(pCurr->m_y - desy));
				pCurr->m_Fcost = pCurr->m_Hcost + pCurr->m_Gcost;

				if (m_map.m_tile[pCurr->m_x][pCurr->m_y] == USED){
					Node_2D * pNode = FindNode(pCurr->m_x, pCurr->m_y);
					if (!pNode){
						InsertNode(pCurr);
					}
					else if (pCurr->m_Fcost < pNode->m_Fcost){
						pNode->m_Hcost = pCurr->m_Hcost;
						pNode->m_Gcost = pCurr->m_Gcost;
						pNode->m_Fcost = pCurr->m_Fcost;
						pNode->m_parent = pCurr->m_parent;
						pNode->m_direction = pCurr->m_direction;
						pNode->m_used = false;
						delete pCurr;
					}
				}
				else{
					InsertNode(pCurr);
					m_map.m_tile[pCurr->m_x][pCurr->m_y] = USED;
				}
			}
		}

		// 하단 확인
		if (MAX_Y_SIZE > pPred->m_y) {

			if (m_map.m_tile[pPred->m_x][pPred->m_y + 1] != DABLE){

				pCurr = new Node_2D();
				pCurr->m_x = pPred->m_x;
				pCurr->m_y = pPred->m_y + 1;
				pCurr->m_parent = pPred;
				pCurr->m_direction = DOWN;
				pCurr->m_used = false;
				pCurr->m_Hcost = pPred->m_Hcost + 1;
				pCurr->m_Gcost = (float)(abs(pCurr->m_x - desx) * abs(pCurr->m_x - desx)) + (abs(pCurr->m_y - desy) * abs(pCurr->m_y - desy));
				pCurr->m_Fcost = pCurr->m_Hcost + pCurr->m_Gcost;

				if (m_map.m_tile[pCurr->m_x][pCurr->m_y] == USED){
					Node_2D * pNode = FindNode(pCurr->m_x, pCurr->m_y);
					if (!pNode){
						InsertNode(pCurr);
					}
					else if (pCurr->m_Fcost < pNode->m_Fcost){
						pNode->m_Hcost = pCurr->m_Hcost;
						pNode->m_Gcost = pCurr->m_Gcost;
						pNode->m_Fcost = pCurr->m_Fcost;
						pNode->m_parent = pCurr->m_parent;
						pNode->m_direction = pCurr->m_direction;
						pNode->m_used = false;
						delete pCurr;
					}
				}
				else{
					InsertNode(pCurr);
					m_map.m_tile[pCurr->m_x][pCurr->m_y] = USED;
				}
			}

			// 좌상 확인
			if (0 < pPred->m_x && 0 < pPred->m_y){
				if (m_map.m_tile[pPred->m_x - 1][pPred->m_y] != DABLE &&
					m_map.m_tile[pPred->m_x][pPred->m_y - 1] != DABLE &&
					m_map.m_tile[pPred->m_x - 1][pPred->m_y - 1] != DABLE){
					pCurr = new Node_2D();
					pCurr->m_x = pPred->m_x - 1;
					pCurr->m_y = pPred->m_y - 1;
					pCurr->m_parent = pPred;
					pCurr->m_direction = LUP;
					pCurr->m_used = false;
					pCurr->m_Hcost = pPred->m_Hcost + 1.5;
					pCurr->m_Gcost = (float)(abs(pCurr->m_x - desx) * abs(pCurr->m_x - desx)) + (abs(pCurr->m_y - desy) * abs(pCurr->m_y - desy));
					pCurr->m_Fcost = pCurr->m_Hcost + pCurr->m_Gcost;

					if (m_map.m_tile[pCurr->m_x][pCurr->m_y] == USED){
						Node_2D * pNode = FindNode(pCurr->m_x, pCurr->m_y);
						if (!pNode){
							InsertNode(pCurr);
						}
						else if (pCurr->m_Fcost < pNode->m_Fcost){
							pNode->m_Hcost = pCurr->m_Hcost;
							pNode->m_Gcost = pCurr->m_Gcost;
							pNode->m_Fcost = pCurr->m_Fcost;
							pNode->m_parent = pCurr->m_parent;
							pNode->m_direction = pCurr->m_direction;
							pNode->m_used = false;
							delete pCurr;
						}
					}
					else{
						InsertNode(pCurr);
						m_map.m_tile[pCurr->m_x][pCurr->m_y] = USED;
					}
				}

				// 우상 확인
				if (513 > pPred->m_x && 0 < pPred->m_y){
					if (m_map.m_tile[pPred->m_x + 1][pPred->m_y] != DABLE &&
						m_map.m_tile[pPred->m_x][pPred->m_y - 1] != DABLE &&
						m_map.m_tile[pPred->m_x + 1][pPred->m_y - 1] != DABLE){
						pCurr = new Node_2D();
						pCurr->m_x = pPred->m_x + 1;
						pCurr->m_y = pPred->m_y - 1;
						pCurr->m_parent = pPred;
						pCurr->m_direction = RUP;
						pCurr->m_used = false;
						pCurr->m_Hcost = pPred->m_Hcost + 1.5;
						pCurr->m_Gcost = (float)(abs(pCurr->m_x - desx) * abs(pCurr->m_x - desx)) + (abs(pCurr->m_y - desy) * abs(pCurr->m_y - desy));
						pCurr->m_Fcost = pCurr->m_Hcost + pCurr->m_Gcost;

						if (m_map.m_tile[pCurr->m_x][pCurr->m_y] == USED){
							Node_2D * pNode = FindNode(pCurr->m_x, pCurr->m_y);
							if (!pNode){
								InsertNode(pCurr);
							}
							else if (pCurr->m_Fcost < pNode->m_Fcost){
								pNode->m_Hcost = pCurr->m_Hcost;
								pNode->m_Gcost = pCurr->m_Gcost;
								pNode->m_Fcost = pCurr->m_Fcost;
								pNode->m_parent = pCurr->m_parent;
								pNode->m_direction = pCurr->m_direction;
								pNode->m_used = false;
								delete pCurr;
							}
						}
						else{
							InsertNode(pCurr);
							m_map.m_tile[pCurr->m_x][pCurr->m_y] = USED;
						}
					}
				}

				// 좌하 확인
				if (0 < pPred->m_x && 513 > pPred->m_y){
					if (m_map.m_tile[pPred->m_x - 1][pPred->m_y] != DABLE &&
						m_map.m_tile[pPred->m_x][pPred->m_y + 1] != DABLE &&
						m_map.m_tile[pPred->m_x - 1][pPred->m_y + 1] != DABLE){
						pCurr = new Node_2D();
						pCurr->m_x = pPred->m_x - 1;
						pCurr->m_y = pPred->m_y + 1;
						pCurr->m_parent = pPred;
						pCurr->m_direction = LDW;
						pCurr->m_used = false;
						pCurr->m_Hcost = pPred->m_Hcost + 1.5;
						pCurr->m_Gcost = (float)(abs(pCurr->m_x - desx) * abs(pCurr->m_x - desx)) + (abs(pCurr->m_y - desy) * abs(pCurr->m_y - desy));
						pCurr->m_Fcost = pCurr->m_Hcost + pCurr->m_Gcost;

						if (m_map.m_tile[pCurr->m_x][pCurr->m_y] == USED){
							Node_2D * pNode = FindNode(pCurr->m_x, pCurr->m_y);
							if (!pNode){
								InsertNode(pCurr);
							}
							else if (pCurr->m_Fcost < pNode->m_Fcost){
								pNode->m_Hcost = pCurr->m_Hcost;
								pNode->m_Gcost = pCurr->m_Gcost;
								pNode->m_Fcost = pCurr->m_Fcost;
								pNode->m_parent = pCurr->m_parent;
								pNode->m_direction = pCurr->m_direction;
								pNode->m_used = false;
								delete pCurr;
							}
						}
						else{
							InsertNode(pCurr);
							m_map.m_tile[pCurr->m_x][pCurr->m_y] = USED;
						}
					}
				}

				// 우하 확인
				if (513 > pPred->m_x && 513 > pPred->m_y){
					if (m_map.m_tile[pPred->m_x + 1][pPred->m_y] != DABLE &&
						m_map.m_tile[pPred->m_x][pPred->m_y + 1] != DABLE &&
						m_map.m_tile[pPred->m_x + 1][pPred->m_y + 1] != DABLE){
						pCurr = new Node_2D();
						pCurr->m_x = pPred->m_x + 1;
						pCurr->m_y = pPred->m_y + 1;
						pCurr->m_parent = pPred;
						pCurr->m_direction = RDW;
						pCurr->m_used = false;
						pCurr->m_Hcost = pPred->m_Hcost + 1.5;
						pCurr->m_Gcost = (float)(abs(pCurr->m_x - desx) * abs(pCurr->m_x - desx)) + (abs(pCurr->m_y - desy) * abs(pCurr->m_y - desy));
						pCurr->m_Fcost = pCurr->m_Hcost + pCurr->m_Gcost;

						if (m_map.m_tile[pCurr->m_x][pCurr->m_y] == USED){
							Node_2D * pNode = FindNode(pCurr->m_x, pCurr->m_y);
							if (!pNode){
								InsertNode(pCurr);
							}
							else if (pCurr->m_Fcost < pNode->m_Fcost){
								pNode->m_Hcost = pCurr->m_Hcost;
								pNode->m_Gcost = pCurr->m_Gcost;
								pNode->m_Fcost = pCurr->m_Fcost;
								pNode->m_parent = pCurr->m_parent;
								pNode->m_direction = pCurr->m_direction;
								pNode->m_used = false;
								delete pCurr;
							}
						}
						else{
							InsertNode(pCurr);
							m_map.m_tile[pCurr->m_x][pCurr->m_y] = USED;
						}
					}
				}
			}
		}

		pPred->m_used = true;

		// 사방을 확인하였으니 아제 새로운 Pred를 선출한다.
		// 선출 방법은 배열에서 사용되지 않은 가장 F_Cost값이 작은 노드를 선발한다.
		pPred = FindPred();
		if (!pPred){
			return FAIL;
		}
	}

	// 검색을 성공적으로 마쳤으면 해당 노드 인덱스를 리턴하고 노드들을 원상복귀한다.
	// return Value;
	//std::cout << " PathSize : " << m_path.size();
	for (int i = 0; i < saveVector->size(); ++i){
		delete *saveVector->begin();
		saveVector->erase(saveVector->begin());
	}

	Node_2D * pNode = new Node_2D();
	pNode->m_x = pPred->m_x * 10;
	pNode->m_y = pPred->m_y * 10;
	pNode->m_direction = pPred->m_direction;
	saveVector->push_back(pNode);
	pPred = pPred->m_parent;
	while (pPred->m_parent != ROOT){
		pNode = new Node_2D();
		pNode->m_x = pPred->m_x *10;
		pNode->m_y = pPred->m_y *10;
		pNode->m_direction = pPred->m_direction;
		saveVector->push_back(pNode);
		pPred = pPred->m_parent;
	}
	//std::cout << " RSize : " << saveVector->size() << std::endl;

	ClearNode();

	return SUCCESS;
}

Node_2D * PathFinder::FindNode(int x, int y){
	for (auto i = m_path.begin(); i != m_path.end(); ++i){
		if ((*i)->m_x == x && (*i)->m_y == y)
			return (*i);
	}
	return NULL;
}

Node_2D * PathFinder::FindPred(void){
	for (auto i = m_path.begin(); i != m_path.end(); ++i){
		if (!(*i)->m_used)
			return (*i);
	}
	return NULL;
}

void PathFinder::InsertNode(Node_2D * newNode){

	for (auto i = m_path.begin(); i != m_path.end(); ++i){
		if ((*i)->m_Fcost > newNode->m_Fcost){
			m_path.insert(i, newNode);
			return;
		}
	}
	m_path.push_back(newNode);
}

void PathFinder::ClearNode(void){
	int size = m_path.size();
	int x;
	int y;
	for (int i = 0; i < size; ++i){
		x = (*m_path.begin())->m_x;
		y = (*m_path.begin())->m_y;
		m_map.m_tile[x][y] = m_pRMap->m_tile[x][y];
		delete * m_path.begin();
		m_path.erase(m_path.begin());
	}
}

bool KeyDown(int nVirtKey)
{
	if ((GetKeyState(nVirtKey) & 0x8000) != 0)
		return true;
	else
		return false;
}