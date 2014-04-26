#include "game_entity.h"
#include <iostream>
#include <SDL2/SDL_opengl.h>

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

		b2PolygonShape groundBox;
		groundBox.SetAsBox(_size.x*0.5f, _size.y*0.5f);

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
		b2Vec2 vec = m_body->GetPosition();
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
			glColor3f(0, 1, 0);
			glVertex2f(vec.x + verts[i].x, vec.y + verts[i].y);
		}

		glEnd();
	}
}
