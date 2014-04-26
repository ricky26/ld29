#include "physics.h"

void BodyHandle::destroy()
{
	if(m_body)
		m_world->DestroyBody(m_body);
}

void BodyHandle::reset(b2World &_world, b2Body *_body)
{
	if(m_body == _body)
		return;

	m_world = &_world;
	m_body = _body;
}
