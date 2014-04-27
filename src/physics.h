#ifndef  _PHYSICS_H_
#define  _PHYSICS_H_

#include <Box2D/Box2D.h>

class BodyHandle
{
public:
	inline BodyHandle()
		: m_world(nullptr)
		, m_body(nullptr)
	{}

	inline BodyHandle(BodyHandle const& _b)
		: m_world(_b.m_world)
		, m_body(nullptr)
	{}
	
	inline BodyHandle(BodyHandle &&_b)
		: m_world(_b.m_world)
		, m_body(_b.m_body)
	{
		_b.m_body = nullptr;
	}

	inline ~BodyHandle() { destroy(); }

	inline bool valid() const { return m_body != nullptr; }

	b2World *world() { return m_world; }
	b2Body *body() const { return m_body; }

	inline b2Body *operator ->() const { return body(); }

	void destroy();
	void reset(b2World &_world, b2Body *_body);

private:
	b2World *m_world;
	b2Body *m_body;
};

class JointHandle
{
public:
	inline JointHandle()
		: m_world(nullptr)
		, m_joint(nullptr)
	{}

	inline JointHandle(JointHandle const& _b)
		: m_world(_b.m_world)
		, m_joint(nullptr)
	{}
	
	inline JointHandle(JointHandle &&_b)
		: m_world(_b.m_world)
		, m_joint(_b.m_joint)
	{
		_b.m_joint = nullptr;
	}

	inline ~JointHandle() { destroy(); }

	inline bool valid() const { return m_joint != nullptr; }

	b2World *world() { return m_world; }
	b2Joint *joint() const { return m_joint; }

	inline b2Joint *operator ->() const { return joint(); }

	void destroy();
	void reset(b2World &_world, b2Joint *_body);

private:
	b2World *m_world;
	b2Joint *m_joint;
};

float physicsToWorld(float);
float worldToPhysics(float);
b2Vec2 physicsToWorld(b2Vec2 const&);
b2Vec2 worldToPhysics(b2Vec2 const&);

#endif //_PHYSICS_H_
