#ifndef  _RESOURCES_H_
#define  _RESOURCES_H_

#include <memory>

struct SDL_Texture;

namespace SDL
{
	typedef std::shared_ptr<SDL_Texture> Texture;
}

class ResourceManager
{
public:
	virtual ~ResourceManager();
	
	virtual SDL::Texture loadTexture(std::string const& _path) = 0;
};

#endif //_RESOURCES_H_
