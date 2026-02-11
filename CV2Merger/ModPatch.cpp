#include "ModPatch.h"
#include <filesystem>
#include "Util.h"
#include "magic_enum.hpp"
#include "Config.h"

std::map<std::string, SDK::FCharacterCustomizeDataTableInnerList> ModPatch::DT_Inner_Female;
std::map<std::string, SDK::FCharacterCustomizeDataTableInnerList> ModPatch::DT_Inner_Male;
std::map<std::string, SDK::FCharacterCustomizeDataTableBootsList> ModPatch::DT_Boots_Female;
std::map<std::string, SDK::FCharacterCustomizeDataTableBootsList> ModPatch::DT_Boots_Male;
std::map<std::string, SDK::FCharacterCustomizeDataTableGlovesList> ModPatch::DT_Gloves_Female;
std::map<std::string, SDK::FCharacterCustomizeDataTableGlovesList> ModPatch::DT_Gloves_Male;
std::map<std::string, SDK::FCharacterCustomizeDataTableMaskList> ModPatch::DT_Mask_Female;
std::map<std::string, SDK::FCharacterCustomizeDataTableMaskList> ModPatch::DT_Mask_Male;
std::map<std::string, SDK::FCharacterCustomizeDataTableOuterList> ModPatch::DT_Outer_Female;
std::map<std::string, SDK::FCharacterCustomizeDataTableOuterList> ModPatch::DT_Outer_Male;

std::map<std::string, SDK::FCharacterCustomizeDataTableAccessoryPreset> ModPatch::DT_AccessoryPresetDLC_Free;
std::map<std::string, SDK::FCharacterCustomizeDataTableAttachToList> ModPatch::DT_AccessoryAttachToList;

std::map<std::string, SDK::FCharacterCustomizeDataTableHairBase> ModPatch::DT_HairBaseList;
std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart> ModPatch::DT_HairBack;
std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart> ModPatch::DT_HairFront;
std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart> ModPatch::DT_HairFrontSide;
std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart> ModPatch::DT_HairOther;
std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart> ModPatch::DT_HairSide;
std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart> ModPatch::DT_HairTop;
std::map<std::string, SDK::FCharacterCustomizeDataTableHairSet> ModPatch::DT_HairSetList_Female;
std::map<std::string, SDK::FCharacterCustomizeDataTableHairSet> ModPatch::DT_HairSetList_Male;

void ModPatch::ProcessInnerTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableInnerList>* DataTable)
{
	for (auto& [key, value] : table)
	{
		std::string name = key.str().data();
		toml::array* data = nullptr;

		if (value.is_array())
			data = value.as_array();
		else
		{
			printf("[CV2Merger] [TOML] entry \"%s\" in toml is not an array", name);
			continue;
		}

		if (data->size() <= 0)
		{
			printf("[CV2Merger] [TOML] entry data for \"%s\" is empty in toml\n", name.c_str());
			continue;
		}
		if (DataTable->contains(name.c_str()))
		{
			printf("[CV2Merger] [TOML] Duplicate entry for \"%s\" found in toml\n", name.c_str());
			continue;
		}

		SDK::TSoftObjectPtr<SDK::UTexture2D> thumbnailpath;
		thumbnailpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(0)->value_or(""))};
		SDK::TSoftObjectPtr<SDK::USkeletalMesh> meshpath;
		meshpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(1)->value_or("")) };

		SDK::FCharacterCustomizeDataTableInnerList Inner = *new SDK::FCharacterCustomizeDataTableInnerList();
		Inner.Thumbnail = thumbnailpath;
		Inner.Mesh = meshpath;
		
		if (data->get(2)->value_or("") != "")
		{
			Inner.FlagCondition = *new SDK::FStoryFlagEvaluationFormula();
			Inner.FlagCondition.EvaluationFormula = FNameHelper::FStringFromString(data->get(2)->value_or(""));
		}
		if (data->get(3)->as_array()->size() != 7)
		{
			printf("[CV2Merger] [TOML] entry data for \"%s\" has invalid cloth color count in toml\n", name.c_str());
		}
		else
		{
			int colorIndex = 0;
			for (auto& color : *data->get(3)->as_array())
			{
				toml::array* colorArray = color.as_array();
				SDK::FCharacterCustomizeDataTableClothColor clothColor = *new SDK::FCharacterCustomizeDataTableClothColor();
				clothColor.IsSpecialColor = colorArray->get(0)->value_or(false);
				clothColor.ColorPaletteRowName = FNameHelper::FNameFromString(colorArray->get(1)->value_or(""));
				clothColor.ColorName = FNameHelper::FNameFromString(colorArray->get(2)->value_or(""));

				Inner.Color[colorIndex] = clothColor;
				colorIndex++;
			}
		}
		Inner.CustomizableColorNumber = data->get(4)->value_or(0);
		Inner.PartVisibilityInfoArray = *new SDK::TAllocatedArray<SDK::FCharacterCustomizeDataTableClothPartVisibility>(data->get(5)->as_array()->size() + 5);
		for (auto& partVisibility : *data->get(5)->as_array())
		{
			toml::array* partArray = partVisibility.as_array();
			SDK::FCharacterCustomizeDataTableClothPartVisibility partVis = *new SDK::FCharacterCustomizeDataTableClothPartVisibility();

			SDK::TSoftObjectPtr<SDK::UTexture2D> clothTexturepath;
			clothTexturepath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(partArray->get(0)->value_or("")) };
			partVis.ClothPartTexture = clothTexturepath;

			toml::array* HideParamsArray = partArray->get(1)->as_array();
			partVis.ClothPartHideParameters = *new SDK::TAllocatedArray<SDK::FName>(20);
			for (auto& hideParam : *HideParamsArray)
			{
				partVis.ClothPartHideParameters.Add(FNameHelper::FNameFromString(hideParam.value_or("")));
			}

			toml::array* SubstituteParamsArray = partArray->get(2)->as_array();
			partVis.ClothPartSubstituteParameters = *new SDK::TAllocatedArray<SDK::FName>(20);
			for (auto& subParam : *SubstituteParamsArray)
			{
				partVis.ClothPartSubstituteParameters.Add(FNameHelper::FNameFromString(subParam.value_or("")));
			}

			partVis.ParentIndex = partArray->get(3)->value_or(-1);
			partVis.PartType = magic_enum::enum_cast<SDK::ECharacterCustomizePartType>(partArray->get(4)->value_or("General")).value_or(SDK::ECharacterCustomizePartType::General);
			Inner.PartVisibilityInfoArray.Add(partVis);
		}
		Inner.RightVolumeType = magic_enum::enum_cast<SDK::ECharacterCustomizeBootsHeightType>(data->get(6)->value_or("None")).value_or(SDK::ECharacterCustomizeBootsHeightType::None);
		Inner.LeftVolumeType = magic_enum::enum_cast<SDK::ECharacterCustomizeBootsHeightType>(data->get(7)->value_or("None")).value_or(SDK::ECharacterCustomizeBootsHeightType::None);
		Inner.bHasBoots = data->get(8)->value_or(false);
		Inner.HairAffectType = magic_enum::enum_cast<SDK::ECharacterCustomizeClothHairAffectType>(data->get(9)->value_or("None")).value_or(SDK::ECharacterCustomizeClothHairAffectType::None);
		Inner.bAffectNail = data->get(10)->value_or(false);
		Inner.bAffectPedicure = data->get(11)->value_or(false);
		Inner.DefaultMaskRowName = FNameHelper::FNameFromString(data->get(12)->value_or("None_Mask"));
		Inner.DefaultGlovesRowName = FNameHelper::FNameFromString(data->get(13)->value_or("None_GV"));
		Inner.DefaultBootsRowName = FNameHelper::FNameFromString(data->get(14)->value_or("None_Boots"));
		Inner.DefaultOuterRowName = FNameHelper::FNameFromString(data->get(15)->value_or("None_Outer"));

		DataTable->insert({ name, Inner });
	}
}

void ModPatch::ProcessBootsTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableBootsList>* DataTable)
{
	for (auto& [key, value] : table)
	{
		std::string name = key.str().data();
		toml::array* data = nullptr;

		if (value.is_array())
			data = value.as_array();
		else
		{
			printf("[CV2Merger] [TOML] entry \"%s\" in toml is not an array", name);
			continue;
		}

		if (data->size() <= 0)
		{
			printf("[CV2Merger] [TOML] entry data for \"%s\" is empty in toml\n", name.c_str());
			continue;
		}
		if (DataTable->contains(name.c_str()))
		{
			printf("[CV2Merger] [TOML] Duplicate entry for \"%s\" found in toml\n", name.c_str());
			continue;
		}

		SDK::FCharacterCustomizeDataTableBootsList Boots = *new SDK::FCharacterCustomizeDataTableBootsList();
		SDK::TSoftObjectPtr<SDK::UTexture2D> thumbnailpath;
		thumbnailpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(0)->value_or("")) };
		Boots.Thumbnail = thumbnailpath;

		SDK::TSoftObjectPtr<SDK::USkeletalMesh> meshpath;
		meshpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(1)->value_or("")) };
		Boots.Mesh = meshpath;

		if (data->get(2)->value_or("") != "")
		{
			Boots.FlagCondition = *new SDK::FStoryFlagEvaluationFormula();
			Boots.FlagCondition.EvaluationFormula = FNameHelper::FStringFromString(data->get(2)->value_or(""));
		}
		int colorIndex = 0;
		for (auto& color : *data->get(3)->as_array())
		{
			toml::array* colorArray = color.as_array();
			SDK::FCharacterCustomizeDataTableClothColor clothColor = *new SDK::FCharacterCustomizeDataTableClothColor();
			clothColor.IsSpecialColor = colorArray->get(0)->value_or(false);
			clothColor.ColorPaletteRowName = FNameHelper::FNameFromString(colorArray->get(1)->value_or(""));
			clothColor.ColorName = FNameHelper::FNameFromString(colorArray->get(2)->value_or(""));

			Boots.Color[colorIndex] = clothColor;
			colorIndex++;
		}
		Boots.CustomizableColorNumber = data->get(4)->value_or(0);
		Boots.PartVisibilityInfoArray = *new SDK::TAllocatedArray<SDK::FCharacterCustomizeDataTableClothPartVisibility>(data->get(5)->as_array()->size() + 5);
		for (auto& partVisibility : *data->get(5)->as_array())
		{
			toml::array* partArray = partVisibility.as_array();
			SDK::FCharacterCustomizeDataTableClothPartVisibility partVis = *new SDK::FCharacterCustomizeDataTableClothPartVisibility();

			SDK::TSoftObjectPtr<SDK::UTexture2D> clothTexturepath;
			clothTexturepath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(0)->value_or("")) };
			partVis.ClothPartTexture = clothTexturepath;

			toml::array* HideParamsArray = partArray->get(1)->as_array();
			partVis.ClothPartHideParameters = *new SDK::TAllocatedArray<SDK::FName>(20);
			for (auto& hideParam : *HideParamsArray)
			{
				partVis.ClothPartHideParameters.Add(FNameHelper::FNameFromString(hideParam.value_or("")));
			}

			toml::array* SubstituteParamsArray = partArray->get(2)->as_array();
			partVis.ClothPartSubstituteParameters = *new SDK::TAllocatedArray<SDK::FName>(20);
			for (auto& subParam : *SubstituteParamsArray)
			{
				partVis.ClothPartSubstituteParameters.Add(FNameHelper::FNameFromString(subParam.value_or("")));
			}

			partVis.ParentIndex = partArray->get(3)->value_or(-1);
			partVis.PartType = magic_enum::enum_cast<SDK::ECharacterCustomizePartType>(partArray->get(4)->value_or("General")).value_or(SDK::ECharacterCustomizePartType::General);
			Boots.PartVisibilityInfoArray.Add(partVis);
		}
		Boots.RightHeightType = magic_enum::enum_cast<SDK::ECharacterCustomizeBootsHeightType>(data->get(6)->value_or("None")).value_or(SDK::ECharacterCustomizeBootsHeightType::None);
		Boots.LeftHeightType = magic_enum::enum_cast<SDK::ECharacterCustomizeBootsHeightType>(data->get(7)->value_or("None")).value_or(SDK::ECharacterCustomizeBootsHeightType::None);
		for (auto& part : *data->get(8)->as_array())
		{
			Boots.RightHeightDependentParameter.Add(FNameHelper::FNameFromString(part.value_or("")));
		}
		for (auto& part : *data->get(9)->as_array())
		{
			Boots.LeftHeightDependentParameter.Add(FNameHelper::FNameFromString(part.value_or("")));
		}
		Boots.bAffectPedicure = data->get(10)->value_or(false);

		DataTable->insert({ name, Boots });
	}
}

void ModPatch::ProcessGlovesTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableGlovesList>* DataTable)
{
	for (auto& [key, value] : table)
	{
		std::string name = key.str().data();
		toml::array* data = nullptr;

		if (value.is_array())
			data = value.as_array();
		else
		{
			printf("[CV2Merger] [TOML] entry \"%s\" in toml is not an array", name);
			continue;
		}

		if (data->size() <= 0)
		{
			printf("[CV2Merger] [TOML] entry data for \"%s\" is empty in toml\n", name.c_str());
			continue;
		}
		if (DataTable->contains(name.c_str()))
		{
			printf("[CV2Merger] [TOML] Duplicate entry for \"%s\" found in toml\n", name.c_str());
			continue;
		}

		SDK::FCharacterCustomizeDataTableGlovesList Gloves = *new SDK::FCharacterCustomizeDataTableGlovesList();
		SDK::TSoftObjectPtr<SDK::UTexture2D> thumbnailpath;
		thumbnailpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(0)->value_or("")) };
		Gloves.Thumbnail = thumbnailpath;

		SDK::TSoftObjectPtr<SDK::USkeletalMesh> meshpath;
		meshpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(1)->value_or("")) };
		Gloves.Mesh = meshpath;

		if (data->get(2)->value_or("") != "")
		{
			Gloves.FlagCondition = *new SDK::FStoryFlagEvaluationFormula();
			Gloves.FlagCondition.EvaluationFormula = FNameHelper::FStringFromString(data->get(2)->value_or(""));
		}
		int colorIndex = 0;
		for (auto& color : *data->get(3)->as_array())
		{
			toml::array* colorArray = color.as_array();
			SDK::FCharacterCustomizeDataTableClothColor clothColor = *new SDK::FCharacterCustomizeDataTableClothColor();
			clothColor.IsSpecialColor = colorArray->get(0)->value_or(false);
			clothColor.ColorPaletteRowName = FNameHelper::FNameFromString(colorArray->get(1)->value_or(""));
			clothColor.ColorName = FNameHelper::FNameFromString(colorArray->get(2)->value_or(""));

			Gloves.Color[colorIndex] = clothColor;
			colorIndex++;
		}
		Gloves.CustomizableColorNumber = data->get(4)->value_or(0);
		Gloves.bAffectNail = data->get(5)->value_or(false);

		//Find a way to make TMaps
		Gloves.LeftFingerExposureMap = *new SDK::TMap<SDK::ECharacterCustomizeFinger, bool>();
		for (auto& finger : *data->get(6)->as_array())
		{
			toml::array* fingerArray = finger.as_array();
			SDK::ECharacterCustomizeFinger Key = magic_enum::enum_cast<SDK::ECharacterCustomizeFinger>(fingerArray->get(0)->value_or("MAX")).value_or(SDK::ECharacterCustomizeFinger::MAX);
			bool Value = fingerArray->get(1)->value_or(true);

			//Curently no way to make a new TMap in memory
		}
		Gloves.RightFingerExposureMap = *new SDK::TMap<SDK::ECharacterCustomizeFinger, bool>();
		for (auto& finger : *data->get(7)->as_array())
		{

		}

		DataTable->insert({ name, Gloves });
	}
}

void ModPatch::ProcessMaskTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableMaskList>* DataTable)
{
	for (auto& [key, value] : table)
	{
		std::string name = key.str().data();
		toml::array* data = nullptr;

		if (value.is_array())
			data = value.as_array();
		else
		{
			printf("[CV2Merger] [TOML] entry \"%s\" in toml is not an array", name);
			continue;
		}

		if (data->size() <= 0)
		{
			printf("[CV2Merger] [TOML] entry data for \"%s\" is empty in toml\n", name.c_str());
			continue;
		}
		if (DataTable->contains(name.c_str()))
		{
			printf("[CV2Merger] [TOML] Duplicate entry for \"%s\" found in toml\n", name.c_str());
			continue;
		}

		SDK::FCharacterCustomizeDataTableMaskList Mask = *new SDK::FCharacterCustomizeDataTableMaskList();
		SDK::TSoftObjectPtr<SDK::UTexture2D> thumbnailpath;
		thumbnailpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(0)->value_or("")) };
		Mask.Thumbnail = thumbnailpath;

		SDK::TSoftObjectPtr<SDK::USkeletalMesh> meshpath;
		meshpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(1)->value_or("")) };
		Mask.Mesh = meshpath;

		if (data->get(2)->value_or("") != "")
		{
			Mask.FlagCondition = *new SDK::FStoryFlagEvaluationFormula();
			Mask.FlagCondition.EvaluationFormula = FNameHelper::FStringFromString(data->get(2)->value_or(""));
		}

		int colorIndex = 0;
		for (auto& color : *data->get(3)->as_array())
		{
			toml::array* colorArray = color.as_array();
			SDK::FCharacterCustomizeDataTableClothColor clothColor = *new SDK::FCharacterCustomizeDataTableClothColor();
			clothColor.IsSpecialColor = colorArray->get(0)->value_or(false);
			clothColor.ColorPaletteRowName = FNameHelper::FNameFromString(colorArray->get(1)->value_or(""));
			clothColor.ColorName = FNameHelper::FNameFromString(colorArray->get(2)->value_or(""));

			Mask.Color[colorIndex] = clothColor;
			colorIndex++;
		}
		Mask.CustomizableColorNumber = data->get(4)->value_or(0);
		Mask.PartVisibilityInfoArray = *new SDK::TAllocatedArray<SDK::FCharacterCustomizeDataTableClothPartVisibility>(data->get(5)->as_array()->size() + 5);
		for (auto& partVisibility : *data->get(5)->as_array())
		{
			toml::array* partArray = partVisibility.as_array();
			SDK::FCharacterCustomizeDataTableClothPartVisibility partVis = *new SDK::FCharacterCustomizeDataTableClothPartVisibility();

			SDK::TSoftObjectPtr<SDK::UTexture2D> clothTexturepath;
			clothTexturepath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(0)->value_or("")) };
			partVis.ClothPartTexture = clothTexturepath;

			toml::array* HideParamsArray = partArray->get(1)->as_array();
			partVis.ClothPartHideParameters = *new SDK::TAllocatedArray<SDK::FName>(20);
			for (auto& hideParam : *HideParamsArray)
			{
				partVis.ClothPartHideParameters.Add(FNameHelper::FNameFromString(hideParam.value_or("")));
			}

			toml::array* SubstituteParamsArray = partArray->get(2)->as_array();
			partVis.ClothPartSubstituteParameters = *new SDK::TAllocatedArray<SDK::FName>(20);
			for (auto& subParam : *SubstituteParamsArray)
			{
				partVis.ClothPartSubstituteParameters.Add(FNameHelper::FNameFromString(subParam.value_or("")));
			}

			partVis.ParentIndex = partArray->get(3)->value_or(-1);
			partVis.PartType = magic_enum::enum_cast<SDK::ECharacterCustomizePartType>(partArray->get(4)->value_or("General")).value_or(SDK::ECharacterCustomizePartType::General);
			Mask.PartVisibilityInfoArray.Add(partVis);
		}
		Mask.HairAffectType = magic_enum::enum_cast<SDK::ECharacterCustomizeClothHairAffectType>(data->get(6)->value_or("General")).value_or(SDK::ECharacterCustomizeClothHairAffectType::None);

		DataTable->insert({ name, Mask });
	}
}

void ModPatch::ProcessOuterTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableOuterList>* DataTable)
{
	for (auto& [key, value] : table)
	{
		std::string name = key.str().data();
		toml::array* data = nullptr;

		if (value.is_array())
			data = value.as_array();
		else
		{
			printf("[CV2Merger] [TOML] entry \"%s\" in toml is not an array", name);
			continue;
		}

		if (data->size() <= 0)
		{
			printf("[CV2Merger] [TOML] entry data for \"%s\" is empty in toml\n", name.c_str());
			continue;
		}
		if (DataTable->contains(name.c_str()))
		{
			printf("[CV2Merger] [TOML] Duplicate entry for \"%s\" found in toml\n", name.c_str());
			continue;
		}

		SDK::FCharacterCustomizeDataTableOuterList Outer = *new SDK::FCharacterCustomizeDataTableOuterList();
		SDK::TSoftObjectPtr<SDK::UTexture2D> thumbnailpath;
		thumbnailpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(0)->value_or("")) };
		Outer.Thumbnail = thumbnailpath;

		SDK::TSoftObjectPtr<SDK::USkeletalMesh> meshpath;
		meshpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(1)->value_or("")) };
		Outer.Mesh = meshpath;

		if (data->get(2)->value_or("") != "")
		{
			Outer.FlagCondition = *new SDK::FStoryFlagEvaluationFormula();
			Outer.FlagCondition.EvaluationFormula = FNameHelper::FStringFromString(data->get(2)->value_or(""));
		}

		int colorIndex = 0;
		for (auto& color : *data->get(3)->as_array())
		{
			toml::array* colorArray = color.as_array();
			SDK::FCharacterCustomizeDataTableClothColor clothColor = *new SDK::FCharacterCustomizeDataTableClothColor();
			clothColor.IsSpecialColor = colorArray->get(0)->value_or(false);
			clothColor.ColorPaletteRowName = FNameHelper::FNameFromString(colorArray->get(1)->value_or(""));
			clothColor.ColorName = FNameHelper::FNameFromString(colorArray->get(2)->value_or(""));

			Outer.Color[colorIndex] = clothColor;
			colorIndex++;
		}

		Outer.CustomizableColorNumber = data->get(4)->value_or(0);
		Outer.HairAffectType = magic_enum::enum_cast<SDK::ECharacterCustomizeClothHairAffectType>(data->get(5)->value_or("General")).value_or(SDK::ECharacterCustomizeClothHairAffectType::None);

		DataTable->insert({ name, Outer });
	}
}

void ModPatch::ProcessAccessoryPresetTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableAccessoryPreset>* DataTable)
{
	for (auto& [key, value] : table)
	{
		std::string name = key.str().data();
		toml::array* data = nullptr;

		if (value.is_array())
			data = value.as_array();
		else
		{
			printf("[CV2Merger] [TOML] entry \"%s\" in toml is not an array", name);
			continue;
		}

		if (data->size() <= 0)
		{
			printf("[CV2Merger] [TOML] entry data for \"%s\" is empty in toml\n", name.c_str());
			continue;
		}
		if (DataTable->contains(name.c_str()))
		{
			printf("[CV2Merger] [TOML] Duplicate entry for \"%s\" found in toml\n", name.c_str());
			continue;
		}

		SDK::FCharacterCustomizeDataTableAccessoryPreset AccessoryPreset = *new SDK::FCharacterCustomizeDataTableAccessoryPreset();
		SDK::TSoftObjectPtr<SDK::UTexture2D> thumbnailpath;
		thumbnailpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(0)->value_or("")) };
		AccessoryPreset.Thumbnail = thumbnailpath;

		SDK::TSoftObjectPtr<SDK::USkeletalMesh> meshpath;
		meshpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(1)->value_or("")) };
		AccessoryPreset.Mesh = meshpath;
		AccessoryPreset.AnimClass = nullptr;
		AccessoryPreset.AttachRowName = FNameHelper::FNameFromString(data->get(3)->value_or(""));

		auto rootTrans = data->get(4)->as_array();
		AccessoryPreset.RootTransform = *new SDK::FEulerAngleTransform();
		AccessoryPreset.RootTransform.Translation = *new SDK::FVector(
			rootTrans->get(0)->as_array()->get(0)->value_or(0.0f),
			rootTrans->get(0)->as_array()->get(1)->value_or(0.0f),
			rootTrans->get(0)->as_array()->get(2)->value_or(0.0f)
		);
		AccessoryPreset.RootTransform.Rotation = *new SDK::FVector(
			rootTrans->get(1)->as_array()->get(0)->value_or(0.0f),
			rootTrans->get(1)->as_array()->get(1)->value_or(0.0f),
			rootTrans->get(1)->as_array()->get(2)->value_or(0.0f)
		);
		AccessoryPreset.RootTransform.Scale = *new SDK::FVector(
			rootTrans->get(2)->as_array()->get(0)->value_or(0.0f),
			rootTrans->get(2)->as_array()->get(1)->value_or(0.0f),
			rootTrans->get(2)->as_array()->get(2)->value_or(0.0f)
		);

		auto OrientTrans = data->get(5)->as_array();
		AccessoryPreset.OrientTransform = *new SDK::FEulerAngleTransform();
		AccessoryPreset.OrientTransform.Translation = *new SDK::FVector(
			OrientTrans->get(0)->as_array()->get(0)->value_or(0.0f),
			OrientTrans->get(0)->as_array()->get(1)->value_or(0.0f),
			OrientTrans->get(0)->as_array()->get(2)->value_or(0.0f)
		);
		AccessoryPreset.OrientTransform.Rotation = *new SDK::FVector(
			OrientTrans->get(1)->as_array()->get(0)->value_or(0.0f),
			OrientTrans->get(1)->as_array()->get(1)->value_or(0.0f),
			OrientTrans->get(1)->as_array()->get(2)->value_or(0.0f)
		);
		AccessoryPreset.OrientTransform.Scale = *new SDK::FVector(
			OrientTrans->get(2)->as_array()->get(0)->value_or(0.0f),
			OrientTrans->get(2)->as_array()->get(1)->value_or(0.0f),
			OrientTrans->get(2)->as_array()->get(2)->value_or(0.0f)
		);

		auto MeshTrans = data->get(6)->as_array();
		AccessoryPreset.MeshTransform = *new SDK::FEulerAngleTransform();
		AccessoryPreset.MeshTransform.Translation = *new SDK::FVector(
			MeshTrans->get(0)->as_array()->get(0)->value_or(0.0f),
			MeshTrans->get(0)->as_array()->get(1)->value_or(0.0f),
			MeshTrans->get(0)->as_array()->get(2)->value_or(0.0f)
		);
		AccessoryPreset.MeshTransform.Rotation = *new SDK::FVector(
			MeshTrans->get(1)->as_array()->get(0)->value_or(0.0f),
			MeshTrans->get(1)->as_array()->get(1)->value_or(0.0f),
			MeshTrans->get(1)->as_array()->get(2)->value_or(0.0f)
		);
		AccessoryPreset.MeshTransform.Scale = *new SDK::FVector(
			MeshTrans->get(2)->as_array()->get(0)->value_or(0.0f),
			MeshTrans->get(2)->as_array()->get(1)->value_or(0.0f),
			MeshTrans->get(2)->as_array()->get(2)->value_or(0.0f)
		);

		AccessoryPreset.bTransformable = data->get(7)->value_or(false);
		AccessoryPreset.bScaleNegate = data->get(8)->value_or(false);
		AccessoryPreset.MaxColor = magic_enum::enum_cast<SDK::ECharacterCustomizeAccessoryColorSlot>(data->get(9)->value_or("None")).value_or(SDK::ECharacterCustomizeAccessoryColorSlot::Slot1);
		AccessoryPreset.MaxColorNum = data->get(10)->value_or(0);

		int colorIndex = 0;
		for (auto& color : *data->get(11)->as_array())
		{
			toml::array* colorArray = color.as_array();
			SDK::FCharacterCustomizeDataTableAccessoryColor clothColor = *new SDK::FCharacterCustomizeDataTableAccessoryColor();
			clothColor.IsSpecialColor = colorArray->get(0)->value_or(false);
			clothColor.ColorPaletteRowName = FNameHelper::FNameFromString(colorArray->get(1)->value_or(""));
			clothColor.ColorName = FNameHelper::FNameFromString(colorArray->get(2)->value_or(""));

			AccessoryPreset.Color[colorIndex] = clothColor;
			colorIndex++;
		}

		AccessoryPreset.Cost = data->get(12)->value_or(0);
		if (data->get(13)->value_or("") != "")
			AccessoryPreset.FlagCondition = *new SDK::FStoryFlagEvaluationFormula();
		AccessoryPreset.FlagCondition.EvaluationFormula = FNameHelper::FStringFromString(data->get(13)->value_or(""));
		AccessoryPreset.bIsHat = data->get(14)->value_or(false);
		AccessoryPreset.bIsHat = data->get(15)->value_or(false);
		AccessoryPreset.bIsHat = data->get(16)->value_or(false);

		DataTable->insert({ name, AccessoryPreset });
	}
}

void ModPatch::ProcessHairBaseTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableHairBase>* DataTable)
{
	for (auto& [key, value] : table)
	{
		std::string name = key.str().data();
		toml::array* data = nullptr;

		if (value.is_array())
			data = value.as_array();
		else
		{
			printf("[CV2Merger] [TOML] entry \"%s\" in toml is not an array", name);
			continue;
		}

		if (data->size() <= 0)
		{
			printf("[CV2Merger] [TOML] entry data for \"%s\" is empty in toml\n", name.c_str());
			continue;
		}
		if (DataTable->contains(name.c_str()))
		{
			printf("[CV2Merger] [TOML] Duplicate entry for \"%s\" found in toml\n", name.c_str());
			continue;
		}

		SDK::FCharacterCustomizeDataTableHairBase HairBase = *new SDK::FCharacterCustomizeDataTableHairBase();
		SDK::TSoftObjectPtr<SDK::UTexture2D> thumbnailpath;
		thumbnailpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(0)->value_or("")) };
		HairBase.Thumbnail = thumbnailpath;

		SDK::TSoftObjectPtr<SDK::USkeletalMesh> meshpath;
		meshpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(1)->value_or("")) };
		HairBase.BaseMesh = meshpath;

		SDK::TSoftObjectPtr<SDK::USkeletalMesh> shadowmeshpath;
		shadowmeshpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(2)->value_or("")) };
		HairBase.BaseShadowMesh = shadowmeshpath;

		SDK::TSoftObjectPtr<SDK::UMaterialInterface> materialpath;
		materialpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(3)->value_or("")) };
		HairBase.BaseShadowMaterial = materialpath;

		for (auto& part : *data->get(4)->as_array())
		{
			HairBase.AvailableHairFrontList.Add(FNameHelper::FNameFromString(part.value_or("")));
		}
		for (auto& part : *data->get(5)->as_array())
		{
			HairBase.AvailableHairFrontSideList.Add(FNameHelper::FNameFromString(part.value_or("")));
		}
		for (auto& part : *data->get(6)->as_array())
		{
			HairBase.AvailableHairSideList.Add(FNameHelper::FNameFromString(part.value_or("")));
		}
		for (auto& part : *data->get(7)->as_array())
		{
			HairBase.AvailableHairTopList.Add(FNameHelper::FNameFromString(part.value_or("")));
		}
		for (auto& part : *data->get(8)->as_array())
		{
			HairBase.AvailableHairBackList.Add(FNameHelper::FNameFromString(part.value_or("")));
		}
		for (auto& part : *data->get(9)->as_array())
		{
			HairBase.AvailableHairOtherList.Add(FNameHelper::FNameFromString(part.value_or("")));
		}
		HairBase.bIsLengthChangeAvailable = data->get(10)->value_or(false);
		HairBase.bIsCurlAvailable = data->get(11)->value_or(false);

		DataTable->insert({ name, HairBase });
	}
}

void ModPatch::ProcessHairPartTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableHairPart>* DataTable)
{
	for (auto& [key, value] : table)
	{
		std::string name = key.str().data();
		toml::array* data = nullptr;

		if (value.is_array())
			data = value.as_array();
		else
		{
			printf("[CV2Merger] [TOML] entry \"%s\" in toml is not an array", name);
			continue;
		}

		if (data->size() <= 0)
		{
			printf("[CV2Merger] [TOML] entry data for \"%s\" is empty in toml\n", name.c_str());
			continue;
		}
		if (DataTable->contains(name.c_str()))
		{
			printf("[CV2Merger] [TOML] Duplicate entry for \"%s\" found in toml\n", name.c_str());
			continue;
		}

		SDK::FCharacterCustomizeDataTableHairPart HairPart = *new SDK::FCharacterCustomizeDataTableHairPart();
		SDK::TSoftObjectPtr<SDK::UTexture2D> thumbnailpath;
		thumbnailpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(0)->value_or("")) };
		HairPart.Thumbnail = thumbnailpath;

		SDK::TSoftObjectPtr<SDK::USkeletalMesh> meshpath;
		meshpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(1)->value_or("")) };
		HairPart.Mesh = meshpath;

		HairPart.AnimClass = nullptr;

		SDK::TSoftObjectPtr<SDK::USkeletalMesh> shadowmeshpath;
		shadowmeshpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(3)->value_or("")) };
		HairPart.ShadowMesh = shadowmeshpath;

		SDK::TSoftObjectPtr<SDK::UMaterialInterface> materialpath;
		materialpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(4)->value_or("")) };
		HairPart.ShadowMaterial = materialpath;

		HairPart.bIsLengthChangeAvailable = data->get(5)->value_or(false);
		HairPart.bIsCurlAvailable = data->get(6)->value_or(false);

		DataTable->insert({ name, HairPart });
	}
}

void ModPatch::ProcessHairSetTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableHairSet>* DataTable)
{
	for (auto& [key, value] : table)
	{
		std::string name = key.str().data();
		toml::array* data = nullptr;

		if (value.is_array())
			data = value.as_array();
		else
		{
			printf("[CV2Merger] [TOML] entry \"%s\" in toml is not an array", name);
			continue;
		}

		if (data->size() <= 0)
		{
			printf("[CV2Merger] [TOML] entry data for \"%s\" is empty in toml\n", name.c_str());
			continue;
		}
		if (DataTable->contains(name.c_str()))
		{
			printf("[CV2Merger] [TOML] Duplicate entry for \"%s\" found in toml\n", name.c_str());
			continue;
		}

		SDK::FCharacterCustomizeDataTableHairSet HairSet = *new SDK::FCharacterCustomizeDataTableHairSet();
		SDK::TSoftObjectPtr<SDK::UTexture2D> thumbnailpath;
		thumbnailpath.ObjectID = { .AssetPath = FNameHelper::FNameFromString(data->get(0)->value_or("")) };
		HairSet.Thumbnail = thumbnailpath;

		HairSet.HairBaseName = FNameHelper::FNameFromString(data->get(1)->value_or(""));
		HairSet.HairFrontName = FNameHelper::FNameFromString(data->get(2)->value_or(""));
		HairSet.HairFrontSideName = FNameHelper::FNameFromString(data->get(3)->value_or(""));
		HairSet.HairSideName = FNameHelper::FNameFromString(data->get(4)->value_or(""));
		HairSet.HairTopName = FNameHelper::FNameFromString(data->get(5)->value_or(""));
		HairSet.HairBackName = FNameHelper::FNameFromString(data->get(6)->value_or(""));
		HairSet.HairOtherName = FNameHelper::FNameFromString(data->get(7)->value_or(""));
		HairSet.bHairTaperEnabled = data->get(8)->value_or(false);
		HairSet.bHairCurlEnabled = data->get(9)->value_or(false);
		HairSet.bHairHighlightEnabled = data->get(10)->value_or(false);

		DataTable->insert({ name, HairSet });
	}
}

void ModPatch::ProcessAccessoryAttachToTable(toml::table table, std::map<std::string, SDK::FCharacterCustomizeDataTableAttachToList>* DataTable)
{
	for (auto& [key, value] : table)
	{
		std::string name = key.str().data();
		toml::array* data = nullptr;

		if (value.is_array())
			data = value.as_array();
		else
		{
			printf("[CV2Merger] [TOML] entry \"%s\" in toml is not an array", name);
			continue;
		}

		if (data->size() <= 0)
		{
			printf("[CV2Merger] [TOML] entry data for \"%s\" is empty in toml\n", name.c_str());
			continue;
		}
		if (DataTable->contains(name.c_str()))
		{
			printf("[CV2Merger] [TOML] Duplicate entry for \"%s\" found in toml\n", name.c_str());
			continue;
		}

		SDK::FCharacterCustomizeDataTableAttachToList AttachTo = *new SDK::FCharacterCustomizeDataTableAttachToList();
		AttachTo.DisplayName = FNameHelper::FTextFromString(data->get(0)->value_or(""));
		AttachTo.SocketName = FNameHelper::FNameFromString(data->get(1)->value_or(""));
		AttachTo.AttachTargetMeshType = magic_enum::enum_cast<SDK::EAttachTargetMesh>(data->get(2)->value_or("BodyBase")).value_or(SDK::EAttachTargetMesh::BodyBase);

		AttachTo.SocketTransform = *new SDK::FTransform();
		auto trans = data->get(3)->as_array();
		AttachTo.SocketTransform.Rotation.X = trans->get(0)->as_array()->get(0)->value_or(0.0);
		AttachTo.SocketTransform.Rotation.Y = trans->get(0)->as_array()->get(1)->value_or(0.0);
		AttachTo.SocketTransform.Rotation.Z = trans->get(0)->as_array()->get(2)->value_or(0.0);
		AttachTo.SocketTransform.Rotation.W = trans->get(0)->as_array()->get(3)->value_or(1.0);

		AttachTo.SocketTransform.Translation.X = trans->get(1)->as_array()->get(0)->value_or(0.0);
		AttachTo.SocketTransform.Translation.Y = trans->get(1)->as_array()->get(1)->value_or(0.0);
		AttachTo.SocketTransform.Translation.Z = trans->get(1)->as_array()->get(2)->value_or(0.0);

		AttachTo.SocketTransform.Scale3D.X = trans->get(2)->as_array()->get(0)->value_or(1.0);
		AttachTo.SocketTransform.Scale3D.X = trans->get(2)->as_array()->get(1)->value_or(1.0);
		AttachTo.SocketTransform.Scale3D.X = trans->get(2)->as_array()->get(2)->value_or(1.0);

		AttachTo.MoveType = magic_enum::enum_cast<SDK::ECharacterCustomizeAttachToMoveType>(data->get(4)->value_or("Cylindrical")).value_or(SDK::ECharacterCustomizeAttachToMoveType::Cylindrical);
		AttachTo.bKeepWearingInSpa = data->get(5)->value_or(false);

		DataTable->insert({ name, AttachTo });
	}
}

bool ModPatch::init()
{
	printf("[CV2Merger] [TOML] Processing toml patches...\n");
	printf("[CV2Merger] Current File Path %s\n", std::filesystem::current_path().generic_string().c_str());
	std::string basePath = std::filesystem::current_path().generic_string();
	std::string modsPath = "";
	if (basePath.contains("Win64"))
		modsPath = "..\\..\\Content\\Paks";
	else
		modsPath = "..\\CodeVein2\\CodeVein2\\Content\\Paks";

	if (!std::filesystem::exists(modsPath)) {
		printf("[CV2Merger] [Error] Could not find mods path\n");
		return false;
	}

	int i = 0;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(modsPath))
	{
		if (std::filesystem::path(entry.path()).extension() != ".toml")
			continue;

		printf("[CV2Merger] [TOML] processing: %S\n", entry.path().filename().c_str());
		toml::table config;
		std::string fileString = "";
		try
		{
			std::ifstream file(entry.path());
			std::string str;
			while (std::getline(file, str))
				fileString += str + "\n";
			config = toml::parse(fileString);
		}
		catch (std::exception& exception)
		{
			char text[1024];
			sprintf_s(text, "Failed to parse %S:\n%s", entry.path().c_str(), exception.what());
			MessageBoxA(nullptr, text, "CV2Merger", MB_OK | MB_ICONERROR);
		}
		toml::table* table;

		//Inner Tables
		table = config["DT_Inner_Female"].as_table();
		if (table)
			ProcessInnerTable(*table, &DT_Inner_Female);
		table = config["DT_Inner_Male"].as_table();
		if (table)
			ProcessInnerTable(*table, &DT_Inner_Male);

		//Boots Tables
		table = config["DT_Boots_Female"].as_table();
		if (table)
			ProcessBootsTable(*table, &DT_Boots_Female);
		table = config["DT_Boots_Male"].as_table();
		if (table)
			ProcessBootsTable(*table, &DT_Boots_Male);

		//Gloves Tables
		table = config["DT_Gloves_Female"].as_table();
		if (table)
			ProcessGlovesTable(*table, &DT_Gloves_Female);
		table = config["DT_Gloves_Male"].as_table();
		if (table)
			ProcessGlovesTable(*table, &DT_Gloves_Male);

		table = config["DT_Mask_Female"].as_table();
		if (table)
			ProcessMaskTable(*table, &DT_Mask_Female);
		table = config["DT_Mask_Male"].as_table();
		if (table)
			ProcessMaskTable(*table, &DT_Mask_Male);

		table = config["DT_Outer_Female"].as_table();
		if (table)
			ProcessOuterTable(*table, &DT_Outer_Female);
		table = config["DT_Outer_Male"].as_table();
		if (table)
			ProcessOuterTable(*table, &DT_Outer_Male);


		table = config["DT_AccessoryPresetDLC_Free"].as_table();
		if (table)
			ProcessAccessoryPresetTable(*table, &DT_AccessoryPresetDLC_Free);
		table = config["DT_AccessoryAttachToList"].as_table();
		if (table)
			ProcessAccessoryAttachToTable(*table, &DT_AccessoryAttachToList);

		table = config["DT_HairBaseList"].as_table();
		if (table)
			ProcessHairBaseTable(*table, &DT_HairBaseList);
		table = config["DT_HairBack"].as_table();
		if (table)
			ProcessHairPartTable(*table, &DT_HairBack);
		table = config["DT_HairFront"].as_table();
		if (table)
			ProcessHairPartTable(*table, &DT_HairFront);
		table = config["DT_HairFrontSide"].as_table();
		if (table)
			ProcessHairPartTable(*table, &DT_HairFrontSide);
		table = config["DT_HairOther"].as_table();
		if (table)
			ProcessHairPartTable(*table, &DT_HairOther);
		table = config["DT_HairSide"].as_table();
		if (table)
			ProcessHairPartTable(*table, &DT_HairSide);
		table = config["DT_HairTop"].as_table();
		if (table)
			ProcessHairPartTable(*table, &DT_HairTop);

		table = config["DT_HairSetList_Female"].as_table();
		if (table)
			ProcessHairSetTable(*table, &DT_HairSetList_Female);
		table = config["DT_HairSetList_Male"].as_table();
		if (table)
			ProcessHairSetTable(*table, &DT_HairSetList_Male);

		i++;
	}
	printf("[CV2Merger] [TOML] Finished proccessing %i toml patches\n", i);
	return true;
}