#include "Util.h"

//0x141401fe0 in version 1.0.3.0
void* Sig_TArray_ResizeTo = sigScan(
    "\x48\x89\x5C\x24\x2A\x57\x48\x83\xEC\x30\x48\x63\xDA\x48\x8B\xF9\x85\xD2\x74\x2A\x48\x8B\x0D\x2A\x2A\x2A\x2A\x4C\x8B\xCB\x4D\x03\xC9\x48\x85\xC9\x74\x2A\x48\x8B\x01\x41\xB8\x02\x00\x00\x00\x49\x8B\xD1\xFF\x50\x2A\x4C\x8B\xC8\x49\xD1\xE9\xB8\xFF\xFF\xFF\x7F\x41\x3B\xD9\x44\x0F\x4F\xC8\x41\x8B\xD9\x3B\x5F\x2A",
    "xxxx?xxxxxxxxxxxxxx?xxx????xxxxxxxxxx?xxxxxxxxxxxxxx?xxxxxxxxxxxxxxxxxxxxxxx?");

FUNCTION_PTR(void, __fastcall, TArray_ResizeTo, Sig_TArray_ResizeTo, void* TargetArray, int Size);

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
}

namespace TArrayHelper
{
	void ResizeTArray(void* TargetArray, int Size)
	{
		TArray_ResizeTo(TargetArray, Size);
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