#ifndef  _LIGHT_H_
#define  _LIGHT_H_

#include "renderable.h"
#include "physics.h"
#include "resources.h"

class Game;

namespace Entities
{
	class Light: public Renderable
	{
	public:
		struct Position
		{
			b2Vec2 position;
			b2Vec2 normal;
			b2Body* body;
		};

		typedef std::vector<Position> Positions;

		Light();

		inline BodyHandle const& body() const { return m_body; }

		inline void setGame(Game *_game) { m_game = _game; }
		inline void setWorld(b2World *_world) { m_world = _world; }

		void init();

		virtual void update(Update &_update) override;
		virtual void render() override;
		virtual void renderLight() override;

	private:
		Game* m_game;

		b2World *m_world;

		BodyHandle m_body;

		Positions m_positions;

		bool m_obscured;

		SDL::Texture m_texLight;
		SDL::Texture m_texObj;
	};
}

#endif //_LIGHT_H_
