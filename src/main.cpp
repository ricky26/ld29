#include <stdexcept>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "opengl.h"
#include "event_dispatch.h"
#include "game.h"

struct SDLState
{
	SDLState()
	{
		if(SDL_Init(SDL_INIT_EVERYTHING) == -1)
			throw std::runtime_error("failed to initialise SDL");

		if(IMG_Init(IMG_INIT_PNG) == -1)
			throw std::runtime_error("failed to initialise SDL_image");
	}
	

	~SDLState()
	{
		IMG_Quit();
		SDL_Quit();
	}
};

int main(int _argc, char *_argv[])
{
	try
	{
		bool quit = false;
		SDLState sdl;
		DispatchStack &dispatch = DispatchStack::get();
		Game game;
		Update update;

		glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		update.ticksLast = SDL_GetPerformanceCounter();
		update.tickFrequency = SDL_GetPerformanceFrequency(); // Should really refresh this.

		while(!quit)
		{
			SDL_Event event;
			while(SDL_PollEvent(&event))
			{
				if(!dispatch.handleEvent(event)
				   && (event.type == SDL_QUIT))
					quit = true;
			}

			update.ticksNow = SDL_GetPerformanceCounter();
			update.dt = float((update.ticksNow - update.ticksLast) / double(update.tickFrequency));
			dispatch.update(update);
			update.ticksLast = update.ticksNow;

			dispatch.handleIdle();
			SDL_Delay(0);
		}
	}
	catch(std::exception tExc)
	{
		std::cerr << "An exception occurred: " << tExc.what() << std::endl;
		return -1;
	}
	catch(...)
	{
		std::cerr << "An unknown exception occurred." << std::endl;
		return -2;
	}
}
