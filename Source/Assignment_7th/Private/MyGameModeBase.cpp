// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"
#include "MyPlayerController.h"
#include "MyAirplane.h"

AMyGameModeBase::AMyGameModeBase()
{
	DefaultPawnClass = AMyAirplane::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
}
