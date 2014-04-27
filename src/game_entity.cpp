#include "game_entity.h"
#include <iostream>
#include "opengl.h"

namespace Entities
{
	void Box::create(b2World &_world, b2Vec2 _size, bool _static)
	{
		m_size = _size;

		b2BodyDef bodyDef;
		bodyDef.userData = (Renderable*)this;
		if(!_static)
			bodyDef.type = b2_dynamicBody;
		b2Body *body = _world.CreateBody(&bodyDef);

		b2Vec2 size = worldToPhysics(b2Vec2(_size.x*0.5f, _size.y*0.5f));

		b2PolygonShape groundBox;
		groundBox.SetAsBox(size.x, size.y);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &groundBox;
		if(!_static)
			fixtureDef.density = 1;
		fixtureDef.friction = 0.3f;
		body->CreateFixture(&fixtureDef);

		m_body.reset(_world, body);
	}

	void Box::render()
	{
		b2Vec2 vec = physicsToWorld(m_body->GetPosition());
		float angle = m_body->GetAngle();

		b2Vec2 verts[] = {
			{ -m_size.x*0.5f, -m_size.y*0.5f },
			{ +m_size.x*0.5f, -m_size.y*0.5f },
			{ +m_size.x*0.5f, +m_size.y*0.5f },
			{ -m_size.x*0.5f, +m_size.y*0.5f },
		};

		const float cosA = cosf(angle);
		const float sinA = sinf(angle);

		for(int i = 0; i < sizeof(verts)/sizeof(verts[0]); i++)
		{
			b2Vec2 &vert = verts[i];

			b2Vec2 newVec = { 
				cosA*vert.x - sinA*vert.y,
				sinA*vert.x + cosA*vert.y
			};
			vert = newVec;
		}

		glBegin(GL_QUADS);

		for(int i = 0; i < sizeof(verts)/sizeof(verts[0]); i++)
		{
			if(m_illuminated)
				glColor4f(0, 1, 0, 1);
			else
				glColor4f(0.5, 0.5, 0, 1);
			glVertex2f(vec.x + verts[i].x, vec.y + verts[i].y);
		}

		glEnd();
	}
}
