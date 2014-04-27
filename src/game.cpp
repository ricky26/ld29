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
			if(!_fixture->TestPoint(position))
				return true;

			fixture = _fixture;
			return false;
		}

		b2Vec2 position;
		b2Fixture* fixture;
	};
}

namespace util
{
	void softenTexture(SDL::Texture &tex)
	{
		float w, h;
		SDL_GL_BindTexture(tex.get(), &w, &h);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		SDL_GL_UnbindTexture(tex.get());
	}
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
	m_factory.setResources(this);
	m_factory.setWorld(&m_world);
	m_factory.setGame(this);
	m_factory.init();

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

	const float width = 800;
	const float height = 2400;

	// Light
	{
		std::unique_ptr<Entities::Light> light(m_factory.createLight());
		light->setActive(true);
		//light->setPos(b2Vec2(width*0.5f, 150));
		light->init();
		light->body()->SetTransform(worldToPhysics(b2Vec2(70, 200)), 0);
		m_defaultLight = light.get();
		m_renderables.push_back(std::unique_ptr<Renderable>(light.release()));
	}

	// Left
	{
		std::unique_ptr<Entities::Box> testBox(m_factory.createBricks(b2Vec2(60, height), true));
		testBox->body()->SetTransform(worldToPhysics(b2Vec2(0, height*0.5f)), 0);

		m_ground = testBox.get()->body().body();
		m_renderables.push_back(std::unique_ptr<Renderable>(testBox.release()));
	}
	
	// Right
	{
		std::unique_ptr<Entities::Box> testBox(m_factory.createBricks(b2Vec2(60, height), true));
		testBox->body()->SetTransform(worldToPhysics(b2Vec2(width, height*0.5f)), 0);
		m_renderables.push_back(std::unique_ptr<Renderable>(testBox.release()));
	}

	// Bottom
	{
		std::unique_ptr<Entities::Box> testBox(m_factory.createBricks(b2Vec2(width, 60), true));
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
		std::unique_ptr<Entities::Box> testBox(m_factory.createHalfBricks(b2Vec2(200, 20)));
		testBox->body()->SetTransform(worldToPhysics(b2Vec2(-80, -40 - 40*i)), 0);

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

		b2Vec2 pos = worldToPhysics(b2Vec2(400 + xmod + xoff*55, row*-60));

		if(!(i % 36))
		{
			std::unique_ptr<Entities::Light> light(m_factory.createLight());
			light->setScale(0.3f);
			light->init();
			light->body()->SetTransform(pos, 0);
			m_renderables.push_back(std::unique_ptr<Renderable>(light.release()));
		}
		else if((rand() / float(RAND_MAX)) > 0.99f)
		{
			std::unique_ptr<Entities::Box> testBox(m_factory.createChest());
			testBox->body()->SetTransform(pos, 0);
			addRenderable(testBox.release());	
		}
		else
		{
			std::unique_ptr<Entities::Box> testBox(m_factory.createDirt());
			testBox->body()->SetTransform(pos, 0);
			addRenderable(testBox.release());
		}
	}

	updateCamera();

	for(int i = 0; i < 600; i++)
		m_world.Step(1/60.f, 6, 2);
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

	for(auto &item : renderables())
	{
		if(Entities::Box *box = dynamic_cast<Entities::Box*>(item.get()))
			box->setIlluminated(false);
	}

	for(std::unique_ptr<Renderable> &renderable: m_renderables)
		renderable->update(_update);
}

void Game::onIdle()
{
	glClearColor(0,0,0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	DispatchStack::get().render();

	m_window.present();
}

bool Game::onMouseMotion(const SDL_MouseMotionEvent &_event)
{
	b2Vec2 worldPos(_event.x, _event.y + m_cameraOffset);
	m_lastPos = worldPos;

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
			m_grab->SetTarget(worldToPhysics(worldPos));
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
	{
		if(light->isActive())
			return true;
	}

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
	picker.position = aabb.lowerBound = aabb.upperBound = worldToPhysics(worldPos);
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
	m_grabbedBody = body;
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

			if(!m_wasFixed)
				m_grabbedBody->SetFixedRotation(false);

			m_grab = nullptr;
			m_grabbed = nullptr;
			m_grabbedBody = nullptr;
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
	return true;
}

bool Game::onKeyDown(const SDL_KeyboardEvent &tEvent)
{
	switch(tEvent.keysym.sym)
	{
	case SDLK_q:
	{
		std::unique_ptr<Entities::Box> box(m_factory.createBox(b2Vec2(75, 45)));
		box->body()->SetTransform(worldToPhysics(m_lastPos), 0);
		box->body()->SetAwake(false);
		addRenderable(box.release());
		return true;
	}

	case SDLK_w:
	{
		std::unique_ptr<Entities::Light> light(m_factory.createLight());
		light->setScale(0.5f);
		light->init();
		light->body()->SetTransform(worldToPhysics(m_lastPos), 0);
		light->body()->SetAwake(false);
		addRenderable(light.release());
		return true;
	}

	case SDLK_e:
	{
		std::unique_ptr<Entities::Box> box(m_factory.createChest());
		box->body()->SetTransform(worldToPhysics(m_lastPos), 0);
		box->body()->SetAwake(false);
		addRenderable(box.release());
		return true;
	}

	default:
		return false;
	}
}

SDL::Texture Game::loadTexture(std::string const& _path)
{
	auto ret = SDL::Texture(IMG_LoadTexture(m_window.renderer(), fs::lookup(_path).c_str()),
							SDL_DestroyTexture);
	util::softenTexture(ret);
	return ret;
}

GLuint Game::loadGLTexture(std::string const& _path)
{	
	SDL::Surface surface(IMG_Load(fs::lookup(_path).c_str()),
						 SDL_FreeSurface);
	GLenum texFmt = 0;

	if(!surface.get())
		return 0;

	if(surface->format->BytesPerPixel == 4)
	{
		if(surface->format->Rmask == 0xff)
			texFmt = GL_RGBA;
		else
			texFmt = GL_BGRA;
	}
	else if(surface->format->BytesPerPixel == 3)
	{
		if(surface->format->Rmask == 0xff)
			texFmt = GL_RGB;
		else
			texFmt = GL_BGR;
	}

	if(!texFmt)
		return 0;

	GLuint ret = 0;
	glGenTextures(1, &ret);
	
    glBindTexture(GL_TEXTURE_2D, ret);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	SDL_LockSurface(surface.get());

	glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, texFmt, surface->w, surface->h, 0,
				 texFmt, GL_UNSIGNED_BYTE, surface->pixels);
	SDL_UnlockSurface(surface.get());

	glBindTexture(GL_TEXTURE_2D, 0);
	return ret;
}
