#include "fs.h"

namespace fs
{
	std::string lookup(std::string const& _path)
	{
		return "data/" + _path;
	}
}
