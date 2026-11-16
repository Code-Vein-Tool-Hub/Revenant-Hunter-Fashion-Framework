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
	bool Contains(SDK::TArray<SDK::FName> TargetArray, std::string Entry);

	template<typename T>
	bool Contains(SDK::TArray<T> TargetArray, T Entry)
	{
		for (T item : TargetArray)
		{
			if (item == Entry)
				return true;
		}
		return false;
	}


}

namespace UDataTableHelper
{
	template<typename T>
	T* GetDataRowByName(SDK::UDataTable* Table, std::string Name)
	{
		for (auto& row : Table->RowMap)
		{
			if (row.First.ToString() == Name)
			{
				return (T*)row.Second;
			}
		}
		return nullptr;
	}
}

namespace WorldHelper
{
	// Address of the engine's GWorld global, found by signature at startup (nullptr if the scan failed).
	void Init(void* GWorldAddress);

	// Current world. Use this instead of SDK::UWorld::GetWorld(), which reads a build-specific offset.
	SDK::UWorld* GetWorld();
}

SDK::UObject* FindObjectByClass(const std::string& Name, SDK::UClass* uclass);
