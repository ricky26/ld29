#ifndef  _GAME_SHAPE_H_
#define  _GAME_SHAPE_H_

#include "physics.h"
#include "renderable.h"

namespace Entities
{
	class Box: public Renderable
	{
	public:
		inline BodyHandle& body() { return m_body; }

		void create(b2World &_world, b2Vec2 _size, bool _static=false);

		virtual void render() override;
		
	private:
		BodyHandle m_body;
		b2Vec2 m_size;
	};
}

#endif //_GAME_SHAPE_H_
