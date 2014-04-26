#include "window.h"
#include <stdexcept>

Window::Window()
	: m_window(nullptr)
	, m_renderer(nullptr)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	m_window = SDL_CreateWindow("LD29", 100, 100, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if(!m_window)
		throw std::runtime_error("failed to create window");

	m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(!m_renderer)
		throw std::runtime_error("failed to create renderer");

	m_gl = SDL_GL_CreateContext(m_window);
}

Window::~Window()
{
	if(m_renderer)
	{
		SDL_DestroyRenderer(m_renderer);
		m_renderer = nullptr;
	}

	if(m_window)
	{
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}
}

void Window::present()
{
	SDL_GL_SwapWindow(m_window);
}
