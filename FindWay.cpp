class Pos{
	
	bool walked;
};
//0인지 1인지 구분해놓은 맵정보를 받아와서 맵을 하나 만들어야함.그리고 이 맵 정보를 아래 맵에 대입.
Pos Map[500][500];		//이 맵은 walked인지 판단하는 맵

class Node{ 
	int result_value; 
	int x;
	int y;
	int next_x;
	int next_y;
};
float range(Vector * Alien, Node * Player)
{
	//vector.x - NOde.x를 리턴
	return sqrt(Alien - Player);
}
class RoadFind{

	Pos * pMap = Map;

	vector<Node> path;

	Node * nowNode;

	iterator::vector<Node> * pNode;

	//targetNode는 플레이어 좌표
	Node * Find(Vector * AlienPosition, Node * targetNode)
	{

		// 한걸음 이하인지 확인한다.
		if (range(AlienPosition, targetNode) < 1){
			return targetNode;
		}

		else{
			// 첫노드가 갈수 없는 노드일때. 
			if (!FindNearistNode(PlayerPosition))
				return false;
			else
				nowNode = FindNearistNode(PlayerPosition));
		}

		pNode = NULL;

		while (true){

			float right_value = 0;
			float left_value = 0;
			float up_value= 0;
			float down_value = 0;

			if (nowNode){

				// 노드가 전에 사용 됐는지확인
				if (Map[nowNode.x][nowNode.y].walked){
					// 해당 노드의 삭제
					if (pNode) pNode = path.erase(pNode)

					// 벡터에 다음 노드가 있는지 확인
					if (pNode == path.end())
						return false;
				}

				// 현재 노드의 사용을 표시한다.
				Map[nowNode.x][nowNode.y].walked = true;

				// 4방면 각노드의 최종값 계산
				if (nowNode.x - 1 > 0){
					// 갔었는지 확인
					if (!Map[nowNode.x - 1][nowNode.y].walked){

					}
				}

			}
		}
		// 4진트리 처리 방식에 따라서(방향은 그다지 상관없음) 각 노드의 깊이와 각 노드에서 도착점까지의 거리를 저장해서 리스트에 최종값과 함께 좌표 인덱스를 저장.
		//if (각 4진트리의 walked 가 false인지 확인 ){

		// 최종값을 대입

		// 리스트에 최종값을 비교해서 삽입

		//}
	}



		// 루프.. End 도착지점과 한걸음 이하일 때까지

		// 리스트에 최종 도착점을 넣어야 함.

	}

};