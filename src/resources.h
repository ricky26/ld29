#ifndef  _RESOURCES_H_
#define  _RESOURCES_H_

#include <memory>
#include "opengl.h"

struct SDL_Texture;
struct SDL_Surface;

namespace SDL
{
	typedef std::shared_ptr<SDL_Texture> Texture;
	typedef std::shared_ptr<SDL_Surface> Surface;
}

class ResourceManager
{
public:
	virtual ~ResourceManager();
	
	virtual SDL::Texture loadTexture(std::string const& _path) = 0;
	virtual GLuint loadGLTexture(std::string const& _path) = 0;
};

struct UVs
{
	union
	{
		struct
		{
			float u,v,s,t;
		};

		float data[4];
	};
};

namespace util
{
	void softenTexture(SDL::Texture &tex);
}

#endif //_RESOURCES_H_
