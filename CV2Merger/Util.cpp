#include "Util.h"
#include <thread>

//0x141401fe0 in version 1.0.3.0
void* Sig_TArray_ResizeTo = sigScan(
    "\x48\x89\x5C\x24\x2A\x57\x48\x83\xEC\x30\x48\x63\xDA\x48\x8B\xF9\x85\xD2\x74\x2A\x48\x8B\x0D\x2A\x2A\x2A\x2A\x4C\x8B\xCB\x4D\x03\xC9\x48\x85\xC9\x74\x2A\x48\x8B\x01\x41\xB8\x02\x00\x00\x00\x49\x8B\xD1\xFF\x50\x2A\x4C\x8B\xC8\x49\xD1\xE9\xB8\xFF\xFF\xFF\x7F\x41\x3B\xD9\x44\x0F\x4F\xC8\x41\x8B\xD9\x3B\x5F\x2A",
    "xxxx?xxxxxxxxxxxxxx?xxx????xxxxxxxxxx?xxxxxxxxxxxxxx?xxxxxxxxxxxxxxxxxxxxxxx?");

void* Sig_TArrayU8_ResizeGrow = sigScan(
	"\x48\x89\x5C\x24\x2A\x57\x48\x83\xEC\x30\x48\x63\x59\x2A\x48\x8B\xF9\x3B\xDA\x7C\x2A\x83\x7F\x2A\x00\xB9\x04\x00\x00\x00\x74\x2A\x48\x8D\x0C\x2A\x48\xC1\xE9\x03\x48\x83\xC1\x10\x48\x03\xCB\xEB\x2A\x48\x3B\xD9\x48\x0F\x47\xCB\x4C\x8B\x0D\x2A\x2A\x2A\x2A\x4D\x85\xC9\x74\x2A\x49\x8B\x01\x48\x8B\xD1\x41\xB8\x01\x00\x00\x00\x49\x8B\xC9\xFF\x50\x2A\x48\x8B\xC8\x3B\xD9\xC7\x44\x24\x2A\x01\x00\x00\x00\xB8\xFF\xFF\xFF\x7F\x41\xB9\x01\x00\x00\x00\x0F\x4F\xC8\x89\x4F\x2A\x44\x8B\xC1\x48\x8B\xCF\xE8\x2A\x2A\x2A\x2A\x48\x8B\x5C\x24\x2A\x48\x83\xC4\x30\x5F\xC3\x8B\xCB\xE8\x2A\x2A\x2A\x2A",
	"xxxx?xxxxxxxx?xxxxxx?xx?xxxxxxx?xxx?xxxxxxxxxxxx?xxxxxxxxxx????xxxx?xxxxxxxxxxxxxxxxx?xxxxxxxx?xxxxxxxxxxxxxxxxxxxx?xxxxxxx????xxxx?xxxxxxxxx????");

FUNCTION_PTR(void, __fastcall, TArray_ResizeTo, Sig_TArray_ResizeTo, void* TargetArray, int Size);
FUNCTION_PTR(void, __fastcall, TArrayU8_ResizeGrow, Sig_TArrayU8_ResizeGrow, void* TargetArray, int Size);

namespace FNameHelper
{
	SDK::FName FNameFromString(const std::string str)
	{
		std::wstring wstr = std::wstring(str.begin(), str.end());
		SDK::FName fname = SDK::UKismetStringLibrary::Conv_StringToName(SDK::FString(wstr.c_str()));
		return fname;
	}

	SDK::FString FStringFromString(const std::string str)
	{
		return SDK::UKismetStringLibrary::Conv_NameToString(FNameFromString(str));
	}
	SDK::FText FTextFromString(const std::string str)
	{
		return SDK::UKismetTextLibrary::Conv_NameToText(FNameFromString(str));
	}
}

namespace TArrayHelper
{
	void ResizeTArray(void* TargetArray, int Size)
	{
		TArray_ResizeTo(TargetArray, Size);
	}
	bool Contains(SDK::TArray<SDK::FName> TargetArray, std::string Entry)
	{
		for (auto& name : TargetArray)
		{
			if (name.ToString() == Entry)
				return true;
		}
		return false;
	}
	void ResizeGrowU8(void* TargetArray, int Size)
	{
		TArrayU8_ResizeGrow(TargetArray, Size);
	}
}

namespace WorldHelper
{
	static SDK::UWorld** GWorldPtr = nullptr;

	void Init(void* GWorldAddress)
	{
		GWorldPtr = static_cast<SDK::UWorld**>(GWorldAddress);
	}

	SDK::UWorld* GetWorld()
	{
		if (GWorldPtr)
			return *GWorldPtr;

		// Signature not found: fall back to the game viewport's world. UEngine::GetEngine() finds the
		// engine through GObjects, so this doesn't depend on any build-specific offset either.
		SDK::UEngine* Engine = SDK::UEngine::GetEngine();
		if (Engine && Engine->GameViewport)
			return Engine->GameViewport->World;

		return nullptr;
	}
}

SDK::UObject* FindObjectByClass(const std::string& Name, SDK::UClass* uclass)
{
	for (int i = 0; i < SDK::UObject::GObjects->Num(); ++i)
	{
		SDK::UObject* Object = SDK::UObject::GObjects->GetByIndex(i);

		if (!Object)
			continue;
		if (!Object->IsA(uclass))
			continue;

		if (Object->GetName() == Name)
			return Object;
	}

	return nullptr;
}