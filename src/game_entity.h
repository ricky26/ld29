#ifndef  _GAME_SHAPE_H_
#define  _GAME_SHAPE_H_

#include "physics.h"
#include "renderable.h"

namespace Entities
{
	class Box: public Renderable
	{
	public:
		inline Box() : m_illuminated(false), m_dragJoint(nullptr) {}

		inline BodyHandle& body() { return m_body; }

		inline bool isIlluminated() const { return m_illuminated; }
		inline void setIlluminated(bool _i) { m_illuminated = _i; }

		inline b2Joint *dragJoint() const { return m_dragJoint; }
		inline void setDragJoint(b2Joint *_dragJoint) { m_dragJoint = _dragJoint; }

		void create(b2World &_world, b2Vec2 _size, bool _static=false);

		virtual void render() override;

	private:
		BodyHandle m_body;
		b2Vec2 m_size;
		bool m_illuminated;

		b2Joint* m_dragJoint;
	};

	class Rope: public Renderable
	{
	public:
		Rope() {}


	private:
		
	};
}

#endif //_GAME_SHAPE_H_
