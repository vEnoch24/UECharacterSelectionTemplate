#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CharacterSelectSetup.generated.h"

class USkeletalMesh;
UENUM()
enum class ECharacterSelectStarterPresentation : uint8
{
    Basic UMETA(DisplayName="Basic - static artwork"),
    Animated2D UMETA(DisplayName="2D - animated portraits and text"),
    Character3D UMETA(DisplayName="3D - skeletal character previews")
};
class UWidgetBlueprint;
class UCharacterSelectLayoutData;
class UCharacterSelectRegistry;

/** Editor-only inputs for creating an independent selection setup. */
UCLASS()
class CHARACTERSELECTIONEDITOR_API UCharacterSelectSetupOptions : public UObject
{
	GENERATED_BODY()
public:
	/** Optional full screen derived from CharacterSelectWidget. Copies its widget tree and animations. Leave empty to generate editable basic screen, grid, slot and preview Blueprints without demo assets. */
	UPROPERTY(EditAnywhere, Category="1. Starter")
	TObjectPtr<UWidgetBlueprint> StarterScreen;
	/** Optional: copy navigation rules. Grid dimensions always come from the starter screen. */
	UPROPERTY(EditAnywhere, Category="1. Starter")
	TObjectPtr<UCharacterSelectLayoutData> StarterLayout;
	/** Optional: copy character references into a new registry. Character assets and art remain shared. */
	UPROPERTY(EditAnywhere, Category="1. Starter")
	TObjectPtr<UCharacterSelectRegistry> StarterRoster;
    /** Used only when Starter Screen is empty. Generates editable motion and presentation defaults without demo art. */
    UPROPERTY(EditAnywhere, Category="1. Starter", meta=(EditCondition="StarterScreen == nullptr", EditConditionHides))
    ECharacterSelectStarterPresentation Presentation = ECharacterSelectStarterPresentation::Basic;
    /** Supply your own mesh for the 3D example. Add idle/selection clips later in Studio > 3D presentation. */
    UPROPERTY(EditAnywhere, Category="1. Starter", meta=(EditCondition="StarterScreen == nullptr && Presentation == ECharacterSelectStarterPresentation::Character3D", EditConditionHides))
    TObjectPtr<USkeletalMesh> PreviewMesh;
	/** Content folder, for example /Game/MyFighter/UI. */
	UPROPERTY(EditAnywhere, Category="2. Destination")
	FString DestinationFolder = TEXT("/Game/CharacterSelect/Generated");
	/** Letters, digits and underscores only; begin with a letter. */
	UPROPERTY(EditAnywhere, Category="2. Destination")
	FString SetupName = TEXT("MySelection");
};

struct FCharacterSelectSetupResult
{
	UWidgetBlueprint* Screen = nullptr;
	UCharacterSelectLayoutData* Layout = nullptr;
	UCharacterSelectRegistry* Roster = nullptr;
	UBlueprint* Launcher = nullptr;
	// Includes partial results if asset creation fails. Nothing is saved automatically.
	TArray<UObject*> CreatedAssets;
};

namespace CharacterSelectSetup
{
	CHARACTERSELECTIONEDITOR_API bool Validate(const UCharacterSelectSetupOptions* Options, FString& Error);
	CHARACTERSELECTIONEDITOR_API FString Describe(const UCharacterSelectSetupOptions* Options);
	CHARACTERSELECTIONEDITOR_API bool Create(const UCharacterSelectSetupOptions* Options, FCharacterSelectSetupResult& Result, FString& Error);
}
