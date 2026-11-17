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
#include <atomic>
#include <unordered_set>

static std::vector<std::string> MeshSlots = { "InnerMesh", "GlovesMesh", "BootsMesh", "MaskMesh", "OuterMesh" };
static const wchar_t* const ModVersionSuffix = L" - RHFF v2.2.0";
static SDK::UFunction::FNativeFuncPtr orig_Exec_GetApplicationVersionString = nullptr;

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

//0x141600eb0 in version 2.0.3.0
void* Sig_UTitleMenuUIObject_GetApplicationVersionString = sigScan(
    "\x40\x53\x48\x83\xEC\x20\x65\x48\x8B\x04\x25\x2A\x2A\x2A\x2A\xBA\x74\x01\x00\x00\x48\x8B\x08\x8B\x04\x2A\x39\x05\x2A\x2A\x2A\x2A\x7F\x2A\x48\x8D\x05\x2A\x2A\x2A\x2A\x48\x83\xC4\x20\x5B\xC3\x4C\x8B\x05\x2A\x2A\x2A\x2A\x48\x8D\x0D\x2A\x2A\x2A\x2A\xF2\x0F\x10\x05\x2A\x2A\x2A\x2A",
    "xxxxxxxxxxx????xxxxxxxxxx?xx????x?xxx????xxxxxxxxx????xxx????xxxx????");

// Engine globals. Each sig matches an instruction that loads the global RIP-relative,
// so the address is resolved at startup instead of using the SDK's build-specific Offsets.

//0x1417270a0 in version 2.0.3.0 - object-index lookup, MOV RCX,[GObjects] (disp at +16, ends at +20)
void* Sig_GObjects = sigScan(
    "\x0F\xB7\xC8\x8B\xD0\x48\xC1\xEA\x10\x4C\x8D\x04\x49\x48\x8B\x0D\x2A\x2A\x2A\x2A\x48\x8B\x14\xD1\x4A\x83\x3C\xC2\x00",
    "xxxxxxxxxxxxxxxx????xxxxxxxxx");

//0x1416552c0 in version 2.0.3.0 - FNameEntry lookup, LEA RDX,[GNames] (disp at +20, ends at +24)
void* Sig_GNames = sigScan(
    "\x40\x53\x48\x83\xEC\x20\x80\x3D\x2A\x2A\x2A\x2A\x00\x8B\xD9\x74\x09\x48\x8D\x15\x2A\x2A\x2A\x2A\xEB\x16\x48\x8D\x0D\x2A\x2A\x2A\x2A\xE8",
    "xxxxxxxx????xxxxxxxx????xxxxx????x");

//0x143837860 in version 2.0.3.0 - GetWorld-style accessor, MOV RAX,[GWorld] (disp at +21, ends at +25)
void* Sig_GWorld = sigScan(
    "\x48\x8B\x41\x18\x48\x85\xC0\x74\x09\x48\x8B\x40\x30\x48\x85\xC0\x75\x07\x48\x8B\x05\x2A\x2A\x2A\x2A\xC3",
    "xxxxxxxxxxxxxxxxxxxxx????x");

#pragma endregion

#pragma region Functions

FUNCTION_PTR(void, __fastcall, UDataTable_AddRow, Sig_UDataTable_AddRow, SDK::UDataTable* _this, SDK::FName RowName, SDK::FTableRowBase* RowData);

// Points the SDK at GObjects/GNames and the world helper at GWorld. Must run before any SDK call.
// If a signature isn't found, GObjects/GNames fall back to the SDK's built-in offsets (only right
// for the game version the SDK was dumped from) and GetWorld falls back to the viewport's world.
void InitEngineGlobals()
{
    uintptr_t Base = (uintptr_t)GetModuleHandle(nullptr);

    void* GObjects = resolveRipRelative(Sig_GObjects, 16, 20);
    void* GNames = resolveRipRelative(Sig_GNames, 20, 24);
    void* GWorld = resolveRipRelative(Sig_GWorld, 21, 25);

    if (GObjects)
        SDK::UObject::GObjects.InitManually(GObjects);
    if (GNames)
        SDK::FName::InitManually(GNames);
    WorldHelper::Init(GWorld);

    // Offsets are printed relative to the image base so they can be compared against a fresh SDK dump's Offsets.
    auto Report = [Base](const char* Name, void* Address, const char* Fallback)
    {
        if (Address)
            printf("[RHFF] %s found at base+0x%llX\n", Name, (unsigned long long)((uintptr_t)Address - Base));
        else
            printf("[RHFF] [Warning] %s signature not found, %s\n", Name, Fallback);
    };
    Report("GObjects", GObjects, "using the SDK's built-in offset");
    Report("GNames", GNames, "using the SDK's built-in offset");
    Report("GWorld", GWorld, "using the game viewport's world instead");
}

void EnableConsole()
{
    SDK::UEngine* Engine = SDK::UEngine::GetEngine();
    SDK::UWorld* World = WorldHelper::GetWorld();

    SDK::UInputSettings::GetDefaultObj()->ConsoleKeys[0].KeyName = SDK::UKismetStringLibrary::Conv_StringToName(L"Tilde");
    SDK::UObject* NewObject = SDK::UGameplayStatics::SpawnObject(Engine->ConsoleClass, Engine->GameViewport);
    Engine->GameViewport->ViewportConsole = static_cast<SDK::UConsole*>(NewObject);

    SDK::APlayerController* MyController = World->OwningGameInstance->LocalPlayers[0]->PlayerController;
    //MyController->CheatClass = SDK::UAppCheatManager::StaticClass();
    //MyController->CheatManager = static_cast<SDK::UAppCheatManager*>(SDK::UGameplayStatics::SpawnObject(MyController->CheatClass, MyController));
    MyController->EnableCheats();

}

// Packs both FName fields into one integer so it can be hashed; equal FNames give equal keys.
inline uint64_t FNameKey(const SDK::FName& Name)
{
    return (static_cast<uint64_t>(static_cast<uint32_t>(Name.ComparisonIndex)) << 32) | static_cast<uint32_t>(Name.Number);
}

// Clears the flag condition on the table's copy of every merged row (AddRow copies rows into the table).
template <typename T>
void Patch_FlagCondition(SDK::UDataTable* TablePtr, const ModTable<T>& ModRows, uint64_t offset)
{
    std::unordered_set<uint64_t> ModRowNames;
    ModRowNames.reserve(ModRows.size());
    for (const auto& [key, row] : ModRows)
        ModRowNames.insert(FNameKey(row.Name));

    for (auto& entry : TablePtr->RowMap)
    {
        if (ModRowNames.contains(FNameKey(entry.First)))
        {
            //printf("[RHFF] Row Location %p\n", entry.Second);
            WRITE_MEMORY(entry.Second + offset, uint8_t, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0)
        }
    }
}

// Every DataTable the merger patches. Add a table here to get a matching TableNames::<name> FName.
#define MERGED_DATATABLES(X) \
    X(DT_Inner_Female) X(DT_Inner_Male) \
    X(DT_HairBaseList) X(DT_HairBack) X(DT_HairFront) X(DT_HairFrontSide) X(DT_HairOther) X(DT_HairSide) X(DT_HairTop) \
    X(DT_HairSetList_Female) X(DT_HairSetList_Male) \
    X(DT_AccessoryPresetDLC_Free) X(DT_AccessoryPresetIsHat) X(DT_AccessoryAttachToList) \
    X(DT_Boots_Female) X(DT_Boots_Male) X(DT_Gloves_Female) X(DT_Gloves_Male) \
    X(DT_Mask_Female) X(DT_Mask_Male) X(DT_Outer_Female) X(DT_Outer_Male) \
    X(DT_EyeBase) X(DT_EyeDetail) X(DT_EyeHighlight) \
    X(DT_Kintsugi) X(DT_FacePaintDLC) X(DT_Manicure)

// FNames for the table names, built once so the Serialize hook compares integers instead of strings.
namespace TableNames
{
#define DECLARE_TABLE_FNAME(Name) static SDK::FName Name;
    MERGED_DATATABLES(DECLARE_TABLE_FNAME)
#undef DECLARE_TABLE_FNAME
}

// Set once TableNames is built. Before that, ModPatch::init() hasn't run, so there's nothing to merge anyway.
static std::atomic<bool> TableNamesReady = false;

// Conv_StringToName needs the UObject system up and goes through ProcessEvent, so call this on the game thread.
void InitTableNames()
{
#define INIT_TABLE_FNAME(Name) TableNames::Name = FNameHelper::FNameFromString(#Name);
    MERGED_DATATABLES(INIT_TABLE_FNAME)
#undef INIT_TABLE_FNAME
    TableNamesReady.store(true, std::memory_order_release);
}

// Replaces the native thunk of UTitleMenuUIObject::GetApplicationVersionString, which the title menu
// Blueprint's event graph calls to fill ApplicationVersionText. Only runs when the title menu asks for it.
static void Exec_GetApplicationVersionString(void* Context, void* Stack, void* Result)
{
    orig_Exec_GetApplicationVersionString(Context, Stack, Result);

    if (!Result)
        return;

    SDK::FString* Version = static_cast<SDK::FString*>(Result);
    *Version = SDK::UKismetStringLibrary::Concat_StrStr(*Version, SDK::FString(ModVersionSuffix));
}

// Swaps the UFunction's native pointer (no signature needed: the function is found by name).
// Needs the UObject system up, so it's called from the OnShaderCompileWaitFinished hook.
static void InstallTitleVersionText()
{
    if (orig_Exec_GetApplicationVersionString)
        return;

    SDK::UClass* TitleUIObjectClass = SDK::UTitleMenuUIObject::StaticClass();
    SDK::UFunction* Func = TitleUIObjectClass ? TitleUIObjectClass->GetFunction("TitleMenuUIObject", "GetApplicationVersionString") : nullptr;
    if (!Func || !Func->ExecFunction)
    {
        printf("[RHFF] [Warning] Couldn't find TitleMenuUIObject.GetApplicationVersionString, title version text won't show the mod version\n");
        return;
    }

    orig_Exec_GetApplicationVersionString = Func->ExecFunction;
    Func->ExecFunction = &Exec_GetApplicationVersionString;
}

#pragma endregion

#pragma region Hooks

HOOK(void, __stdcall, Hook_UDataTable_Serialize, Sig_UDataTable_Serialize, SDK::UDataTable* _this, uint64_t* param2)
{
    //printf("[RHFF] UDataTable::Serialize called for DataTable: %s\n", _this->GetName().c_str());
    orig_Hook_UDataTable_Serialize(_this, param2);

    if (!TableNamesReady.load(std::memory_order_acquire))
        return;

    const SDK::FName& TableName = _this->Name;

    if (TableName == TableNames::DT_Inner_Female)
    {
        if (ModPatch::DT_Inner_Female.size() > 0) {
            //printf("[RHFF] DataTableName: %s\n", _this->GetFullName().c_str());
            for (auto& entry : ModPatch::DT_Inner_Female)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Inner_Female, offsetof(SDK::FCharacterCustomizeDataTableInnerList, FlagCondition));
        }
    }
    else if (TableName == TableNames::DT_Inner_Male)
    {
        if (ModPatch::DT_Inner_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Inner_Male)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Inner_Male, offsetof(SDK::FCharacterCustomizeDataTableInnerList, FlagCondition));
        }
    }

    else if (TableName == TableNames::DT_HairBaseList)
    {
        if (ModPatch::DT_HairBaseList.size() > 0) {
            for (auto& entry : ModPatch::DT_HairBaseList)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
		}

        //Addon table is for merging new parts options into an existing hair base
        if (Config::CompanionPakLoaded)
        {
            SDK::UWorld* World = WorldHelper::GetWorld();
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
                                        SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontList, FNameHelper::FNameFromString("Front_None"), World);
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontList, part, World);
                                }

                                for (auto& part : entry.second.second.AvailableHairFrontSideList)
                                {
                                    if (TArrayHelper::Contains(row->AvailableHairFrontSideList, part))
                                        continue;
                                    if (row->AvailableHairFrontSideList.Num() <= 0)
                                        SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontSideList, FNameHelper::FNameFromString("FrontSide_None"), World);
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontSideList, part, World);
                                }

                                for (auto& part : entry.second.second.AvailableHairSideList)
                                {
                                    if (TArrayHelper::Contains(row->AvailableHairSideList, part))
                                        continue;
                                    if (row->AvailableHairSideList.Num() <= 0)
                                        SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairSideList, FNameHelper::FNameFromString("Side_None"), World);
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairSideList, part, World);
                                }

                                for (auto& part : entry.second.second.AvailableHairTopList)
                                {
                                    if (TArrayHelper::Contains(row->AvailableHairTopList, part))
                                        continue;
                                    if (row->AvailableHairTopList.Num() <= 0)
                                        SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairTopList, FNameHelper::FNameFromString("Top_None"), World);
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairTopList, part, World);
                                }

                                for (auto& part : entry.second.second.AvailableHairBackList)
                                {
                                    if (TArrayHelper::Contains(row->AvailableHairBackList, part))
                                        continue;
                                    if (row->AvailableHairBackList.Num() <= 0)
                                        SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairBackList, FNameHelper::FNameFromString("Back_None"), World);
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairBackList, part, World);
                                }

                                for (auto& part : entry.second.second.AvailableHairOtherList)
                                {
                                    if (TArrayHelper::Contains(row->AvailableHairOtherList, part))
                                        continue;
                                    if (row->AvailableHairOtherList.Num() <= 0)
                                        SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairOtherList, FNameHelper::FNameFromString("Other_None"), World);
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairOtherList, part, World);
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
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontList, FNameHelper::FNameFromString("Front_None"), World);
                                SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontList, part, World);
                            }

                            for (auto& part : entry.second.second.AvailableHairFrontSideList)
                            {
                                if (row->AvailableHairFrontSideList.Num() <= 0)
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontSideList, FNameHelper::FNameFromString("FrontSide_None"), World);
                                SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairFrontSideList, part, World);
                            }

                            for (auto& part : entry.second.second.AvailableHairSideList)
                            {
                                if (row->AvailableHairSideList.Num() <= 0)
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairSideList, FNameHelper::FNameFromString("Side_None"), World);
                                SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairSideList, part, World);
                            }

                            for (auto& part : entry.second.second.AvailableHairTopList)
                            {
                                if (row->AvailableHairTopList.Num() <= 0)
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairTopList, FNameHelper::FNameFromString("Top_None"), World);
                                SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairTopList, part, World);
                            }

                            for (auto& part : entry.second.second.AvailableHairBackList)
                            {
                                if (row->AvailableHairBackList.Num() <= 0)
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairBackList, FNameHelper::FNameFromString("Back_None"), World);
                                SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairBackList, part, World);
                            }

                            for (auto& part : entry.second.second.AvailableHairOtherList)
                            {
                                if (row->AvailableHairOtherList.Num() <= 0)
                                    SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairOtherList, FNameHelper::FNameFromString("Other_None"), World);
                                SDK::UBPFL_MapHelper_C::Add_to_FName_Array(row->AvailableHairOtherList, part, World);
                            }
                        }
                    }
                }
            }
        }
    }
    else if (TableName == TableNames::DT_HairBack)
    {
        if (ModPatch::DT_HairBack.size() > 0) {
            for (auto& entry : ModPatch::DT_HairBack)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == TableNames::DT_HairFront)
    {
        if (ModPatch::DT_HairFront.size() > 0) {
            for (auto& entry : ModPatch::DT_HairFront)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
	}
    else if (TableName == TableNames::DT_HairFrontSide)
    {
        if (ModPatch::DT_HairFrontSide.size() > 0) {
            for (auto& entry : ModPatch::DT_HairFrontSide)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
		}
    }
    else if (TableName == TableNames::DT_HairOther)
    {
        if (ModPatch::DT_HairOther.size() > 0) {
            for (auto& entry : ModPatch::DT_HairOther)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == TableNames::DT_HairSide)
    {
        if (ModPatch::DT_HairSide.size() > 0) {
            for (auto& entry : ModPatch::DT_HairSide)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == TableNames::DT_HairTop)
    {
        if (ModPatch::DT_HairTop.size() > 0) {
            for (auto& entry : ModPatch::DT_HairTop)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == TableNames::DT_HairSetList_Female)
    {
        if (ModPatch::DT_HairSetList_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_HairSetList_Female)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == TableNames::DT_HairSetList_Male)
    {
        if (ModPatch::DT_HairSetList_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_HairSetList_Male)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }

    else if (TableName == TableNames::DT_AccessoryPresetDLC_Free)
    {
        if (ModPatch::DT_AccessoryPresetDLC_Free.size() > 0) {
            for (auto& entry : ModPatch::DT_AccessoryPresetDLC_Free)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
			Patch_FlagCondition(_this, ModPatch::DT_AccessoryPresetDLC_Free, offsetof(SDK::FCharacterCustomizeDataTableAccessoryPreset, FlagCondition));
        }
    }
    else if (TableName == TableNames::DT_AccessoryPresetIsHat)
    {
        if (ModPatch::DT_AccessoryPresetIsHat.size() > 0) {
            for (auto& entry : ModPatch::DT_AccessoryPresetIsHat)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_AccessoryPresetIsHat, offsetof(SDK::FCharacterCustomizeDataTableAccessoryPreset, FlagCondition));
        }
    }
    else if (TableName == TableNames::DT_AccessoryAttachToList)
    {
        if (ModPatch::DT_AccessoryAttachToList.size() > 0)
        {
            for (auto& entry : ModPatch::DT_AccessoryAttachToList)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }

    else if (TableName == TableNames::DT_Boots_Female)
    {
        if (ModPatch::DT_Boots_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_Boots_Female)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Boots_Female, offsetof(SDK::FCharacterCustomizeDataTableBootsList, FlagCondition));
        }
    }
    else if (TableName == TableNames::DT_Boots_Male)
    {
        if (ModPatch::DT_Boots_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Boots_Male)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Boots_Male, offsetof(SDK::FCharacterCustomizeDataTableBootsList, FlagCondition));
        }
    }
    else if (TableName == TableNames::DT_Gloves_Female)
    {
        if (ModPatch::DT_Gloves_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_Gloves_Female)
            {
                //printf("[CVMerger] Entry location %p\n", &entry.second);
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Gloves_Female, offsetof(SDK::FCharacterCustomizeDataTableGlovesList, FlagCondition));
        }
    }
    else if (TableName == TableNames::DT_Gloves_Male)
    {
        if (ModPatch::DT_Gloves_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Gloves_Male)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Gloves_Male, offsetof(SDK::FCharacterCustomizeDataTableGlovesList, FlagCondition));
        }
    }
    else if (TableName == TableNames::DT_Mask_Female)
    {
        if (ModPatch::DT_Mask_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_Mask_Female)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Mask_Female, offsetof(SDK::FCharacterCustomizeDataTableMaskList, FlagCondition));
        }
    }
    else if (TableName == TableNames::DT_Mask_Male)
    {
        if (ModPatch::DT_Mask_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Mask_Male)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Mask_Male, offsetof(SDK::FCharacterCustomizeDataTableMaskList, FlagCondition));
        }
    }
    else if (TableName == TableNames::DT_Outer_Female)
    {
        if (ModPatch::DT_Outer_Female.size() > 0) {
            for (auto& entry : ModPatch::DT_Outer_Female)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Outer_Female, offsetof(SDK::FCharacterCustomizeDataTableOuterList, FlagCondition));
        }
    }
    else if (TableName == TableNames::DT_Outer_Male)
    {
        if (ModPatch::DT_Outer_Male.size() > 0) {
            for (auto& entry : ModPatch::DT_Outer_Male)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_Outer_Male, offsetof(SDK::FCharacterCustomizeDataTableOuterList, FlagCondition));
        }
    }
    
    else if (TableName == TableNames::DT_EyeBase)
    {
        if (ModPatch::DT_EyeBase.size() > 0) {
            for (auto& entry : ModPatch::DT_EyeBase)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
	}
    else if (TableName == TableNames::DT_EyeDetail)
    {
        if (ModPatch::DT_EyeDetail.size() > 0) {
            for (auto& entry : ModPatch::DT_EyeDetail)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == TableNames::DT_EyeHighlight)
    {
        if (ModPatch::DT_EyeHighlight.size() > 0) {
            for (auto& entry : ModPatch::DT_EyeHighlight)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }

    else if (TableName == TableNames::DT_Kintsugi)
    {
        if (ModPatch::DT_Kintsugi.size() > 0)
        {
            for (auto& entry : ModPatch::DT_Kintsugi)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }
    else if (TableName == TableNames::DT_FacePaintDLC)
    {
        if (ModPatch::DT_FacePaintDLC.size() > 0)
        {
            for (auto& entry : ModPatch::DT_FacePaintDLC)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
            Patch_FlagCondition(_this, ModPatch::DT_FacePaintDLC, offsetof(SDK::FCharacterCustomizeDataTableFacePaintMaskPreset, FlagCondition));
        }
    }
    else if (TableName == TableNames::DT_Manicure)
    {
        if (ModPatch::DT_Manicure.size() > 0)
        {
            for (auto& entry : ModPatch::DT_Manicure)
            {
                UDataTable_AddRow(_this, entry.second.Name, (SDK::FTableRowBase*)(&entry.second.Data));
                printf("[RHFF] [DT Merger] Added entry %s to DataTable \"%s\"\n", entry.first.c_str(), _this->GetName().c_str());
            }
        }
    }

    return;
}

HOOK(void, __stdcall, Hook_UGameFlowManager_OnShaderCompileWaitFinished, Sig_UGameFlowManager_OnShaderCompileWaitFinished, uint64_t* param1)
{
    ModPatch::init();
    InitTableNames();
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
    //std::string FuncName = Function->GetFullName();

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

    auto CCSystem = SDK::UGameSystemManager::GetCharacterCustomizeSystem(WorldHelper::GetWorld());
    auto maskRow = CCSystem->CharacterCustomizeDataManager->GetMaskDataByRowName(CCSystem->GetGender(), *InName);

    _this->CharacterCustomizeParameter.Mask.PartVisibilityArray.Clear();
    if (_this->CharacterCustomizeParameter.Mask.PartVisibilityArray.Max() < maskRow.PartVisibilityInfoArray.Num())
        TArrayHelper::ResizeTArray(&_this->CharacterCustomizeParameter.Mask.PartVisibilityArray, maskRow.PartVisibilityInfoArray.Num());

    for (int i = 0; i < maskRow.PartVisibilityInfoArray.Num(); i++)
    {
        _this->CharacterCustomizeParameter.Mask.PartVisibilityArray.Add(SDK::ECharacterCustomizeClothPartVisibility::Visible);
    }
}

HOOK(SDK::FString*, __stdcall, Hook_UTitleMenuUIObject_GetApplicationVersionString, Sig_UTitleMenuUIObject_GetApplicationVersionString)
{
    SDK::FString* Version = orig_Hook_UTitleMenuUIObject_GetApplicationVersionString();
    if (!Version)
        return nullptr;

    if (!SDK::UKismetStringLibrary::Contains(*Version, SDK::FString(ModVersionSuffix), false, false))
        *Version = SDK::UKismetStringLibrary::Concat_StrStr(*Version, SDK::FString(ModVersionSuffix));
    return Version;
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

        InitEngineGlobals();

        INSTALL_HOOK(Hook_UTitleMenuUIObject_GetApplicationVersionString);
        INSTALL_HOOK(hook_CreateFileW);
        //INSTALL_HOOK(Hook_UObject_ProcessEvent);
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

