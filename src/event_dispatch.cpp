#include "event_dispatch.h"

bool EventDispatch::handleEvent(SDL_Event &tEvent)
{
	switch(tEvent.type)
	{
	case SDL_KEYDOWN:
		return onKeyDown(tEvent.key);

	case SDL_KEYUP:
		return onKeyUp(tEvent.key);

	case SDL_MOUSEMOTION:
		return onMouseMotion(tEvent.motion);
		
	case SDL_MOUSEBUTTONDOWN:
		return onMouseDown(tEvent.button);

	case SDL_MOUSEBUTTONUP:
		return onMouseUp(tEvent.button);

	case SDL_MOUSEWHEEL:
		return onMouseWheel(tEvent.wheel);

	case SDL_QUIT:
		return onQuit();
	}

	return false;
}

void EventDispatch::handleIdle()
{
	onIdle();
}

// DispatchStack

void DispatchStack::push(EventDispatch &tDispatch)
{
	if(!m_stack.empty())
		m_stack.back()->onHide();
	
	m_stack.push_back(&tDispatch);
	tDispatch.onInit();
	tDispatch.onShow();
}

void DispatchStack::pop(EventDispatch &tDispatch)
{
	if(m_stack.back() == &tDispatch)
	{
		m_stack.back()->onShutdown();
		m_stack.pop_back();

		if(!m_stack.empty())
			m_stack.back()->onShow();
	}
}

bool DispatchStack::handleEvent(SDL_Event &tEvent)
{
	for(auto it = m_stack.rbegin(); it != m_stack.rend(); ++it)
	{
		if((*it)->handleEvent(tEvent))
			return true;
	}

	return false;
}

void DispatchStack::handleIdle()
{
	for(auto it = m_stack.begin(); it != m_stack.end(); ++it)
		(*it)->handleIdle();
}

void DispatchStack::render()
{
	for(auto it = m_stack.begin(); it != m_stack.end(); ++it)
		(*it)->render();
}

void DispatchStack::update(Update &_us)
{
	for(auto it = m_stack.begin(); it != m_stack.end(); ++it)
		(*it)->update(_us);
}
	
DispatchStack &DispatchStack::get()
{
	static DispatchStack stack;
	return stack;
}
