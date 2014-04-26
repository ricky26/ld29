#include "game.h"
#include "game_entity.h"
#include <iostream>
#include <SDL2/SDL_opengl.h>

Game::Game()
	: m_world(b2Vec2(0, 9.8f))
	, m_physTime(0)
	, m_cameraOffset(0)
{
	DispatchStack::get().push(*this);

	const float width = 800;
	const float height = 2400;

	// Left
	{
		std::unique_ptr<Entities::Box> testBox(new Entities::Box());
		testBox->create(m_world, b2Vec2(30, height), true);
		testBox->body()->SetTransform(b2Vec2(0, height*0.5f), 0);
		m_renderables.push_back(std::unique_ptr<Renderable>(testBox.release()));
	}
	
	// Right
	{
		std::unique_ptr<Entities::Box> testBox(new Entities::Box());
		testBox->create(m_world, b2Vec2(30, height), true);
		testBox->body()->SetTransform(b2Vec2(width, height*0.5f), 0);
		m_renderables.push_back(std::unique_ptr<Renderable>(testBox.release()));
	}

	// Bottom
	{
		std::unique_ptr<Entities::Box> testBox(new Entities::Box());
		testBox->create(m_world, b2Vec2(width, 30), true);
		testBox->body()->SetTransform(b2Vec2(width*0.5f, height), 0);
		m_renderables.push_back(std::unique_ptr<Renderable>(testBox.release()));
	}

	// Box
	for(int i = 0; i < 500; i++)
	{
		int row = i / 3;
		int col = i % 3;

		int xoff = col - 1;

		float xmod = (row & 1) ? 0 : 30;
		
		std::unique_ptr<Entities::Box> testBox(new Entities::Box());
		testBox->create(m_world, b2Vec2(50, 50));
		testBox->body()->SetTransform(b2Vec2(400 + xmod + xoff*55, row*-60), 0);
		m_renderables.push_back(std::unique_ptr<Renderable>(testBox.release()));
	}
}

Game::~Game()
{
	DispatchStack::get().pop(*this);
}

void Game::updateCamera()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 800, 600 + m_cameraOffset, m_cameraOffset, 0, 1);
}

void Game::onInit()
{
	updateCamera();
}

void Game::render()
{
	for(std::unique_ptr<Renderable> &renderable: m_renderables)
		renderable->render();
}

void Game::update(Update& _update)
{
	const float physTimeFrame = 1.f/60.f;

	m_physTime += _update.dt*10;
	while(m_physTime > physTimeFrame)
	{
		m_world.Step(physTimeFrame, 6, 2);
		m_physTime -= physTimeFrame;
	}

	for(std::unique_ptr<Renderable> &renderable: m_renderables)
		renderable->update(_update);
}

void Game::onIdle()
{
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	DispatchStack::get().render();

	m_window.present();
}

bool Game::onMouseWheel(const SDL_MouseWheelEvent &tEvent)
{
	m_cameraOffset += tEvent.y * 100;
	m_cameraOffset = std::max(-10000.f, std::min(2400-600.f, m_cameraOffset));
	updateCamera();
	return true;
}
