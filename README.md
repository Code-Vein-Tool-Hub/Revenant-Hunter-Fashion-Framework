# Revenant Hunter Fashion Framework
RHFF is a framework for adding new standalone outfits and accessories in Code Vein 2

> [!IMPORTANT]
> This Plugin was made off of the latest(v2.0.3.0) of the **Steam** version of the game, other versions are not actively supported.  

## Navigation
[Installation](#installation)  
[Features](#features)  
[Credits](#credits)  

## Installation
Download the lastest release of RHFF - Revenant Hunter Fashion Framework.  
Extract the zip to "CodeVein2\CodeVein2\Binaries\Win64" where the "CodeVein2-Win64-Shipping.exe" is located.  
you should have a plugins folder and dsound.dll in that folder after installing.

As of 1.6 of RHFF, this mod now comes with a .pak to provide some extra functionality to the framework.  
the `z10_RHFF_P.pak/z10_RHFF_P.ucas/z10_RHFF_P.utoc` should be installed into your ~mods folder along with other pak mods.
full mods path is "CodeVein2\CodeVein2\Content\Paks\~mods".

## Features
Revenant Hunter Fashion Framework is a mod for patching in new outfits and accessories into Code Vein 2.  
Patching is done at runtime through toml files provided with mods and requires no extra steps for the user to install brand new outfits.  

With the [Character Creator Extended](https://www.nexusmods.com/codevein2/mods/208) mod RHFF adds multiple options for Hair Parts through it's companion pak.  
RHFF also fixes Mask hide options unlocked by Character Creator Extended.

RHFF also includes an optional fix for outfit roughness with "RoughnessFixForSpecialColors" in the config. In the Vanilla game there's a bug with color slots 1 and 2(the special colors) in which roughness is set to 1.0 always, meaning the roughness will always be maxed out when using normal colors.

#### Supported DataTables
Clothing Tables:
- DT_Inner_Female/DT_Inner_Male
- DT_Boots_Female/DT_Boots_Male
- DT_Gloves_Female/DT_Gloves_Male
- DT_Mask_Female/DT_Mask_Male
- DT_Outer_Female/DT_Outer_Male

Accessory Tables:
- DT_AccessoryPresetDLC_Free
- DT_AccessoryPresetIsHat
- DT_AccessoryAttachToList

Hair Tables:
- DT_HairBaseList
- DT_HairBack
- DT_HairFront
- DT_HairFrontSide
- DT_HairOther
- DT_HairSide
- DT_HairTop
- DT_HairSetList_Female/DT_HairSetList_Male

Eye Tables:
- DT_EyeBase
- DT_EyeDetail
- DT_EyeHighlight
- DT_Eyebrow
- DT_EyeLash

Other Tables:
- DT_Kintsugi
- DT_FacePaintDLC
- DT_Manicure

## Credits
[Dumper-7](https://github.com/Encryqed/Dumper-7) - For SDK creation  
[TOML++](https://marzer.github.io/tomlplusplus/) - Toml parsing  
[magic_enum](https://github.com/Neargye/magic_enum) - Converting strings into enums  
[Ultimate-ASI-Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) - plugin loading
