#pragma once
#include "pch.h"
#include "SDK.hpp"

// A merged row plus its row-name FName, built once at TOML parse time so merging never converts strings.
template<typename T>
struct ModRow
{
	SDK::FName Name;
	T Data;
};

// Keyed by the TOML row name (keeps duplicate checks and the alphabetical add order).
template<typename T>
using ModTable = std::map<std::string, ModRow<T>>;

class ModPatch
{
public:
	static ModTable<SDK::FCharacterCustomizeDataTableInnerList> DT_Inner_Female;
	static ModTable<SDK::FCharacterCustomizeDataTableInnerList> DT_Inner_Male;
	static ModTable<SDK::FCharacterCustomizeDataTableBootsList> DT_Boots_Female;
	static ModTable<SDK::FCharacterCustomizeDataTableBootsList> DT_Boots_Male;
	static ModTable<SDK::FCharacterCustomizeDataTableGlovesList> DT_Gloves_Female;
	static ModTable<SDK::FCharacterCustomizeDataTableGlovesList> DT_Gloves_Male;
	static ModTable<SDK::FCharacterCustomizeDataTableMaskList> DT_Mask_Female;
	static ModTable<SDK::FCharacterCustomizeDataTableMaskList> DT_Mask_Male;
	static ModTable<SDK::FCharacterCustomizeDataTableOuterList> DT_Outer_Female;
	static ModTable<SDK::FCharacterCustomizeDataTableOuterList> DT_Outer_Male;

	static ModTable<SDK::FCharacterCustomizeDataTableAccessoryPreset> DT_AccessoryPresetDLC_Free;
	static ModTable<SDK::FCharacterCustomizeDataTableAccessoryPreset> DT_AccessoryPresetIsHat;
	static ModTable<SDK::FCharacterCustomizeDataTableAttachToList> DT_AccessoryAttachToList;

	static ModTable<SDK::FCharacterCustomizeDataTableHairBase> DT_HairBaseList;
	static ModTable<SDK::FCharacterCustomizeDataTableHairPart> DT_HairBack;
	static ModTable<SDK::FCharacterCustomizeDataTableHairPart> DT_HairFront;
	static ModTable<SDK::FCharacterCustomizeDataTableHairPart> DT_HairFrontSide;
	static ModTable<SDK::FCharacterCustomizeDataTableHairPart> DT_HairOther;
	static ModTable<SDK::FCharacterCustomizeDataTableHairPart> DT_HairSide;
	static ModTable<SDK::FCharacterCustomizeDataTableHairPart> DT_HairTop;
	static ModTable<SDK::FCharacterCustomizeDataTableHairSet> DT_HairSetList_Female;
	static ModTable<SDK::FCharacterCustomizeDataTableHairSet> DT_HairSetList_Male;
	static std::map<std::string, std::pair<std::vector<std::string>, SDK::FCharacterCustomizeDataTableHairBase>> DT_HairBaseListAddon;

	static ModTable<SDK::FCharacterCustomizeDataTableTextureList> DT_EyeBase;
	static ModTable<SDK::FCharacterCustomizeDataTableEyeTexture> DT_EyeDetail;
	static ModTable<SDK::FCharacterCustomizeDataTableTextureList> DT_EyeHighlight;
	static ModTable<SDK::FCharacterCustomizeDataTableEyebrowTexture> DT_Eyebrow;
	static ModTable<SDK::FCharacterCustomizeDataTableTextureList> DT_EyeLash;

	static ModTable<SDK::FCharacterCustomizeDataTableKintsugiList> DT_Kintsugi;
	static ModTable<SDK::FCharacterCustomizeDataTableFacePaintMaskPreset> DT_FacePaintDLC;
	static ModTable<SDK::FCharacterCustomizeDataTableManicurePatternList> DT_Manicure;

	static ModTable<SDK::FCharacterCustomizeDataTablePreset> DT_AppAvatarPresetFemale;
	static ModTable<SDK::FCharacterCustomizeDataTablePreset> DT_AppAvatarPresetMale;


private:
	static void ProcessInnerTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableInnerList>* DataTable);
	static void ProcessBootsTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableBootsList>* DataTable);
	static void ProcessGlovesTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableGlovesList>* DataTable);
	static void ProcessMaskTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableMaskList>* DataTable);
	static void ProcessOuterTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableOuterList>* DataTable);

	static void ProcessAccessoryPresetTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableAccessoryPreset>* DataTable);
	static void ProcessAccessoryAttachToTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableAttachToList>* DataTable);

	static void ProcessHairBaseTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableHairBase>* DataTable);
	static void ProcessHairPartTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableHairPart>* DataTable);
	static void ProcessHairSetTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableHairSet>* DataTable);
	static void ProcessHairBaseAddon(toml::table table, std::map<std::string, std::pair<std::vector<std::string>, SDK::FCharacterCustomizeDataTableHairBase>>* DataTable);

	static void ProcessTextureListTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableTextureList>* DataTable);
	static void ProcessEyeTextureTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableEyeTexture>* DataTable);
	static void ProcessEyeBrowTextureTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableEyebrowTexture>* DataTable);

	static void ProcessKintsugiTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableKintsugiList>* DataTable);
	static void ProcessFacePaintTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableFacePaintMaskPreset>* DataTable);
	static void ProcessManicureTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTableManicurePatternList>* DataTable);

	static void ProcessAvatarPresetTable(toml::table table, ModTable<SDK::FCharacterCustomizeDataTablePreset>* DataTable);

public:
	static bool init();
};

