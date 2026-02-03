#pragma once
#include "pch.h"
#include "SDK.hpp"
#include "Signature.h"

namespace FNameHelper
{
	SDK::FName FNameFromString(const std::string str);
	SDK::FString FStringFromString(const std::string str);
}

namespace TArrayHelper
{
	void ResizeTArray(void* TargetArray, int Size);
}

SDK::UObject* FindObjectByClass(const std::string& Name, SDK::UClass* uclass);

