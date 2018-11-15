#include "BoundingBox.h"

BoundingBox::BoundingBox()
{
	// infinite small 
	_min.x = Infinity;
	_min.y = Infinity;
	_min.z = Infinity;

	_max.x = Infinity;
	_max.y = Infinity;
	_max.z = Infinity;
}

bool BoundingBox::isPointInside(D3DXVECTOR3& p)
{
	if (p.x >= _min.x && p.y >= _min.y && p.z >= _min.z &&
		p.x <= _max.x && p.y <= _max.y && p.z <= _max.z)
	{
		return true;
	}
	else
	{
		return false;
	}
}