#ifndef  _GAME_SHAPE_H_
#define  _GAME_SHAPE_H_

#include "physics.h"
#include "renderable.h"
#include "resources.h"

class Game;

namespace Entities
{
	class Light;

	class Box: public Renderable
	{
	public:
		inline Box() : m_illuminated(false), m_dragJoint(nullptr) {}

		inline BodyHandle& body() { return m_body; }
		inline void setBody(BodyHandle &&_body) { m_body = std::move(_body); }

		inline bool isIlluminated() const { return m_illuminated; }
		inline void setIlluminated(bool _i) { m_illuminated = _i; }

		inline b2Joint *dragJoint() const { return m_dragJoint; }
		inline void setDragJoint(b2Joint *_dragJoint) { m_dragJoint = _dragJoint; }

	private:
		BodyHandle m_body;
		bool m_illuminated;

		b2Joint* m_dragJoint;
	};

	class Rope: public Renderable
	{
	public:
		Rope() {}


	private:
		
	};

	class Factory
	{
	public:
		Factory();
		~Factory();

		inline ResourceManager* resources() const { return m_resources; }
		inline void setResources(ResourceManager *_r) { m_resources = _r; }

		inline b2World *world() const { return m_world; }
		inline void setWorld(b2World *_w) { m_world = _w; }

		inline Game *game() const { return m_game; }
		inline void setGame(Game *_g) { m_game = _g; }

		void init();

		Box *createBox(b2Vec2 const& _size, bool _static=false);
		Box *createChest();
		Box *createDirt();
		Box *createBricks(b2Vec2 const& _size, bool _static=false);
		Box *createHalfBricks(b2Vec2 const& _size, bool _static=false);

		Light *createLight();

	private:
		Game *m_game;
		b2World *m_world;
		ResourceManager *m_resources;

		GLuint m_texture;
		GLuint m_bricks;
	};


}

#endif //_GAME_SHAPE_H_
