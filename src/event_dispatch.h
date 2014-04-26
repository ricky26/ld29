#ifndef  _EVENT_DISPATCH_H_
#define  _EVENT_DISPATCH_H_

#include <SDL2/SDL_events.h>
#include <stack>
#include "update.h"

class IEventDispatch
{
public:
	virtual bool handleEvent(SDL_Event &tEvent) { return false; }
	virtual void handleIdle() {}
	virtual void render() {}
	virtual void update(Update&) {}
};

class EventDispatch: public IEventDispatch
{
public:
	virtual bool handleEvent(SDL_Event &tEvent) override;
	virtual void handleIdle() override;

	virtual void onInit() {}
	virtual void onShow() {}
	virtual void onHide() {}
	virtual void onShutdown() {}

	virtual void onIdle() {};
	virtual bool onQuit() { return false; };

	virtual bool onKeyDown(const SDL_KeyboardEvent &tEvent) { return false; }
	virtual bool onKeyUp(const SDL_KeyboardEvent &tEvent) { return false; }

	virtual bool onMouseMotion(const SDL_MouseMotionEvent &tEvent) { return false; }
	virtual bool onMouseDown(const SDL_MouseButtonEvent &tEvent) { return false; }
	virtual bool onMouseUp(const SDL_MouseButtonEvent &tEvent) { return false; }
	virtual bool onMouseWheel(const SDL_MouseWheelEvent &tEvent) { return false; }
};

class DispatchStack: public IEventDispatch
{
public:
	typedef std::deque<EventDispatch*> Stack;

	inline Stack& container() { return m_stack; }

	void push(EventDispatch &tDispatch);
	void pop(EventDispatch &tDispatch);

	// IEventDispatch
	virtual bool handleEvent(SDL_Event &tEvent) override;
	virtual void handleIdle() override;
	virtual void render() override;
	virtual void update(Update&) override;
	
	static DispatchStack &get();

private:
	Stack m_stack;
};

#endif //_EVENT_DISPATCH_H_
