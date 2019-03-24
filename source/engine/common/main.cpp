#include "stdafx.h"
#include "config.h"
#include "InnoApplication.h"

int main(int argc, char *argv[])
{
	if (!InnoApplication::setup(nullptr, argv[1]))
	{
		return 0;
	}
	if (!InnoApplication::initialize())
	{
		return 0;
	}
	while (InnoApplication::getStatus() == ObjectStatus::ALIVE)
	{
		if (!InnoApplication::update())
		{
			InnoApplication::terminate();
			return 0;
		}
	}

	return 0;
}