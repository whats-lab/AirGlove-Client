#include "AirGloveClientRemap.h"

FName UAirGloveClientRemap::GetRemappedBoneName_Implementation(FName BoneName) const
{
	if (BoneName == TEXT("Wrist"))
	{
		return NAME_None;
	}
	return FName(BonePrefix + BoneName.ToString());
}
