#include "game_entity.h"
#include <iostream>
#include "opengl.h"
#include "light.h"

namespace Entities
{
	// Hidden classes

	class BasicBox: public Box
	{
	public:
		BasicBox(Factory &_factory, b2Vec2 _size, bool _static)
		{
			m_size = _size;

			b2BodyDef bodyDef;
			bodyDef.userData = (Renderable*)this;
			if(!_static)
				bodyDef.type = b2_dynamicBody;
			b2Body *body = _factory.world()->CreateBody(&bodyDef);

			b2Vec2 size = worldToPhysics(b2Vec2(_size.x*0.5f, _size.y*0.5f));

			b2PolygonShape boxShape;
			boxShape.SetAsBox(size.x, size.y);

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &boxShape;
			if(!_static)
				fixtureDef.density = 1;
			fixtureDef.friction = 0.9f;
			body->CreateFixture(&fixtureDef);

			setBody(BodyHandle(*_factory.world(), body));
		}
		
		virtual void render() override
		{
			b2Vec2 vec = physicsToWorld(body()->GetPosition());
			float angle = body()->GetAngle();

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
				if(isIlluminated())
					glColor4f(0, 1, 0, 1);
				else
					glColor4f(0.5, 0.5, 0, 1);
				glVertex2f(vec.x + verts[i].x, vec.y + verts[i].y);
			}

			glEnd();
		}

	protected:
		b2Vec2 m_size;
	};

	class TexBox: public BasicBox
	{
	public:
		TexBox(Factory &_factory, b2Vec2 _size, GLuint _texture, UVs const& _uvs,
			   float _padding = 0, bool _static = false)
			: BasicBox(_factory, _size, _static)
			, m_uvs(_uvs)
			, m_texture(_texture)
			, m_padding(_padding)
		{}

		virtual void render() override
		{
			b2Vec2 vec = physicsToWorld(body()->GetPosition());
			float angle = body()->GetAngle();

			b2Vec2 size = { m_size.x * (0.5f + m_padding),
							m_size.y * (0.5f + m_padding) };

			b2Vec2 verts[] = {
				{ -size.x, -size.y }, { m_uvs.u, m_uvs.t },
				{ +size.x, -size.y }, { m_uvs.s, m_uvs.t },
				{ +size.x, +size.y }, { m_uvs.s, m_uvs.v },
				{ -size.x, +size.y }, { m_uvs.u, m_uvs.v },
			};

			const float cosA = cosf(angle);
			const float sinA = sinf(angle);

			glBindTexture(GL_TEXTURE_2D, m_texture);
			glBegin(GL_QUADS);

			for(int i = 0; i < (sizeof(verts)/sizeof(verts[0]))/2; i++)
			{
				b2Vec2 const& pos = verts[i << 1];
				b2Vec2 const& uvs = verts[(i << 1)|1];

				b2Vec2 rpos = { 
					cosA*pos.x - sinA*pos.y,
					sinA*pos.x + cosA*pos.y
				};

				glTexCoord2f(uvs.x, uvs.y);
				glColor4f(1,1,1,1);
				glVertex2f(vec.x + rpos.x, vec.y + rpos.y);
			}

			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
		}

	private:
		GLuint m_texture;
		UVs m_uvs;
		float m_padding;
	};

	// Factory

	static void mapUVs(UVs &_uvs)
	{
		const float w = 1024;
		const float h = 1024;

		_uvs.s += _uvs.u;
		_uvs.t += _uvs.v;
		
		_uvs.u /= w;
		_uvs.v /= h;
		_uvs.s /= w;
		_uvs.t /= h;

		std::swap(_uvs.v, _uvs.t);
	}

	Factory::Factory()
		: m_texture(0)
		, m_bricks(0)
		, m_world(nullptr)
		, m_resources(nullptr)
	{
	}

	Factory::~Factory()
	{
		glDeleteTextures(1, &m_texture);
		glDeleteTextures(1, &m_bricks);
	}

	void Factory::init()
	{
		m_texture = m_resources->loadGLTexture("textures/sprites.png");
		m_bricks = m_resources->loadGLTexture("textures/bricks.png");
	}
	
	Box *Factory::createBox(b2Vec2 const& _size, bool _static)
	{
		return new BasicBox(*this, _size, _static);
	}

	Box *Factory::createChest()
	{
		UVs uvs = { 0, 0, 256, 256 };
		mapUVs(uvs);

		return new TexBox(*this, b2Vec2(60, 60), m_texture, uvs, 0.1f);
	}
	
	Box *Factory::createDirt()
	{
		UVs uvs = { 256, 128, 128, 128 };
		mapUVs(uvs);

		if((rand() / float(RAND_MAX)) > 0.7f)
		{
			UVs uvs2 = { 256, 0, 128, 128 };
			uvs = uvs2;
			mapUVs(uvs);
		}

		return new TexBox(*this, b2Vec2(50, 50), m_texture, uvs, 0.2f);
	}

	Box *Factory::createBricks(b2Vec2 const& _size, bool _static)
	{
		UVs uvs = { 0, 0, 1, 1 };

		const float div = std::min(_size.x, _size.y);
		uvs.s = _size.x / div;
		uvs.t = _size.y / div;

		return new TexBox(*this, _size, m_bricks, uvs, 0, _static);
	}

	Box *Factory::createHalfBricks(b2Vec2 const& _size, bool _static)
	{
		UVs uvs = { 0, 0, 1, 1 };

		const float div = std::min(_size.x, _size.y) * 2;
		uvs.s = _size.x / div;
		uvs.t = _size.y / div;

		return new TexBox(*this, _size, m_bricks, uvs, 0, _static);
	}

	Light *Factory::createLight()
	{
		std::unique_ptr<Light> ret(new Light());

		ret->setWorld(m_world);
		ret->setGame(m_game);

		return ret.release();
	}
}
