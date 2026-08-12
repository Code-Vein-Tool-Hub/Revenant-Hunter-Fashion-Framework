#pragma once
#include "pch.h"

class Config
{
public:
	static bool Console;
	static bool AccessoryInfo;
	static bool CompanionPakLoaded;

	static bool init();
	static bool postinit();
};

