#include "pch.h"
#include "Object.h"


Object::Object()
{
	objectInfo = new Protocol::ObjectInfo();
	posInfo = objectInfo->mutable_pos_info();
}

Object::~Object()
{
	delete objectInfo;
}
