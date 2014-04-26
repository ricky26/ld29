#ifndef  _WINDOW_H_
#define  _WINDOW_H_

#include <SDL2/SDL.h>

class Window
{
public:
	Window();
	~Window();

	inline SDL_Window* window() const { return m_window; }
	inline SDL_Renderer* renderer() const { return m_renderer; }
	inline SDL_GLContext gl() const { return m_gl; }

	void present();

private:
	SDL_Window *m_window;
	SDL_Renderer *m_renderer;
	SDL_GLContext m_gl;
};

#endif //_WINDOW_H_
