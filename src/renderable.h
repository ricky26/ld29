#ifndef  _RENDERABLE_H_
#define  _RENDERABLE_H_

#include "update.h"

class Renderable
{
public:
	virtual ~Renderable();

	virtual void render() {}
	virtual void update(Update& _update) {}

	virtual void renderLight() {}
};

#endif //_RENDERABLE_H_
