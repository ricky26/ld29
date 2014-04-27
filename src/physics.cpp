#include "physics.h"

// BodyHandle

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

// BodyHandle

void JointHandle::destroy()
{
	if(m_joint)
		m_world->DestroyJoint(m_joint);
}

void JointHandle::reset(b2World &_world, b2Joint *_joint)
{
	if(m_joint == _joint)
		return;

	m_world = &_world;
	m_joint = _joint;
}

// Functions

const float g_physicsScale = 1e-2f;

float physicsToWorld(float _v)
{
	return _v / g_physicsScale;
}

float worldToPhysics(float _v)
{
	return _v * g_physicsScale;
}


b2Vec2 physicsToWorld(b2Vec2 const& _a)
{
	return b2Vec2(_a.x / g_physicsScale,
				  _a.y / g_physicsScale);
}

b2Vec2 worldToPhysics(b2Vec2 const& _a)
{
	return b2Vec2(_a.x * g_physicsScale,
				  _a.y * g_physicsScale);
}
