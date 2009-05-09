#include "Body.h"
#include "..\TechnoType\Body.h"

#include <YRMath.h>
#include <TacticalClass.h>

hash_SpotlightExt TechnoExt::SpotlightExt;
BuildingLightClass * TechnoExt::ActiveBuildingLight = NULL;

// just in case
DEFINE_HOOK(420F40, Spotlights_UpdateFoo, 6)
{
	return Game::CurrentFrameRate >= Game::GetMinFrameRate()
	  ? 0
	  : 0x421346;
}

// bugfix #182: Spotlights cause an IE
DEFINE_HOOK(5F5155, ObjectClass_Put, 6)
{
	return R->get_EAX()
	 ? 0
	 : 0x5F5210;
}

DEFINE_HOOK(6F6D04, TechnoClass_Put, 6)
{
	GET(TechnoClass *, T, ESI);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	if(pTypeData->Is_Spotlighted) {
		new BuildingLightClass(T);
	}
	return 0;
}

DEFINE_HOOK(435820, BuildingLightClass_CTOR, 6)
{
	GET_STACK(TechnoClass *, T, 0x4);
	GET(BuildingLightClass *, BL, ECX);

	if(TechnoExt::SpotlightExt.find(T) != TechnoExt::SpotlightExt.end()) {
		TechnoExt::SpotlightExt.erase(T);
	}
	TechnoExt::SpotlightExt[T] = BL;
	return 0;
}

DEFINE_HOOK(4370C0, BuildingLightClass_SDDTOR, A)
{
	GET(BuildingLightClass *, BL, ECX);
	TechnoClass *T = BL->OwnerObject;
	if(TechnoExt::SpotlightExt.find(T) != TechnoExt::SpotlightExt.end()) {
		TechnoExt::SpotlightExt.erase(T);
	}
	return 0;
}

DEFINE_HOOK(70FBE3, TechnoClass_Activate, 5)
{
	GET(TechnoClass *, T, ECX);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	if(pTypeData->Is_Spotlighted) {
		hash_SpotlightExt::iterator i = TechnoExt::SpotlightExt.find(T);
		if(i != TechnoExt::SpotlightExt.end()) {
//			TechnoExt::SpotlightExt.erase(i);
			delete i->second;
		}
		new BuildingLightClass(T);
	}
	return 0;
}

DEFINE_HOOK(70FC97, TechnoClass_Deactivate, 6)
{
	GET(TechnoClass *, T, ESI);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	if(pTypeData->Is_Spotlighted) {
		hash_SpotlightExt::iterator i = TechnoExt::SpotlightExt.find(T);
		if(i != TechnoExt::SpotlightExt.end()) {
//			TechnoExt::SpotlightExt.erase(i);
			delete i->second;
		}
	}
	return 0;
}

DEFINE_HOOK(435C08, BuildingLightClass_Draw_ForceType, 5)
{
	return 0x435C16;
}

DEFINE_HOOK(435C32, BuildingLightClass_Draw_PowerOnline, A)
{
	GET(TechnoClass *, T, EDI);
	return (T->WhatAmI() != abs_Building || (T->IsPowerOnline() && !reinterpret_cast<BuildingClass *>(T)->IsFogged))
	  ? 0x435C52
	  : 0x4361BC;
}

DEFINE_HOOK(436459, BuildingLightClass_Update, 6)
{
	static const double Facing2Rad = (2 * 3.14) / 0xFFFF;
	GET(BuildingLightClass *, BL, EDI);
	TechnoClass *Owner = BL->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	if(Owner && Owner->WhatAmI() != abs_Building) {
		CoordStruct Loc = *Owner->get_Location();
		DWORD Facing;
		switch(pTypeData->Spot_AttachedTo) {
			case TechnoTypeExt::sa_Barrel:
				Owner->get_BarrelFacing()->GetFacing(&Facing);
				break;
			case TechnoTypeExt::sa_Turret:
				Owner->get_TurretFacing()->GetFacing(&Facing);
				break;
			default:
				Owner->get_Facing()->GetFacing(&Facing);
		}
		WORD F = (WORD)Facing;

		double Angle = Facing2Rad * F;
		Loc.Y -= pTypeData->Spot_Distance * Math::cos(Angle);
		Loc.X += pTypeData->Spot_Distance * Math::sin(Angle);

		BL->set_field_B8(&Loc);
		BL->set_field_C4(&Loc);
//		double zer0 = 0.0;
		__asm { fldz }
		return 0x43645F;
	}
	return 0;
}

/*
FINE_HOOK(436619, BuildingLightClass_Update_Typeof, 6)
{
	return 0x436629;
}

FINE_HOOK(436664, BuildingLightClass_Update_Power, A)
{
	GET(TechnoClass *, T, EDI);
	return (T->WhatAmI() != abs_Building || (T->IsPowerOnline() && !T->IsFogged))
	  ? 0x436676
	  : 0x4368EB;
//	return ;
}
*/

DEFINE_HOOK(435BE0, BuildingLightClass_Draw_Start, 6)
{
	GET(BuildingLightClass *, BL, ECX);
	TechnoExt::ActiveBuildingLight = BL;
	return 0;
}

DEFINE_HOOK(436072, BuildingLightClass_Draw_430, 6)
{
	TechnoClass *Owner = TechnoExt::ActiveBuildingLight->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	R->set_EAX((pTypeData ? pTypeData->Spot_Height : 250) + 180);
	return 0x436078;
}

DEFINE_HOOK(4360D8, BuildingLightClass_Draw_400, 6)
{
	TechnoClass *Owner = TechnoExt::ActiveBuildingLight->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	R->set_ECX((pTypeData ? pTypeData->Spot_Height : 250) + 150);
	return 0x4360DE;
}

DEFINE_HOOK(4360FF, BuildingLightClass_Draw_250, 6)
{
	TechnoClass *Owner = TechnoExt::ActiveBuildingLight->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	R->set_ECX(pTypeData ? pTypeData->Spot_Height : 250);
	return 0x436105;
}

//5FF278
DEFINE_HOOK(453CD3, SpotlightClass_CTOR, 6)
{
	GET_STACK(SpotlightClass *, Spot, 0x14);
	GET(BuildingLightClass *, BL, ESI);

	TechnoClass *Owner = BL->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());

	eSpotlightFlags Flags = 0;
	if(pTypeData->Spot_Reverse) {
		Flags |= sf_NoColor;
	}
	if(pTypeData->Spot_DisableR) {
		Flags |= sf_NoRed;
	}
	if(pTypeData->Spot_DisableG) {
		Flags |= sf_NoGreen;
	}
	if(pTypeData->Spot_DisableB) {
		Flags |= sf_NoBlue;
	}

	Spot->DisableFlags = Flags;

	return 0;
}
