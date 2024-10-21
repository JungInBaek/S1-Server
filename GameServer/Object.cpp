#include "pch.h"
#include "Object.h"


Object::Object()
{
	
}

Object::~Object()
{

}

Protocol::ObjectInfo Object::changeToPacket()
{
	Protocol::ObjectInfo object;
	object.set_object_id(objectId);
	object.set_object_type(objectType);

	Protocol::PosInfo* pos = object.mutable_pos_info();
	pos->set_yaw(yaw);

	Protocol::VectorInfo* vector = pos->mutable_vector_info();
	vector->set_x(position.x);
	vector->set_y(position.y);
	vector->set_z(position.z);

	return object;
}
