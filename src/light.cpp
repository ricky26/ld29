#include "light.h"
#include "game.h"
#include "game_entity.h"
#include "opengl.h"

namespace
{
	struct MinRayCast: public b2RayCastCallback
	{
		inline MinRayCast() : fixture(NULL), ignore(NULL) {}

		virtual float ReportFixture(b2Fixture *_fixture, const b2Vec2 &_p, const b2Vec2 &_normal, float _frac) override
		{
			if(ignore && (_fixture->GetBody() == ignore))
				return -1;

			fixture = _fixture;
			pos = _p;
			normal = _normal;

/*
			const float xray = worldToPhysics(20.f);
			pos.x -= _normal.x * xray;
			pos.y -= _normal.y * xray;
*/

			return _frac;
		}

		b2Body* ignore;
		b2Fixture* fixture;
		b2Vec2 pos;
		b2Vec2 normal;
	};

	struct PickObject: public b2QueryCallback
	{
		inline PickObject() : fixture(nullptr), ignore(NULL) {}

		virtual bool ReportFixture(b2Fixture *_fixture)
		{
			if(ignore && (_fixture->GetBody() == ignore))
				return true;

			fixture = _fixture;
			return false;
		}

		b2Body* ignore;
		b2Fixture* fixture;
	};
}

namespace Entities
{
	static const float g_maxDist = 1024.f;

	Light::Light()
		: m_world(nullptr)
	{
		m_positions.resize(720);
	}

	void Light::init()
	{
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.userData = this;
		
		b2Body *body = m_world->CreateBody(&bodyDef);

		b2CircleShape shape;
		shape.m_radius = worldToPhysics(50);
		body->CreateFixture(&shape, 1);

		m_body.reset(*m_world, body);

		m_texLight = m_game->loadTexture("textures/lightsource.png");
		m_texObj = m_game->loadTexture("textures/bulb.png");
	}
	
	void Light::update(Update &_update)
	{
		const int passes = m_positions.size();
		const b2Vec2 pos = physicsToWorld(m_body->GetPosition());

		if(m_game)
		{
			for(auto &item : m_game->renderables())
			{
				if(Entities::Box *box = dynamic_cast<Entities::Box*>(item.get()))
				{
					box->setIlluminated(false);
				}
			}
		}
		
		b2Vec2 firstPos, lastPos;

		PickObject picker;
		picker.ignore = m_body.body();

		b2AABB point;
		point.lowerBound = point.upperBound = worldToPhysics(pos);
		if(m_world)
			m_world->QueryAABB(&picker, point);
		if(picker.fixture)
		{
			if(Entities::Box *box = dynamic_cast<Entities::Box*>(
				   reinterpret_cast<Renderable*>(
					   picker.fixture->GetBody()->GetUserData())))
				box->setIlluminated(true);
			
			m_obscured = true;
			return;
		}

		m_obscured = false;
		for(int i = 0; i < passes; i++)
		{
			const float angle = (i / float(passes-1)) * M_PI * 2.f;

			b2Vec2 worstPos = {
				pos.x + cosf(angle)*g_maxDist*2, 
				pos.y + sinf(angle)*g_maxDist*2 };
			b2Vec2 endPos = {
				pos.x + cosf(angle)*g_maxDist, 
				pos.y + sinf(angle)*g_maxDist };

			MinRayCast caster;
			caster.ignore = m_body.body();
			caster.pos = worldToPhysics(worstPos);

			if(m_world)
				m_world->RayCast(&caster, worldToPhysics(pos), worldToPhysics(endPos));

			if(caster.fixture)
			{
				m_positions[i].body = caster.fixture->GetBody();

				if(Entities::Box *box = dynamic_cast<Entities::Box*>(
					   reinterpret_cast<Renderable*>(
						   caster.fixture->GetBody()->GetUserData())))
					box->setIlluminated(true);
			}
			else
			{
				m_positions[i].body = nullptr;
			}

			b2Vec2 worldPos = physicsToWorld(caster.pos) - pos;
			m_positions[i].position = worldPos;
			m_positions[i].normal = caster.normal;
		}
	}

	void Light::render()
	{
		const b2Vec2 pos = physicsToWorld(m_body->GetPosition());
		const int passes = 128;
		const float r = 50.f;

		glBegin(GL_TRIANGLES);
		for(int i = 0; i <= passes; i++)
		{
			const float angle = (i / float(passes-1)) * M_PI * 2;
			const float prevAngle = ((i-1) / float(passes-1)) * M_PI * 2;
			
			const float x = cosf(angle) * r;
			const float y = sinf(angle) * r;

			const float pX = cosf(prevAngle) * r;
			const float pY = sinf(prevAngle) * r;

			glColor3f(1, 1, 0);
			glVertex2f(pos.x, pos.y);
			glColor3f(1, 1, 0);
			glVertex2f(pos.x + pX, pos.y + pY);
			glColor3f(1, 1, 0);
			glVertex2f(pos.x + x, pos.y + y);
		}
		glEnd();
	}

	void Light::renderLight()
	{
		const b2Vec2 pos = physicsToWorld(m_body->GetPosition());
		
		const float r = 1;
		const float g = 1;
		const float b = 0.95f;
		const float a = 0.85f;

		if(!m_obscured)
		{
			float texW, texH;
			SDL_GL_BindTexture(m_texLight.get(), &texW, &texH);
			glBegin(GL_TRIANGLES);
			for(int i = 0; i <= m_positions.size(); i++)
			{
				const Position &lastt = i ? m_positions[i-1] : m_positions.back();
				const Position &thist = (i < m_positions.size()) ? m_positions[i] : m_positions.front();

				b2Vec2 lastPos = pos + lastt.position;
				b2Vec2 thisPos = pos + thist.position;

				const float fDistMult = g_maxDist * 2.f;

				glTexCoord2f(0.5f*texW, 0.5f*texH);
				glColor4f(r, g, b, a);
				glVertex2f(pos.x, pos.y);
				glTexCoord2f((0.5f + lastt.position.x / fDistMult)*texW, (0.5f + lastt.position.y / fDistMult)*texH);
				glColor4f(r, g, b, a);
				glVertex2f(lastPos.x, lastPos.y);
				glTexCoord2f((0.5f + thist.position.x / fDistMult)*texW, (0.5f + thist.position.y / fDistMult)*texH);
				glColor4f(r, g, b, a);
				glVertex2f(thisPos.x, thisPos.y);

				if(lastt.body && (lastt.body == thist.body))
				{
					const float xray = 40.f;

					b2Vec2 lastn = { -lastt.normal.x * xray, -lastt.normal.y * xray };
					b2Vec2 thisn = { -thist.normal.x * xray,  -thist.normal.y * xray };

					b2Vec2 lastnu = lastt.position + lastn;
					b2Vec2 thisnu = thist.position + lastn;

					b2Vec2 lastnPos = lastPos + lastn;
					b2Vec2 thisnPos = thisPos + thisn;

					glTexCoord2f((0.5f + lastt.position.x / fDistMult)*texW, (0.5f + lastt.position.y / fDistMult)*texH);
					glColor4f(r, g, b, a);
					glVertex2f(lastPos.x, lastPos.y);
					glTexCoord2f((0.5f + thisnu.x / fDistMult)*texW, (0.5f + thisnu.y / fDistMult)*texH);
					glColor4f(r, g, b, 0);
					glVertex2f(thisnPos.x, thisnPos.y);
					glTexCoord2f((0.5f + lastnu.x / fDistMult)*texW, (0.5f + lastnu.y / fDistMult)*texH);
					glColor4f(r, g, b, 0);
					glVertex2f(lastnPos.x, lastnPos.y);


					glTexCoord2f((0.5f + lastt.position.x / fDistMult)*texW, (0.5f + lastt.position.y / fDistMult)*texH);
					glColor4f(r, g, b, a);
					glVertex2f(lastPos.x, lastPos.y);
					glTexCoord2f((0.5f + thist.position.x / fDistMult)*texW, (0.5f + thist.position.y / fDistMult)*texH);
					glColor4f(r, g, b, a);
					glVertex2f(thisPos.x, thisPos.y);
					glTexCoord2f((0.5f + thisnu.x / fDistMult)*texW, (0.5f + thisnu.y / fDistMult)*texH);
					glColor4f(r, g, b, 0);
					glVertex2f(thisnPos.x, thisnPos.y);
				}
			}
			glEnd();
			SDL_GL_UnbindTexture(m_texLight.get());
		}
	}
}
