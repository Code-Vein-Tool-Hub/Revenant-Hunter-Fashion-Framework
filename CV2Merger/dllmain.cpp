// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Signature.h"
#include "SDK.hpp"
#include "ModPatch.h"
#include "Util.h"
#include "Config.h"
#include "SDK/BPFL_MapHelper_classes.hpp"
#include <filesystem>
#include <thread>

static bool TitleLoad = false;
static std::vector<std::string> MeshSlots = { "InnerMesh", "GlovesMesh", "BootsMesh", "MaskMesh", "OuterMesh" };

#pragma region Sigs

//0x143da5900 in version 1.0.3.0
//0x143e76120 in version 2.0.3.0
void* Sig_UDataTable_AddRow = sigScan(
    "\x48\x89\x5C\x24\x2A\x48\x89\x6C\x24\x2A\x56\x57\x41\x56\x48\x83\xEC\x20\x48\x8B\xF9\x48\x89\x4C\x24\x2A",
    "xxxx?xxxx?xxxxxxxxxxxxxxx?");

//0x1461a0370 in version 2.0.3.0
void* Sig_UGameFlowManager_OnShaderCompileWaitFinished = sigScan(
    "\x40\x55\x41\x56\x48\x83\xEC\x58\x48\x8B\x15\x2A\x2A\x2A\x2A",
    "xxxxxxxxxxx????");

//0x143da4ff0 in version 2.0.3.0
void* Sig_UDataTable_Serialize = sigScan(
    "\x48\x89\x5C\x24\x2A\x57\x48\x81\xEC\xC0\x01\x00\x00\x48\x8B\x05\x2A\x2A\x2A\x2A\x48\x33\xC4\x48\x89\x84\x24\x2A\x2A\x2A\x2A\x48\x8D\x05\x2A\x2A\x2A\x2A\x48\x89\x54\x24\x2A\x48\x89\x44\x24\x2A\x48\x8B\xFA",
    "xxxx?xxxxxxxxxxx????xxxxxxx????xxx????xxxx?xxxx?xxx");

//0x146151e90 in version 2.0.3.0
void* Sig_ACharacterCustomizePawn_CameraResetPressed = sigScan(
    "\x40\x53\x48\x83\xEC\x30\x80\xB9\x2A\x2A\x2A\x2A\x00\x48\x8B\xD9\x74\x2A\x48\x8B\x81\x2A\x2A\x2A\x2A\x48\x85\xC0",
    "xxxxxxxx????xxxxx?xxx????xxx");

//0x141876fa0 in version 2.0.3.0
void* Sig_UObject_ProcessEvent = sigScan(
    "\x40\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\x10\x01\x00\x00",
    "xxxxxxxxxxxxxxxxxxx");

//0x143f7f640 in version 2.0.3.0
void* Sig_InputHook = sigScan(
    "\x48\x89\x5C\x24\x2A\x48\x89\x74\x24\x2A\x57\x48\x83\xEC\x20\x48\x8B\x02\x41\x8B\xF8",
    "xxxx?xxxx?xxxxxxxxxxx");

//0x14615e7b0 in version 2.0.3.0
void* Sig_UCharacterCustomizeUtility_SetScalarParameterValueToAllMesh = sigScan(
    "\x48\x85\xC9\x0F\x84\x2A\x2A\x2A\x2A\x48\x89\x5C\x24\x2A\x57\x48\x83\xEC\x40\x8B\x41\x2A",
    "xxxxx????xxxx?xxxxxxx?");

//0x146115bf0 in version 2.0.3.0
void* Sig_ACharacterCustomizeSystem_SetMaskMesh = sigScan(
    "\x48\x89\x5C\x24\x2A\x57\x48\x81\xEC\xA0\x01\x00\x00\x48\x8B\x05\x2A\x2A\x2A\x2A\x48\x33\xC4\x48\x89\x84\x24\x2A\x2A\x2A\x2A\x48\x8B\x02\x48\x8B\xFA\x48\x89\x81\x2A\x2A\x2A\x2A\x48\x8B\xD9\xE8\x2A\x2A\x2A\x2A\x48\x8B\xCB\xE8\x2A\x2A\x2A\x2A\x48\x85\xC0\x0F\x84\x2A\x2A\x2A\x2A\x8B\x48\x2A\xC1\xE9\x1E\xF6\xD1\xF6\xC1\x01\x0F\x84\x2A\x2A\x2A\x2A\x4C\x8B\x0F\x48\x8D\x54\x24\x2A\x44\x0F\xB6\x83\x2A\x2A\x2A\x2A\x48\x8B\xC8\xE8\x2A\x2A\x2A\x2A\x45\x33\xC9",
    "xxxx?xxxxxxxxxxx????xxxxxxx????xxxxxxxxx????xxxx????xxxx????xxxxx????xx?xxxxxxxxxx????xxxxxxx?xxxx????xxxx????xxx");

#pragma endregion

#pragma region Functions

FUNCTION_PTR(void, __fastcall, UDataTable_AddRow, Sig_UDataTable_AddRow, SDK::UDataTable* _this, SDK::FName RowName, SDK::FTableRowBase* RowData);

void EnableConsole()
{
    SDK::UEngine* Engine = SDK::UEngine::GetEngine();
    SDK::UWorld* World = SDK::UWorld::GetWorld();

    SDK::UInputSettings::GetDefaultObj()->ConsoleKeys[0].KeyName = SDK::UKismetStringLibrary::Conv_StringToName(L"Tilde");
    SDK::UObject* NewObject = SDK::UGameplayStatics::SpawnObject(Engine->ConsoleClass, Engine->GameViewport);
    Engine->GameViewport->ViewportConsole = static_cast<SDK::UConsole*>(NewObject);

    SDK::APlayerController* MyController = World->OwningGameInstance->LocalPlayers[0]->PlayerController;
    //MyController->CheatClass = SDK::UAppCheatManager::StaticClass();
    //MyController->CheatManager = static_cast<SDK::UAppCheatManager*>(SDK::UGameplayStatics::SpawnObject(MyController->CheatClass, MyController));
    MyController->EnableCheats();

}

template <typename T>
void Patch_FlagCondition(SDK::UDataTable* TablePtr, std::map<std::string, T> ModTable, uint64_t offset)
{
    for (auto& entry : TablePtr->RowMap)
    {
        if (ModTable.contains(entry.First.ToString()))
        {
            //printf("[RHFF] Row Location %p\n", entry.Second);
            WRITE_MEMORY(entry.Second + offset, uint8_t, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0)
        }
    }
}

#pragma endregion

#pragma region Hooks

HOOK(void, __stdcall, Hook_UDataTable_Serialize, Sig_UDataTable_Serialize, SDK::UDataTable* _this, uint64_t* param2)
{
    //printf("[RHFF] UDataTable::Serialize called for DataTable: %s\n", _this->GetName().c_str());
    orig_Hook_UDataTable_Serialize(_this, param2);
    std::string TableName = _this->GetName();
    
    if (TableName == "DT_Inner_Female")
    {
        if (ModPatch::DT_Inner_Female.size() > 0) {
            //printf("[RHFF] DataTableName: %s\n", _this->GetFullName().c_str());
            for (auto& entry : ModPatch::DT_Inner_Female)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Inner_Female, 0x58);
        }
    }
    else if (TableName == "DT_Inner_Male")
    {
        if (ModPatch::DT_Inner_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Inner_Male)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Inner_Male, 0x58);
        }
    }

    else if (TableName == "DT_HairBaseList")
    {
        if (ModPatch::DT_HairBaseList.size() > 0) {
            for (auto& entry : ModPatch::DT_HairBaseList)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
		}

        //Addon table is for merging new parts options into an existing hair base
        if (Config::CompanionPakLoaded)
        {
            if (ModPatch::DT_HairBaseListAddon.size() > 0)
            {
                for (auto& entry : ModPatch::DT_HairBaseListAddon)
                {
                    printf("[RHFF] [DT Merger] Processing HairBaseList addon \"%s\"... \n", entry.first.c_str());

                    for (std::string table : entry.second.first)
                    {
                        if (table == "AddToAll")
                        {
                            printf("[RHFF] [DT Merger] Adding hair parts to all entries.\n");
                            for (auto& rowdata : _this->RowMap)
                            {
                                auto row = (SDK::FCharacterCustomizeDataTableHairBase*)rowdata.Second;

                                for (auto& part : entry.second.second.AvailableHairFrontList)
                                {
                                    if (TArrayHelper::Contains(row->AvailableHairFrontList, part))
                                        continue;
                                    if (row->AvailableHairFrontList.Num() <= 0)
                                        SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontList, FNameHelper::FNameFromString("Front_None"), SDK::UWorld::GetWorld());
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontList, part, SDK::UWorld::GetWorld());
                                }

                                for (auto& part : entry.second.second.AvailableHairFrontSideList)
                                {
                                    if (TArrayHelper::Contains(row->AvailableHairFrontSideList, part))
                                        continue;
                                    if (row->AvailableHairFrontSideList.Num() <= 0)
                                        SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontSideList, FNameHelper::FNameFromString("FrontSide_None"), SDK::UWorld::GetWorld());
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontSideList, part, SDK::UWorld::GetWorld());
                                }

                                for (auto& part : entry.second.second.AvailableHairSideList)
                                {
                                    if (TArrayHelper::Contains(row->AvailableHairSideList, part))
                                        continue;
                                    if (row->AvailableHairSideList.Num() <= 0)
                                        SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairSideList, FNameHelper::FNameFromString("Side_None"), SDK::UWorld::GetWorld());
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairSideList, part, SDK::UWorld::GetWorld());
                                }

                                for (auto& part : entry.second.second.AvailableHairTopList)
                                {
                                    if (TArrayHelper::Contains(row->AvailableHairTopList, part))
                                        continue;
                                    if (row->AvailableHairTopList.Num() <= 0)
                                        SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairTopList, FNameHelper::FNameFromString("Top_None"), SDK::UWorld::GetWorld());
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairTopList, part, SDK::UWorld::GetWorld());
                                }

                                for (auto& part : entry.second.second.AvailableHairBackList)
                                {
                                    if (TArrayHelper::Contains(row->AvailableHairBackList, part))
                                        continue;
                                    if (row->AvailableHairBackList.Num() <= 0)
                                        SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairBackList, FNameHelper::FNameFromString("Back_None"), SDK::UWorld::GetWorld());
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairBackList, part, SDK::UWorld::GetWorld());
                                }

                                for (auto& part : entry.second.second.AvailableHairOtherList)
                                {
                                    if (TArrayHelper::Contains(row->AvailableHairOtherList, part))
                                        continue;
                                    if (row->AvailableHairOtherList.Num() <= 0)
                                        SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairOtherList, FNameHelper::FNameFromString("Other_None"), SDK::UWorld::GetWorld());
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairOtherList, part, SDK::UWorld::GetWorld());
                                }
                            }
                            break;
                        }
                        else
                        {
                            printf("[RHFF] [DT Merger] Adding hair parts to %s\n", table.c_str());
                            SDK::FCharacterCustomizeDataTableHairBase* row = UDataTableHelper::GetDataRowByName<SDK::FCharacterCustomizeDataTableHairBase>(_this, table);
                            if (!row)
                            {
                                printf("[RHFF] [DT Merger] Unable to find BaseHair entry \"%s\"\n", table.c_str());
                                continue;
                            }

                            for (auto& part : entry.second.second.AvailableHairFrontList)
                            {
                                if (row->AvailableHairFrontList.Num() <= 0)
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontList, FNameHelper::FNameFromString("Front_None"), SDK::UWorld::GetWorld());
                                SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontList, part, SDK::UWorld::GetWorld());
                            }

                            for (auto& part : entry.second.second.AvailableHairFrontSideList)
                            {
                                if (row->AvailableHairFrontSideList.Num() <= 0)
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontSideList, FNameHelper::FNameFromString("FrontSide_None"), SDK::UWorld::GetWorld());
                                SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontSideList, part, SDK::UWorld::GetWorld());
                            }

                            for (auto& part : entry.second.second.AvailableHairSideList)
                            {
                                if (row->AvailableHairSideList.Num() <= 0)
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairSideList, FNameHelper::FNameFromString("Side_None"), SDK::UWorld::GetWorld());
                                SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairSideList, part, SDK::UWorld::GetWorld());
                            }

                            for (auto& part : entry.second.second.AvailableHairTopList)
                            {
                                if (row->AvailableHairTopList.Num() <= 0)
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairTopList, FNameHelper::FNameFromString("Top_None"), SDK::UWorld::GetWorld());
                                SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairTopList, part, SDK::UWorld::GetWorld());
                            }

                            for (auto& part : entry.second.second.AvailableHairBackList)
                            {
                                if (row->AvailableHairBackList.Num() <= 0)
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairBackList, FNameHelper::FNameFromString("Back_None"), SDK::UWorld::GetWorld());
                                SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairBackList, part, SDK::UWorld::GetWorld());
                            }

                            for (auto& part : entry.second.second.AvailableHairOtherList)
                            {
                                if (row->AvailableHairOtherList.Num() <= 0)
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairOtherList, FNameHelper::FNameFromString("Other_None"), SDK::UWorld::GetWorld());
                                SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairOtherList, part, SDK::UWorld::GetWorld());
                            }
                        }
                    }
                }
            }
        }
    }
    else if (TableName == "DT_HairBack")
    {
        if (ModPatch::DT_HairBack.size() > 0) {
            for (auto& entry : ModPatch::DT_HairBack)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == "DT_HairFront")
    {
        if (ModPatch::DT_HairFront.size() > 0) {
            for (auto& entry : ModPatch::DT_HairFront)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
	}
    else if (TableName == "DT_HairFrontSide")
    {
        if (ModPatch::DT_HairFrontSide.size() > 0) {
            for (auto& entry : ModPatch::DT_HairFrontSide)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
		}
    }
    else if (TableName == "DT_HairOther")
    {
        if (ModPatch::DT_HairOther.size() > 0) {
            for (auto& entry : ModPatch::DT_HairOther)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == "DT_HairSide")
    {
        if (ModPatch::DT_HairSide.size() > 0) {
            for (auto& entry : ModPatch::DT_HairSide)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == "DT_HairTop")
    {
        if (ModPatch::DT_HairTop.size() > 0) {
            for (auto& entry : ModPatch::DT_HairTop)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == "DT_HairSetList_Female")
    {
        if (ModPatch::DT_HairSetList_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_HairSetList_Female)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == "DT_HairSetList_Male")
    {
        if (ModPatch::DT_HairSetList_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_HairSetList_Male)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }

    else if (TableName == "DT_AccessoryPresetDLC_Free")
    {
        if (ModPatch::DT_AccessoryPresetDLC_Free.size() > 0) {
            for (auto& entry : ModPatch::DT_AccessoryPresetDLC_Free)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
			Patch_FlagCondition(_this, ModPatch::DT_AccessoryPresetDLC_Free, 0x01B0);
        }
    }
    else if (TableName == "DT_AccessoryPresetIsHat")
    {
        if (ModPatch::DT_AccessoryPresetIsHat.size() > 0) {
            for (auto& entry : ModPatch::DT_AccessoryPresetIsHat)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_AccessoryPresetIsHat, 0x01B0);
        }
    }
    else if (TableName == "DT_AccessoryAttachToList")
    {
        if (ModPatch::DT_AccessoryAttachToList.size() > 0)
        {
            for (auto& entry : ModPatch::DT_AccessoryAttachToList)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }

    else if (TableName == "DT_Boots_Female")
    {
        if (ModPatch::DT_Boots_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_Boots_Female)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Boots_Female, 0x058);
        }
    }
    else if (TableName == "DT_Boots_Male")
    {
        if (ModPatch::DT_Boots_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Boots_Male)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Boots_Male, 0x058);
        }
    }
    else if (TableName == "DT_Gloves_Female")
    {
        if (ModPatch::DT_Gloves_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_Gloves_Female)
            {
                //printf("[CVMerger] Entry location %p\n", &entry.second);
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Gloves_Female, 0x058);
        }
    }
    else if (TableName == "DT_Gloves_Male")
    {
        if (ModPatch::DT_Gloves_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Gloves_Male)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Gloves_Male, 0x058);
        }
    }
    else if (TableName == "DT_Mask_Female")
    {
        if (ModPatch::DT_Mask_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_Mask_Female)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Mask_Female, 0x058);
        }
    }
    else if (TableName == "DT_Mask_Male")
    {
        if (ModPatch::DT_Mask_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Mask_Male)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Mask_Male, 0x058);
        }
    }
    else if (TableName == "DT_Outer_Female")
    {
        if (ModPatch::DT_Outer_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_Outer_Female)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Outer_Female, 0x058);
        }
    }
    else if (TableName == "DT_Outer_Male")
    {
        if (ModPatch::DT_Outer_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Outer_Male)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Outer_Male, 0x058);
        }
    }
    
    else if (TableName == "DT_EyeBase")
    {
        if (ModPatch::DT_EyeBase.size() > 0) {
            for (auto& entry : ModPatch::DT_EyeBase)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
	}
    else if (TableName == "DT_EyeDetail")
    {
        if (ModPatch::DT_EyeDetail.size() > 0) {
            for (auto& entry : ModPatch::DT_EyeDetail)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == "DT_EyeHighlight")
    {
        if (ModPatch::DT_EyeHighlight.size() > 0) {
            for (auto& entry : ModPatch::DT_EyeHighlight)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }

    else if (TableName == "DT_Kintsugi")
    {
        if (ModPatch::DT_Kintsugi.size() > 0)
        {
            for (auto& entry : ModPatch::DT_Kintsugi)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == "DT_FacePaintDLC")
    {
        if (ModPatch::DT_FacePaintDLC.size() > 0)
        {
            for (auto& entry : ModPatch::DT_FacePaintDLC)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_FacePaintDLC, 0x88);
        }
    }
    else if (TableName == "DT_Manicure")
    {
        if (ModPatch::DT_Manicure.size() > 0)
        {
            for (auto& entry : ModPatch::DT_Manicure)
            {
                UDataTable_AddRow(_this, FNameHelper::FNameFromString(entry.first), (SDK::FTableRowBase*)(&entry.second));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }

    return;
}

HOOK(void, __stdcall, Hook_UGameFlowManager_OnShaderCompileWaitFinished, Sig_UGameFlowManager_OnShaderCompileWaitFinished, uint64_t* param1)
{
    ModPatch::init();
    orig_Hook_UGameFlowManager_OnShaderCompileWaitFinished(param1);
}

HOOK(void, __stdcall, Hook_CameraResetPressed, Sig_ACharacterCustomizePawn_CameraResetPressed, SDK::ACharacterCustomizePawn* _this)
{
    printf("[RHFF] Camera rest from %s\n", _this->GetName().c_str());

    SDK::FName name;
    SDK::FEulerAngleTransform LocalRoot;
    SDK::FEulerAngleTransform LocalOrient;
    SDK::FEulerAngleTransform LocalMesh;
    _this->GetAccessoryTransforms(&LocalRoot, &LocalOrient, &LocalMesh);

    printf("[RHFF] GetAccessoryTransforms:\n");
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

HOOK(void, __stdcall, Hook_InputHook, Sig_InputHook, SDK::UGameViewportClient* _this, SDK::FKey Key, SDK::EInputEvent EventType)
{
    if (Key.KeyName.ToString() == "F7")
    {
        if (EventType == SDK::EInputEvent::IE_Pressed)
        {
            
        }
    }
    orig_Hook_InputHook(_this, Key, EventType);
}

HOOK(HRESULT, __stdcall, hook_CreateFileW, &CreateFileW, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    //if (wcsstr(lpFileName, L"hid") == NULL) // don't log audio
    //{
    //    printf("File Access Log: %S\r\n", lpFileName);
    //}
    if (wcsstr(lpFileName, Config::CompanionPakName.c_str()))
    {
        printf("[RHFF] Loaded RHFF Companion pak, extra functionality is available.\n");
        Config::CompanionPakLoaded = true;
    }
    else if (wcsstr(lpFileName, Config::PresetModName.c_str()))
    {
        printf("[RHFF] Loaded Loaded Character Preset Mod.\n");
        Config::PresetModLoaded = true;
    }
    return orig_hook_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HOOK(void, __stdcall, Hook_UObject_ProcessEvent, Sig_UObject_ProcessEvent, SDK::UObject* _this, SDK::UFunction* Function, void* Param)
{
    //printf("[RHFF] ProcessEvent: %s\n", Function->GetFullName().c_str());
    std::string FuncName = Function->GetFullName();
    /*
    if (FuncName.contains("WDT_CharacterPresetImporter_C.Construct"))
    {
        CharacterCreatorPreset::GetSaveFiles(static_cast<SDK::UUserWidget*>(_this));
        printf("[RHFF] [CharacterPresets] Adding Save Files to Widget String Array.\n");
    }
    */

    if (!TitleLoad)
    {
        if (FuncName.contains("WDT_TitleMenu_Variationable.WDT_TitleMenu_Variationable_C.OnLoaded_EBBF5C074734D80FC8EB7495CA8B515E"))
        {
            orig_Hook_UObject_ProcessEvent(_this, Function, Param);

            SDK::UWDT_TitleMenu_Variationable_C* TitleWidget = (SDK::UWDT_TitleMenu_Variationable_C*)(_this);
            TitleWidget->ApplicationVersionText->SetText(FNameHelper::FTextFromString(std::format("{} - RHFF v2.1.1", TitleWidget->ApplicationVersionText->GetText().ToString())));

            TitleLoad = true;
            return;
        }
    }

    return orig_Hook_UObject_ProcessEvent(_this, Function, Param);
}

//Hook to fix normal colors being set to 1.0 roughness
HOOK(void, __stdcall, Hook_UCharacterCustomizeUtility_SetScalarParameterValueToAllMesh, Sig_UCharacterCustomizeUtility_SetScalarParameterValueToAllMesh,
    SDK::AActor* Target,
    SDK::FName ParameterName,
    float Value)
{
    //printf("[RHFF] [Debug] Set Scalar on \"%s\" for \"%s\" to \"%f\"\n", Target->GetName().c_str(), ParameterName.ToString().c_str(), Value);

    //Check for roughness
    if (ParameterName.ToString() == "Roughness(L1)" || ParameterName.ToString() == "Roughness(L2)")
    {
        //make sure it's setting it to 1.0
        if (Value == 1.0f)
        {
            if (std::find(MeshSlots.begin(), MeshSlots.end(), Target->GetName()) != MeshSlots.end() )
            {
                //printf("[RHFF] Patching roughness set on %s to 0.0\n", Target->GetName().c_str());
                return orig_Hook_UCharacterCustomizeUtility_SetScalarParameterValueToAllMesh(Target, ParameterName, 0.0f);
            }
            else if (Target->GetName().starts_with("AccessoryMesh"))
            {
                //printf("[RHFF] Patching roughness set on %s to 0.0\n", Target->GetName().c_str());
                return orig_Hook_UCharacterCustomizeUtility_SetScalarParameterValueToAllMesh(Target, ParameterName, 0.0f);
            }
        }
    }



    return orig_Hook_UCharacterCustomizeUtility_SetScalarParameterValueToAllMesh(Target, ParameterName, Value);
}

HOOK(void, __stdcall, Hook_ACharacterCustomizeSystem_SetMaskMesh, Sig_ACharacterCustomizeSystem_SetMaskMesh, SDK::UCharacterCustomizeComponent* _this, SDK::FName* InName)
{
    orig_Hook_ACharacterCustomizeSystem_SetMaskMesh(_this, InName);

    auto CCSystem = SDK::UGameSystemManager::GetCharacterCustomizeSystem(SDK::UWorld::GetWorld());
    auto maskRow = CCSystem->CharacterCustomizeDataManager->GetMaskDataByRowName(CCSystem->GetGender(), *InName);

    _this->CharacterCustomizeParameter.Mask.PartVisibilityArray.Clear();
    if (_this->CharacterCustomizeParameter.Mask.PartVisibilityArray.Max() < maskRow.PartVisibilityInfoArray.Num())
        TArrayHelper::ResizeTArray(&_this->CharacterCustomizeParameter.Mask.PartVisibilityArray, maskRow.PartVisibilityInfoArray.Num());

    for (int i = 0; i < maskRow.PartVisibilityInfoArray.Num(); i++)
    {
        _this->CharacterCustomizeParameter.Mask.PartVisibilityArray.Add(SDK::ECharacterCustomizeClothPartVisibility::Visible);
    }
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

        INSTALL_HOOK(hook_CreateFileW);
        INSTALL_HOOK(Hook_UObject_ProcessEvent);
		INSTALL_HOOK(Hook_UGameFlowManager_OnShaderCompileWaitFinished);
        INSTALL_HOOK(Hook_UDataTable_Serialize);
        INSTALL_HOOK(Hook_ACharacterCustomizeSystem_SetMaskMesh);
        if (Config::AccessoryInfo)
        {
            INSTALL_HOOK(Hook_CameraResetPressed);
        }
        //Not needed right now but maybe i'll need this in the future
        INSTALL_HOOK(Hook_InputHook);
        if (Config::FixSpecialRoughness)
            INSTALL_HOOK(Hook_UCharacterCustomizeUtility_SetScalarParameterValueToAllMesh)
        

		return TRUE;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

