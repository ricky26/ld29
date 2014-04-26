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

#endif //_PHYSICS_H_
