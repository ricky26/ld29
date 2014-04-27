#ifndef  _GAME_H_
#define  _GAME_H_

#include <Box2D/Box2D.h>
#include "event_dispatch.h"
#include "window.h"
#include "renderable.h"
#include "opengl.h"
#include "resources.h"

class b2MouseJoint;
class b2Body;

namespace Entities
{
	class Light;
};

class Game: public EventDispatch, public ResourceManager
{
public:
	typedef std::vector<std::unique_ptr<Renderable>> Renderables;

	Game();
	~Game();

	inline Renderables const& renderables() const { return m_renderables; }

	void updateCamera();

	virtual void render() override;
	virtual void update(Update& _update) override;

	virtual void onInit() override;
	virtual void onShutdown() override;

	virtual void onIdle() override;

	// Input
	virtual bool onMouseMotion(const SDL_MouseMotionEvent &_event) override;
	virtual bool onMouseDown(const SDL_MouseButtonEvent &_event) override;
	virtual bool onMouseUp(const SDL_MouseButtonEvent &_event) override;
	virtual bool onMouseWheel(const SDL_MouseWheelEvent &_event) override;

	// ResourceManager

	virtual SDL::Texture loadTexture(std::string const& _path) override;

private:
	Window m_window;

	// Physics
	float m_physTime;
	b2World m_world;

	// Scene
	SDL::Texture m_background;
	Renderables m_renderables;
	float m_cameraOffset;

	// Grab
	b2Body*			m_ground;
	Renderable*		m_grabbed;
	b2MouseJoint* 	m_grab;
	bool m_isRotating;
	bool m_wasFixed;

	// Lighting
	Entities::Light* m_defaultLight;
	GLuint m_lightTexture;
	GLuint m_lightFB;
};

#endif //_GAME_H_
