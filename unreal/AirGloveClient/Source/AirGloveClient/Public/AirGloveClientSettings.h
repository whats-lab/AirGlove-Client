#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AirGloveClientSettings.generated.h"

UENUM()
enum class EAirGloveClientLiveLinkConvention : uint8
{
	OpenXR UMETA(DisplayName = "OpenXR (as received, m)"),
	MirroredY UMETA(DisplayName = "Imported Meta hand FBX: (x,-y,z) and (x,-y,z,-w), m"),
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "AirGloveClient"))
class AIRGLOVECLIENT_API UAirGloveClientSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Connection")
	bool bAutoStart = true;

	UPROPERTY(Config, EditAnywhere, Category = "Connection")
	FString ListenAddress = TEXT("127.0.0.1");

	UPROPERTY(Config, EditAnywhere, Category = "Connection")
	int32 ListenPort = 4040;

	UPROPERTY(Config, EditAnywhere, Category = "Connection")
	FString SpineAddress = TEXT("127.0.0.1");

	UPROPERTY(Config, EditAnywhere, Category = "Connection")
	int32 SpinePort = 4042;

	UPROPERTY(Config, EditAnywhere, Category = "Connection", meta = (ClampMin = "0.01"))
	float StaleSeconds = 0.2f;

	UPROPERTY(Config, EditAnywhere, Category = "Live Link")
	bool bLiveLink = true;

	UPROPERTY(Config, EditAnywhere, Category = "Live Link")
	EAirGloveClientLiveLinkConvention LiveLinkConvention = EAirGloveClientLiveLinkConvention::MirroredY;

	UPROPERTY(Config, EditAnywhere, Category = "Live Link")
	FName LeftSubjectName = TEXT("AirGloveClient_Left");

	UPROPERTY(Config, EditAnywhere, Category = "Live Link")
	FName RightSubjectName = TEXT("AirGloveClient_Right");

	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
};
