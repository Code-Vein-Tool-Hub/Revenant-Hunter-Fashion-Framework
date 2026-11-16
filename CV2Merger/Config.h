#pragma once
#include "pch.h"

class Config
{
public:
	static bool Console;
	static bool AccessoryInfo;
	static std::wstring CompanionPakName;
	static bool CompanionPakLoaded;
	static std::wstring PresetModName;
	static bool PresetModLoaded;
	static bool FixSpecialRoughness;

	static bool init();
	static bool postinit();
};

