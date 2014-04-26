#ifndef  _GAME_H_
#define  _GAME_H_

#include <Box2D/Box2D.h>
#include "event_dispatch.h"
#include "window.h"
#include "renderable.h"

class Game: public EventDispatch
{
public:
	typedef std::vector<std::unique_ptr<Renderable>> Renderables;

	Game();
	~Game();

	void updateCamera();

	virtual void render() override;
	virtual void update(Update& _update) override;

	virtual void onInit() override;

	virtual void onIdle() override;

	// Input

	virtual bool onMouseWheel(const SDL_MouseWheelEvent &tEvent) override;

private:
	Window m_window;
	Renderables m_renderables;

	float m_cameraOffset;

	// Physics
	float m_physTime;
	b2World m_world;
};

#endif //_GAME_H_
