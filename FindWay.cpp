class Pos{
	
	bool walked;
};
//0���� 1���� �����س��� �������� �޾ƿͼ� ���� �ϳ� ��������.�׸��� �� �� ������ �Ʒ� �ʿ� ����.
Pos Map[500][500];		//�� ���� walked���� �Ǵ��ϴ� ��

class Node{ 
	int result_value; 
	int x;
	int y;
	int next_x;
	int next_y;
};
float range(Vector * Alien, Node * Player)
{
	//vector.x - NOde.x�� ����
	return sqrt(Alien - Player);
}
class RoadFind{

	Pos * pMap = Map;

	vector<Node> path;

	Node * nowNode;

	iterator::vector<Node> * pNode;

	//targetNode�� �÷��̾� ��ǥ
	Node * Find(Vector * AlienPosition, Node * targetNode)
	{

		// �Ѱ��� �������� Ȯ���Ѵ�.
		if (range(AlienPosition, targetNode) < 1){
			return targetNode;
		}

		else{
			// ù��尡 ���� ���� ����϶�. 
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

				// ��尡 ���� ��� �ƴ���Ȯ��
				if (Map[nowNode.x][nowNode.y].walked){
					// �ش� ����� ����
					if (pNode) pNode = path.erase(pNode)

					// ���Ϳ� ���� ��尡 �ִ��� Ȯ��
					if (pNode == path.end())
						return false;
				}

				// ���� ����� ����� ǥ���Ѵ�.
				Map[nowNode.x][nowNode.y].walked = true;

				// 4��� ������� ������ ���
				if (nowNode.x - 1 > 0){
					// �������� Ȯ��
					if (!Map[nowNode.x - 1][nowNode.y].walked){

					}
				}

			}
		}
		// 4��Ʈ�� ó�� ��Ŀ� ����(������ �״��� �������) �� ����� ���̿� �� ��忡�� ������������ �Ÿ��� �����ؼ� ����Ʈ�� �������� �Բ� ��ǥ �ε����� ����.
		//if (�� 4��Ʈ���� walked �� false���� Ȯ�� ){

		// �������� ����

		// ����Ʈ�� �������� ���ؼ� ����

		//}
	}



		// ����.. End ���������� �Ѱ��� ������ ������

		// ����Ʈ�� ���� �������� �־�� ��.

	}

};