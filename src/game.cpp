#include "game.h"
#include "game_entity.h"
#include "light.h"
#include "opengl.h"
#include "fs.h"
#include <SDL2/SDL_image.h>
#include <iostream>

namespace
{
	struct PickObject: public b2QueryCallback
	{
		inline PickObject() : fixture(nullptr) {}

		virtual bool ReportFixture(b2Fixture *_fixture)
		{
			fixture = _fixture;
			return false;
		}

		b2Fixture* fixture;
	};
}

// ResourceManager

ResourceManager::~ResourceManager() {}

// Game

Game::Game()
	: m_world(b2Vec2(0, 9.8f))
	, m_physTime(0)
	, m_cameraOffset(0)
	, m_grab(nullptr)
	, m_grabbed(nullptr)
	, m_isRotating(false)
{
	DispatchStack::get().push(*this);

	const float width = 800;
	const float height = 2400;

	// Light
	{
		std::unique_ptr<Entities::Light> light(new Entities::Light());
		light->setGame(this);
		light->setWorld(&m_world);
		//light->setPos(b2Vec2(width*0.5f, 150));
		light->init();
		light->body()->SetTransform(worldToPhysics(b2Vec2(70, 200)), 0);
		m_defaultLight = light.get();
		m_renderables.push_back(std::unique_ptr<Renderable>(light.release()));
	}

	// Left
	{
		std::unique_ptr<Entities::Box> testBox(new Entities::Box());
		testBox->create(m_world, b2Vec2(30, height), true);
		testBox->body()->SetTransform(worldToPhysics(b2Vec2(0, height*0.5f)), 0);

		m_ground = testBox.get()->body().body();
		m_renderables.push_back(std::unique_ptr<Renderable>(testBox.release()));
	}
	
	// Right
	{
		std::unique_ptr<Entities::Box> testBox(new Entities::Box());
		testBox->create(m_world, b2Vec2(30, height), true);
		testBox->body()->SetTransform(worldToPhysics(b2Vec2(width, height*0.5f)), 0);
		m_renderables.push_back(std::unique_ptr<Renderable>(testBox.release()));
	}

	// Bottom
	{
		std::unique_ptr<Entities::Box> testBox(new Entities::Box());
		testBox->create(m_world, b2Vec2(width, 30), true);
		testBox->body()->SetTransform(worldToPhysics(b2Vec2(width*0.5f, height)), 0);
		m_renderables.push_back(std::unique_ptr<Renderable>(testBox.release()));
	}	

	// Light joint
	{
		b2DistanceJointDef jointDef;
		jointDef.Initialize(m_ground, m_defaultLight->body().body(),
							b2Vec2(0, m_defaultLight->body()->GetPosition().y),
							m_defaultLight->body()->GetPosition());
		jointDef.collideConnected = true;
		m_world.CreateJoint(&jointDef);
	}

	// Supports
	for(int i = 0; i < 20; i++)
	{
		std::unique_ptr<Entities::Box> testBox(new Entities::Box());
		testBox->create(m_world, b2Vec2(200, 10));
		testBox->body()->SetTransform(worldToPhysics(b2Vec2(-80, -20 - 20*i)), 0);

		b2MouseJointDef jointDef;
		jointDef.maxForce = 100;
		jointDef.collideConnected = true;
		jointDef.bodyA = m_ground;
		jointDef.bodyB = testBox->body().body();
		jointDef.target = testBox->body()->GetPosition();
		testBox->setDragJoint(m_world.CreateJoint(&jointDef));

		testBox->body()->SetFixedRotation(true);

		m_renderables.push_back(std::unique_ptr<Renderable>(testBox.release()));
	}

	// Boxes
	for(int i = 0; i < 500; i++)
	{
		int row = i / 3;
		int col = i % 3;

		int xoff = col - 1;

		float xmod = (row & 1) ? 0 : 30;
		
		std::unique_ptr<Entities::Box> testBox(new Entities::Box());
		testBox->create(m_world, b2Vec2(50, 50));
		testBox->body()->SetTransform(worldToPhysics(b2Vec2(400 + xmod + xoff*55, row*-60)), 0);
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
	glOrtho(0, 800, 600, 0, 0, 1);
}

void Game::onInit()
{
	m_background = loadTexture("textures/background.png");

	glGenFramebuffers(1, &m_lightFB);
	glGenTextures(1, &m_lightTexture);

	// Setup light texture

	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFB);
	glBindTexture(GL_TEXTURE_2D, m_lightTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_lightTexture, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	updateCamera();
}

void Game::onShutdown()
{
	glDeleteTextures(1, &m_lightTexture);
	glDeleteFramebuffers(1, &m_lightFB);
}

void Game::render()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glScalef(0.5f, 0.5f, 1);
	glTranslatef(0, -m_cameraOffset, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFB);
	glClearColor(0.03f,0.03f,0.05f, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	for(std::unique_ptr<Renderable> &renderable: m_renderables)
		renderable->renderLight();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glLoadIdentity();
	glTranslatef(0, -m_cameraOffset, 0);

	// background
	{
		float fW, fH;
		SDL_GL_BindTexture(m_background.get(), &fW, &fH);
		glBegin(GL_QUADS);	
		glColor3f(1,1,1);
		glTexCoord2f(0, fH);
		glVertex2f(0, 2400);
		glTexCoord2f(fW, fH);
		glVertex2f(800, 2400);
		glTexCoord2f(fW, 0);
		glVertex2f(800, -2400);
		glTexCoord2f(0, 0);
		glVertex2f(0, -2400);
		glEnd();
		SDL_GL_UnbindTexture(m_background.get());
	}

	for(std::unique_ptr<Renderable> &renderable: m_renderables)
		renderable->render();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

	glBindTexture(GL_TEXTURE_2D, m_lightTexture);
	glBegin(GL_QUADS);
	glColor4f(1,1,1,1);
	glTexCoord2f(0, 1);
	glVertex2f(0, 0);
	glTexCoord2f(1, 1);
	glVertex2f(800, 0);
	glTexCoord2f(1, 0);
	glVertex2f(800, 600);
	glTexCoord2f(0, 0);
	glVertex2f(0, 600);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Game::update(Update& _update)
{
	const float physTimeFrame = 1.f/60.f;

	m_physTime += _update.dt;
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
	glClearColor(1, 1, 0.7f, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	DispatchStack::get().render();

	m_window.present();
}

bool Game::onMouseMotion(const SDL_MouseMotionEvent &_event)
{
	if(m_grab)
	{
		if(m_isRotating)
		{
			if(Entities::Box *box = dynamic_cast<Entities::Box*>(m_grabbed))
			{
				const float rotAmt = (_event.xrel / float(800)) * M_PI * 2;
				b2Body *body = box->body().body();
				body->SetTransform(body->GetPosition(), body->GetAngle() + rotAmt);
			}
		}
		else
			m_grab->SetTarget(worldToPhysics(b2Vec2(_event.x, _event.y + m_cameraOffset)));
	}

	return false;
}

static bool canDrag(Renderable *renderable)
{
	if(Entities::Box *box = dynamic_cast<Entities::Box*>(renderable))
	{
		if(box->isIlluminated())
			return true;
	}

	if(Entities::Light *light = dynamic_cast<Entities::Light*>(renderable))
		return true;

	return false;
}

bool Game::onMouseDown(const SDL_MouseButtonEvent &_event)
{
	if(m_grab && (_event.button != 1))
	{
		m_isRotating = true;
		return true;
	}

	if(m_grab || (_event.button != 1))
		return false;
	
	b2Vec2 worldPos = b2Vec2(_event.x, _event.y + m_cameraOffset);

	PickObject picker;
	b2AABB aabb;
	aabb.lowerBound = aabb.upperBound = worldToPhysics(worldPos);
	m_world.QueryAABB(&picker, aabb);

	if(!picker.fixture)
		return false;

	b2Body *body = picker.fixture->GetBody();
	if(body->GetType() != b2_dynamicBody)
		return false;

	Renderable *renderable = static_cast<Renderable*>(body->GetUserData());
	if(!canDrag(renderable))
		return false;

	Entities::Box *box = dynamic_cast<Entities::Box*>(renderable);

	if(box && box->dragJoint())
	{
		m_grab = static_cast<b2MouseJoint*>(box->dragJoint());
	}
	else
	{
		b2MouseJointDef jointDef;
		jointDef.maxForce = 100;
		jointDef.collideConnected = true;
		jointDef.bodyB = body;
		jointDef.bodyA = m_ground;

		jointDef.target = aabb.lowerBound;

		m_grab = static_cast<b2MouseJoint*>(m_world.CreateJoint(&jointDef));
	}

	m_wasFixed = body->IsFixedRotation();
	m_grabbed = renderable;
	body->SetFixedRotation(true);
	return true;
}

bool Game::onMouseUp(const SDL_MouseButtonEvent &_event)
{
	if(m_grab)
	{
		if(_event.button != 1)
			m_isRotating = false;
		else
		{
			Entities::Box *box = dynamic_cast<Entities::Box*>(m_grabbed);
		
			if(!box || (box->dragJoint() != m_grab))
				m_world.DestroyJoint(m_grab);

			if(!m_wasFixed && box)
				box->body()->SetFixedRotation(false);

			m_grab = nullptr;
			m_grabbed = nullptr;
		}
		return true;
	}

	return false;
}

bool Game::onMouseWheel(const SDL_MouseWheelEvent &_event)
{
	m_cameraOffset -= _event.y * 100;
	m_cameraOffset = std::max(-10000.f, std::min(2400-600.f, m_cameraOffset));
	updateCamera();

	//b2Vec2 lightPos = m_defaultLight->position();
	//lightPos.y = m_cameraOffset + 150;
	//m_defaultLight->setPos(lightPos);
	return true;
}


SDL::Texture Game::loadTexture(std::string const& _path)
{
	return SDL::Texture(IMG_LoadTexture(m_window.renderer(), fs::lookup(_path).c_str()),
						SDL_DestroyTexture);
}
