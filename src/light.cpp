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
			if(!_fixture->TestPoint(position))
				return true;

			if(ignore && (_fixture->GetBody() == ignore))
				return true;

			fixture = _fixture;
			return false;
		}

		b2Vec2 position;
		b2Body* ignore;
		b2Fixture* fixture;
	};
}

namespace Entities
{
	static const float g_maxDist = 2048.f;

	static void illuminate(Renderable *r)
	{
		if(Box *box = dynamic_cast<Box*>(r))
			box->setIlluminated(true);
		else if(Light *light = dynamic_cast<Light*>(r))
			light->setActive(true);
	}

	Light::Light()
		: m_world(nullptr)
		, m_scale(1)
		, m_active(false)
	{
		m_positions.resize(720);
	}

	void Light::init()
	{
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.userData = this;
		
		b2Body *body = m_world->CreateBody(&bodyDef);

		const float r = 50 * m_scale;
		const float pr = worldToPhysics(r);

		b2CircleShape shape;
		shape.m_radius = pr;

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 1.5f;
		fixtureDef.friction = 0.1f;
		body->CreateFixture(&fixtureDef);

		m_body.reset(*m_world, body);

		m_texLight = m_game->loadTexture("textures/lightsource.png");
		//m_texObj = m_game->loadTexture("textures/bulb.png");
		m_texObj = m_game->loadGLTexture("textures/bulb.png");
	}
	
	void Light::update(Update &_update)
	{
		if(!m_active)
		{
			m_obscured = false;
			return;
		}

		const int passes = m_positions.size();
		const b2Vec2 pos = physicsToWorld(m_body->GetPosition());
		
		b2Vec2 firstPos, lastPos;

		PickObject picker;
		picker.ignore = m_body.body();
		picker.position = m_body->GetPosition();

		b2AABB point;
		point.lowerBound = point.upperBound = worldToPhysics(pos);
		if(m_world)
			m_world->QueryAABB(&picker, point);
		if(false && picker.fixture)
		{
			illuminate(reinterpret_cast<Renderable*>(picker.fixture->GetBody()->GetUserData()));
			m_obscured = true;
			return;
		}

		m_obscured = false;
		for(int i = 0; i < passes; i++)
		{
			const float angle = (i / float(passes-1)) * M_PI * 2.f;

			b2Vec2 worstPos = {
				pos.x + cosf(angle)*g_maxDist*m_scale*2, 
				pos.y + sinf(angle)*g_maxDist*m_scale*2 };
			b2Vec2 endPos = {
				pos.x + cosf(angle)*g_maxDist*m_scale, 
				pos.y + sinf(angle)*g_maxDist*m_scale };

			MinRayCast caster;
			caster.ignore = m_body.body();
			caster.pos = worldToPhysics(worstPos);

			if(m_world)
				m_world->RayCast(&caster, worldToPhysics(pos), worldToPhysics(endPos));

			if(caster.fixture)
			{
				illuminate(reinterpret_cast<Renderable*>(caster.fixture->GetBody()->GetUserData()));
				m_positions[i].body = caster.fixture->GetBody();
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
		const float angle = m_body->GetAngle();
		const int passes = 128;
		const float r = 50.f*m_scale;

		float w=128*m_scale, h=128*m_scale;
		//SDL_GL_BindTexture(m_texObj.get(), &w, &h);
		glBindTexture(GL_TEXTURE_2D, m_texObj);
		glBegin(GL_QUADS);

		float uw=1, uh=1;
		
		b2Vec2 pts[] = {
			{ -w*0.5f, -h*0.5f }, { 0, uh },
			{ w*0.5f, -h*0.5f }, { uw, uh },
			{ w*0.5f, h*0.5f }, { uw, 0 },
			{ -w*0.5f, h*0.5f }, { 0, 0 },
		};

		const float cosA = cosf(angle);
		const float sinA = sinf(angle);
		for(int i = 0; i < sizeof(pts)/(2*sizeof(pts[0])); i++)
		{
			b2Vec2 lpos = pts[i*2];
			b2Vec2 uv = pts[i*2 + 1];

			b2Vec2 xpos = {
				cosA*lpos.x - sinA*lpos.y,
				sinA*lpos.x + cosA*lpos.y };
			
			glTexCoord2f(uv.x, uv.y);
			glColor3f(1,1,1);
			glVertex2f(pos.x + xpos.x, pos.y + xpos.y);
		}

		glEnd();
		//SDL_GL_UnbindTexture(m_texObj.get());
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Light::renderLight()
	{
		const b2Vec2 pos = physicsToWorld(m_body->GetPosition());
		
		const float r = 1;
		const float g = 1;
		const float b = 0.95f;

		float texW, texH;
		SDL_GL_BindTexture(m_texLight.get(), &texW, &texH);

		if(m_active && !m_obscured)
		{
			const float a = 0.85f;

			glBegin(GL_TRIANGLES);
			for(int i = 0; i <= m_positions.size(); i++)
			{
				const Position &lastt = i ? m_positions[i-1] : m_positions.back();
				const Position &thist = (i < m_positions.size()) ? m_positions[i] : m_positions.front();

				b2Vec2 lastPos = pos + lastt.position;
				b2Vec2 thisPos = pos + thist.position;

				const float fDistMult = g_maxDist * m_scale * 2.f;

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
		}

		if(!m_obscured)
		{
			const float sightRange = m_active ? 1.f : 0.1f;
			const float a = m_active ? 1 : 0.3f;

			glBegin(GL_QUADS);

			glTexCoord2f(0, texH);
			glColor4f(r, g, b, a);
			glVertex2f(pos.x - texW*0.5f*m_scale*sightRange, pos.y - texH*0.5f*m_scale*sightRange);
			glTexCoord2f(texW, texH);
			glColor4f(r, g, b, a);
			glVertex2f(pos.x + texW*0.5f*m_scale*sightRange, pos.y - texH*0.5f*m_scale*sightRange);
			glTexCoord2f(texW, 0);
			glColor4f(r, g, b, a);
			glVertex2f(pos.x + texW*0.5f*m_scale*sightRange, pos.y + texH*0.5f*m_scale*sightRange);
			glTexCoord2f(0, 0);
			glColor4f(r, g, b, a);
			glVertex2f(pos.x - texW*0.5f*m_scale*sightRange, pos.y + texH*0.5f*m_scale*sightRange);
			
			glEnd();
		}
		SDL_GL_UnbindTexture(m_texLight.get());
	}
}
