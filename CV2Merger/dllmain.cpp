// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Signature.h"
#include "SDK.hpp"
#include "ModPatch.h"
#include "Util.h"
#include "Config.h"
#include <filesystem>

#pragma region Sigs

//0x143da5900 in version 1.0.3.0
void* Sig_UDataTable_AddRow = sigScan(
    "\x48\x89\x5C\x24\x2A\x48\x89\x6C\x24\x2A\x56\x57\x41\x56\x48\x83\xEC\x20\x48\x8B\xF9\x48\x89\x4C\x24\x2A",
    "xxxx?xxxx?xxxxxxxxxxxxxxx?");

void* Sig_UGameFlowManager_OnShaderCompileWaitFinished = sigScan(
    "\x40\x55\x41\x56\x48\x83\xEC\x58\x48\x8B\x15\x2A\x2A\x2A\x2A",
    "xxxxxxxxxxx????");

void* Sig_UDataTable_Serialize = sigScan(
    "\x48\x89\x5C\x24\x2A\x57\x48\x81\xEC\xC0\x01\x00\x00\x48\x8B\x05\x2A\x2A\x2A\x2A\x48\x33\xC4\x48\x89\x84\x24\x2A\x2A\x2A\x2A\x48\x8D\x05\x2A\x2A\x2A\x2A\x48\x89\x54\x24\x2A\x48\x89\x44\x24\x2A\x48\x8B\xFA",
    "xxxx?xxxxxxxxxxx????xxxxxxx????xxx????xxxx?xxxx?xxx");

void* Sig_ACharacterCustomizePawn_CameraResetPressed = sigScan(
    "\x40\x53\x48\x83\xEC\x30\x80\xB9\x2A\x2A\x2A\x2A\x00\x48\x89\xCB",
    "xxxxxxxx????xxxx");

#pragma endregion

#pragma region Functions

FUNCTION_PTR(void, __fastcall, UDataTable_AddRow, Sig_UDataTable_AddRow, SDK::UDataTable* _this, SDK::FName RowName, SDK::FTableRowBase* RowData);

void EnableConsole()
{
    SDK::UEngine* Engine = SDK::UEngine::GetEngine();
    SDK::UWorld* World = SDK::UWorld::GetWorld();

    SDK::UInputSettings::GetDefaultObj()->ConsoleKeys[0].KeyName = SDK::UKismetStringLibrary::Conv_StringToName(L"F2");
    SDK::UObject* NewObject = SDK::UGameplayStatics::SpawnObject(Engine->ConsoleClass, Engine->GameViewport);
    Engine->GameViewport->ViewportConsole = static_cast<SDK::UConsole*>(NewObject);
}

void Patch_Inner_FlagCondition(SDK::UDataTable* TablePtr, bool male = false)
{
    for (auto& entry : TablePtr->RowMap)
    {
        if (male)
        {
            if (ModPatch::DT_Inner_Male.contains(entry.First.ToString()))
            {
                //printf("[CV2Merger] Row Location %p\n", entry.Second);
                WRITE_MEMORY(entry.Second + 0x58, uint8_t, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0)
            }
        }
        else
        {
            if (ModPatch::DT_Inner_Female.contains(entry.First.ToString()))
            {
                //printf("[CV2Merger] Row Location %p\n", entry.Second);
                WRITE_MEMORY(entry.Second + 0x58, uint8_t, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0)
            }
        }
    }
}

template <typename T>
void Patch_FlagCondition(SDK::UDataTable* TablePtr, std::map<std::string, T> ModTable, uint64_t offset)
{
    for (auto& entry : TablePtr->RowMap)
    {
        if (ModTable.contains(entry.First.ToString()))
        {
            //printf("[CV2Merger] Row Location %p\n", entry.Second);
            WRITE_MEMORY(entry.Second + offset, uint8_t, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0)
        }
    }
}

#pragma endregion

#pragma region Hooks

HOOK(void, __stdcall, Hook_UGameFlowManager_OnShaderCompileWaitFinished, Sig_UGameFlowManager_OnShaderCompileWaitFinished, uint64_t* param1)
{
    ModPatch::init();
    orig_Hook_UGameFlowManager_OnShaderCompileWaitFinished(param1);
}

HOOK(void, __stdcall, Hook_UDataTable_Serialize, Sig_UDataTable_Serialize, SDK::UDataTable* _this, uint64_t* param2)
{
    //printf("[CV2Merger] UDataTable::Serialize called for DataTable: %s\n", _this->GetName().c_str());
    orig_Hook_UDataTable_Serialize(_this, param2);
    std::string TableName = _this->GetName();

    if (TableName == "DT_Inner_Female")
    {
        if (ModPatch::DT_Inner_Female.size() > 0) {
            //printf("[CV2Merger] DataTableName: %s\n", _this->GetFullName().c_str());
            for (auto& entry : ModPatch::DT_Inner_Female)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Inner_Female, 0x58);
        }
    }
    if (TableName == "DT_Inner_Male")
    {
        if (ModPatch::DT_Inner_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Inner_Male)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Inner_Male, 0x58);
        }
    }

    if (TableName == "DT_HairBaseList")
    {
        if (ModPatch::DT_HairBaseList.size() > 0) {
            for (auto& entry : ModPatch::DT_HairBaseList)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
		}
    }
    if (TableName == "DT_HairBack")
    {
        if (ModPatch::DT_HairBack.size() > 0) {
            for (auto& entry : ModPatch::DT_HairBack)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    if (TableName == "DT_HairFront")
    {
        if (ModPatch::DT_HairFront.size() > 0) {
            for (auto& entry : ModPatch::DT_HairFront)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
	}
    if (TableName == "DT_HairFrontSide")
    {
        if (ModPatch::DT_HairFrontSide.size() > 0) {
            for (auto& entry : ModPatch::DT_HairFrontSide)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
		}
    }
    if (TableName == "DT_HairOther")
    {
        if (ModPatch::DT_HairOther.size() > 0) {
            for (auto& entry : ModPatch::DT_HairOther)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    if (TableName == "DT_HairSide")
    {
        if (ModPatch::DT_HairSide.size() > 0) {
            for (auto& entry : ModPatch::DT_HairSide)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    if (TableName == "DT_HairTop")
    {
        if (ModPatch::DT_HairTop.size() > 0) {
            for (auto& entry : ModPatch::DT_HairTop)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    if (TableName == "DT_HairSetList_Female")
    {
        if (ModPatch::DT_HairSetList_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_HairSetList_Female)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    if (TableName == "DT_HairSetList_Male")
    {
        if (ModPatch::DT_HairSetList_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_HairSetList_Male)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }

    if (TableName == "DT_AccessoryPresetDLC_Free")
    {
        if (ModPatch::DT_AccessoryPresetDLC_Free.size() > 0) {
            for (auto& entry : ModPatch::DT_AccessoryPresetDLC_Free)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
			Patch_FlagCondition(_this, ModPatch::DT_AccessoryPresetDLC_Free, 0x01B0);
        }
    }

    if (TableName == "DT_Boots_Female")
    {
        if (ModPatch::DT_Boots_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_Boots_Female)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Boots_Female, 0x058);
        }
    }
    if (TableName == "DT_Boots_Male")
    {
        if (ModPatch::DT_Boots_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Boots_Male)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Boots_Male, 0x058);
        }
    }
    if (TableName == "DT_Gloves_Female")
    {
        if (ModPatch::DT_Gloves_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_Gloves_Female)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Gloves_Female, 0x058);
        }
    }
    if (TableName == "DT_Gloves_Male")
    {
        if (ModPatch::DT_Gloves_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Gloves_Male)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Gloves_Male, 0x058);
        }
    }
    if (TableName == "DT_Mask_Female")
    {
        if (ModPatch::DT_Mask_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_Mask_Female)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Mask_Female, 0x058);
        }
    }
    if (TableName == "DT_Mask_Male")
    {
        if (ModPatch::DT_Mask_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Mask_Male)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Mask_Male, 0x058);
        }
    }
    if (TableName == "DT_Outer_Female")
    {
        if (ModPatch::DT_Outer_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_Outer_Female)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Outer_Female, 0x058);
        }
    }
    if (TableName == "DT_Outer_Male")
    {
        if (ModPatch::DT_Outer_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Outer_Male)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[CV2Merger] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Outer_Male, 0x058);
        }
    }
    return;
}

HOOK(void, __stdcall, Hook_CameraResetPressed, Sig_ACharacterCustomizePawn_CameraResetPressed, SDK::ACharacterCustomizePawn* _this)
{
    printf("[CV2Merger] Camera rest from %s\n", _this->GetName().c_str());

    SDK::FName name;
    SDK::FEulerAngleTransform LocalRoot;
    SDK::FEulerAngleTransform LocalOrient;
    SDK::FEulerAngleTransform LocalMesh;
    _this->GetAccessoryTransforms(&LocalRoot, &LocalOrient, &LocalMesh);

    printf("[CV2Merger] GetAccessoryTransforms:\n");
    printf("    LocalRoot\n        Translation: [%f, %f, %f]:\n", LocalRoot.Translation.X, LocalRoot.Translation.Y, LocalRoot.Translation.Z);
    printf("        Rotation: [%f, %f, %f]:\n", LocalRoot.Rotation.X, LocalRoot.Rotation.Y, LocalRoot.Rotation.Z);
    printf("        Scale: [%f, %f, %f]:\n", LocalRoot.Scale.X, LocalRoot.Scale.Y, LocalRoot.Scale.Z);

    printf("    LocalOrient\n        Translation: [%f, %f, %f]:\n", LocalOrient.Translation.X, LocalOrient.Translation.Y, LocalOrient.Translation.Z);
    printf("        Rotation: [%f, %f, %f]:\n", LocalOrient.Rotation.X, LocalOrient.Rotation.Y, LocalOrient.Rotation.Z);
    printf("        Scale: [%f, %f, %f]:\n", LocalOrient.Scale.X, LocalOrient.Scale.Y, LocalOrient.Scale.Z);

    printf("    LocalMesh\n        Translation: [%f, %f, %f]:\n", LocalMesh.Translation.X, LocalMesh.Translation.Y, LocalMesh.Translation.Z);
    printf("        Rotation: [%f, %f, %f]:\n", LocalMesh.Rotation.X, LocalMesh.Rotation.Y, LocalMesh.Rotation.Z);
    printf("        Scale: [%f, %f, %f]:\n", LocalMesh.Scale.X, LocalMesh.Scale.Y, LocalMesh.Scale.Z);

    orig_Hook_CameraResetPressed(_this);
}

#pragma endregion

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Config::init();

        if (!GetConsoleWindow() && Config::Console) {
            AllocConsole();
            AttachConsole(GetCurrentProcessId());
            freopen("CON", "w", stdout);
        }

		INSTALL_HOOK(Hook_UGameFlowManager_OnShaderCompileWaitFinished);
        INSTALL_HOOK(Hook_UDataTable_Serialize);
        if (Config::AccessoryInfo)
        {
            INSTALL_HOOK(Hook_CameraResetPressed);
        }
		return TRUE;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

