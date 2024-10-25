#include "pch.h"
#include "S1Node.h"


S1Node::S1Node()
{
}

S1Node::S1Node(const S1Node& node)
	: position(node.position), GCost(node.GCost), HCost(node.HCost), parent(node.parent)
{
}

S1Node::S1Node(const S1Vector& position, float GCost, float HCost, S1Node* parent)
	: position(position), GCost(GCost), HCost(HCost), parent(parent)
{
}
