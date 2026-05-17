#pragma once
#include "pch.h"
#include "SDK.hpp"
#include "Signature.h"

namespace FNameHelper
{
	SDK::FName FNameFromString(const std::string str);
	SDK::FString FStringFromString(const std::string str);
	SDK::FText FTextFromString(const std::string str);
}

namespace TArrayHelper
{
	void ResizeTArray(void* TargetArray, int Size);
}

namespace TMapHelper
{
	template <typename K, typename V>
	struct MapAddParams
	{
		SDK::TMap<K, V> TargetMap;
		K Key;
		V Value;
	};

	template <typename K, typename V>
	void Map_Add(SDK::TMap<K, V> TargetMap, K Key, V Value)
	{
		static class SDK::UFunction* Func = nullptr;

		if (Func == nullptr)
			Func = SDK::UBlueprintMapLibrary::StaticClass()->GetFunction("BlueprintMapLibrary", "Map_Add");

		MapAddParams<K, V> Parms;

		Parms.TargetMap = std::move(TargetMap);
		Parms.Key = Key;
		Parms.Value = Value;

		auto Flgs = Func->FunctionFlags;
		Func->FunctionFlags |= 0x400;

		SDK::UBlueprintMapLibrary::GetDefaultObj()->ProcessEvent(Func, &Parms);

		Func->FunctionFlags = Flgs;
	}
}

SDK::UObject* FindObjectByClass(const std::string& Name, SDK::UClass* uclass);

