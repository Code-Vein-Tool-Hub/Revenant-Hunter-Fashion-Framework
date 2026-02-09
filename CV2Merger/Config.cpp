#include "Config.h"
#include <filesystem>

bool Config::Console;
bool Config::AccessoryInfo;

bool Config::init()
{
	toml::table config;
	std::string fileString = "";

	if (!std::filesystem::exists(std::filesystem::current_path().generic_string() + "\\RHFFConfig.toml")) {
		printf("[CV2Merger] [Error] Could not find config path\n");
	}

	try
	{
		std::ifstream file(std::filesystem::current_path().generic_string() + "\\RHFFConfig.toml");
		std::string str;
		while (std::getline(file, str))
			fileString += str + "\n";
		config = toml::parse(fileString);
	}
	catch (std::exception& exception)
	{
		char text[1024];
		sprintf_s(text, "Failed to parse config.toml:\n%s", exception.what());
		MessageBoxA(nullptr, text, "RHFF", MB_OK | MB_ICONERROR);
	}

	Console = config["General"]["Enable_Console"].value_or(true);
	AccessoryInfo = config["General"]["Acc_Info_Tool"].value_or(false);

	return true;
}