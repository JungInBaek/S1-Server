#include "pch.h"
#include "S1Node.h"


S1Node::S1Node()
{
}

S1Node::S1Node(const S1Vector& position, float GCost, float HCost)
	: position(position), GCost(GCost), HCost(HCost)
{
}

S1Node::~S1Node()
{
	//cout << "~S1Node" << endl;
	parent = shared_ptr<S1Node>();
}
