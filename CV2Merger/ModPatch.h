#pragma once
#include "pch.h"
#include "SDK.hpp"

class ModPatch
{
public:
	static std::map<std::string, SDK::FCharacterCustomizeDataTableInnerList> DT_Inner_Female;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableInnerList> DT_Inner_Male;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableBootsList> DT_Boots_Female;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableBootsList> DT_Boots_Male;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableGlovesList> DT_Gloves_Female;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableGlovesList> DT_Gloves_Male;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableMaskList> DT_Mask_Female;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableMaskList> DT_Mask_Male;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableOuterList> DT_Outer_Female;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableOuterList> DT_Outer_Male;

	static std::map<std::string, SDK::FCharacterCustomizeDataTableAccessoryPreset> DT_AccessoryPresetDLC_Free;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableAccessoryPreset> DT_AccessoryPresetIsHat;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableAttachToList> DT_AccessoryAttachToList;

	static std::map<std::string, SDK::FCharacterCustomizeDataTableHairBase> DT_HairBaseList;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart> DT_HairBack;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart> DT_HairFront;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart> DT_HairFrontSide;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart> DT_HairOther;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart> DT_HairSide;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart> DT_HairTop;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableHairSet> DT_HairSetList_Female;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableHairSet> DT_HairSetList_Male;
	static std::map<std::string, std::pair<std::vector<std::string>, SDK::FCharacterCustomizeDataTableHairBase>> DT_HairBaseListAddon;

	static std::map<std::string, SDK::FCharacterCustomizeDataTableTextureList> DT_EyeBase;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableEyeTexture> DT_EyeDetail;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableTextureList> DT_EyeHighlight;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableEyebrowTexture> DT_Eyebrow;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableTextureList> DT_EyeLash;

	static std::map<std::string, SDK::FCharacterCustomizeDataTableKintsugiList> DT_Kintsugi;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableFacePaintMaskPreset> DT_FacePaintDLC;
	static std::map<std::string, SDK::FCharacterCustomizeDataTableManicurePatternList> DT_Manicure;

	static std::map<std::string, SDK::FCharacterCustomizeDataTablePreset> DT_AppAvatarPresetFemale;
	static std::map<std::string, SDK::FCharacterCustomizeDataTablePreset> DT_AppAvatarPresetMale;


private:
	static void ProcessInnerTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableInnerList>* DataTable);
	static void ProcessBootsTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableBootsList>* DataTable);
	static void ProcessGlovesTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableGlovesList>* DataTable);
	static void ProcessMaskTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableMaskList>* DataTable);
	static void ProcessOuterTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableOuterList>* DataTable);

	static void ProcessAccessoryPresetTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableAccessoryPreset>* DataTable);
	static void ProcessAccessoryAttachToTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableAttachToList>* DataTable);

	static void ProcessHairBaseTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableHairBase>* DataTable);
	static void ProcessHairPartTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart>* DataTable);
	static void ProcessHairSetTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableHairSet>* DataTable);
	static void ProcessHairBaseAddon(toml::table table, std::map<std::string, std::pair<std::vector<std::string>, SDK::FCharacterCustomizeDataTableHairBase>>* DataTable);

	static void ProcessTextureListTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableTextureList>* DataTable);
	static void ProcessEyeTextureTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableEyeTexture>* DataTable);
	static void ProcessEyeBrowTextureTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableEyebrowTexture>* DataTable);

	static void ProcessKintsugiTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableKintsugiList>* DataTable);
	static void ProcessFacePaintTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableFacePaintMaskPreset>* DataTable);
	static void ProcessManicureTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableManicurePatternList>* DataTable);

	static void ProcessAvatarPresetTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTablePreset>* DataTable);

public:
	static bool init();
};

