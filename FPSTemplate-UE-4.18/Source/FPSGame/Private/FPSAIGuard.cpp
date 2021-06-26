// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSAIGuard.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "FPSGameMode.h"

// Sets default values
AFPSAIGuard::AFPSAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensignComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));



	if(PawnSensignComp)
		UE_LOG(LogTemp, Warning, TEXT("AFPSAIGuard"));

	SetGuardState(EAIState::Idle);

}

// Called when the game starts or when spawned
void AFPSAIGuard::BeginPlay()
{
	Super::BeginPlay();
	PawnSensignComp->OnSeePawn.AddDynamic(this, &AFPSAIGuard::OnPawnSeen);
	PawnSensignComp->OnHearNoise.AddDynamic(this, &AFPSAIGuard::OnNoiseHeard);

	OriginalRotation = GetActorRotation();

	
}

void AFPSAIGuard::OnPawnSeen(APawn * SeenPawn)
{
	if (SeenPawn == nullptr)
	{
		return;
	}

	DrawDebugSphere(GetWorld(), SeenPawn->GetActorLocation(), 32.0f, 12, FColor::Yellow, false, 10.0f);

	//AFPSExtractionZone::HandleOverlap에꺼 가져옴
	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->CompleteMission(SeenPawn, false);
	}

	SetGuardState(EAIState::Alerted);
}


//@todo 천천히회전하도록 수정하기.
void AFPSAIGuard::OnNoiseHeard(APawn * NoiseInstigator, const FVector& Location, float Volume)
{
	if (GuardState == EAIState::Alerted)
	{
		return; 
	}

	if (NoiseInstigator == nullptr)
	{
		return;
	}

	DrawDebugSphere(GetWorld(), Location, 32.0f, 12, FColor::Green, false, 10.0f);

	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();

	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewLookAt.Pitch = 0.0f; //로컬상태에서의 회전값인라서 0주는듯
	NewLookAt.Roll = 0.0f;

	SetActorRotation(NewLookAt);

	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrigenation);
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrigenation, this, &AFPSAIGuard::ResetOrientation, 3.0f);


	SetGuardState(EAIState::Suspicious);
}

void AFPSAIGuard::ResetOrientation()
{

	SetActorRotation(OriginalRotation);

	SetGuardState(EAIState::Idle);

}

void AFPSAIGuard::SetGuardState(EAIState NewState)
{
	if (GuardState == NewState)
	{
		return;
	}

	GuardState = NewState;

	OnStateChanged(GuardState);
}

// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


