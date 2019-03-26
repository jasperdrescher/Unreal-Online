// Copyright (c) 2019 Jasper Drescher.

#include "OGameInstance.h"
#include "Engine/GameEngine.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/LocalPlayer.h"

UOGameInstance::UOGameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UOGameInstance::OnCreateSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UOGameInstance::OnStartSessionComplete);
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UOGameInstance::OnFindSessionsComplete);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UOGameInstance::OnJoinSessionComplete);
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UOGameInstance::OnDestroySessionComplete);
	OnReadFriendsListCompleteDelegate = FOnReadFriendsListComplete::CreateUObject(this, &UOGameInstance::OnReadFriendsListComplete);
	OnSessionUserInviteAcceptedDelegate = FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UOGameInstance::OnSessionUserInviteAccepted);
	OnSessionInviteReceivedDelegate = FOnSessionInviteReceivedDelegate::CreateUObject(this, &UOGameInstance::OnSessionInviteReceivedComplete);
}

bool UOGameInstance::HostSession(FName arg_SessionName, bool arg_bIsLAN, bool arg_bIsPresence, int32 arg_MaxNumPlayers)
{
	const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	if (LocalPlayer->IsValidLowLevelFast())
	{
		FUniqueNetIdWrapper UniqueNetIdWrapper = FUniqueNetIdWrapper(LocalPlayer->GetPreferredUniqueNetId());
		return CreateSession(UniqueNetIdWrapper.GetUniqueNetId(), arg_SessionName, arg_bIsLAN, arg_bIsPresence, arg_MaxNumPlayers);
	}

	return false;
}

bool UOGameInstance::FindSessions(bool arg_bIsLAN, bool arg_bIsPresence)
{
	const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	if (LocalPlayer->IsValidLowLevelFast())
	{
		FUniqueNetIdWrapper UniqueNetIdWrapper = FUniqueNetIdWrapper(LocalPlayer->GetPreferredUniqueNetId());
		FindSessions(UniqueNetIdWrapper.GetUniqueNetId(), arg_bIsLAN, arg_bIsPresence);

		return true;
	}

	return false;
}

bool UOGameInstance::JoinSession()
{
	const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();

	if (LocalPlayer->IsValidLowLevelFast())
	{
		// Just a SearchResult where we can save the one we want to use, for the case we find more than one!
		FOnlineSessionSearchResult SearchResult;

		// If the Array is not empty, we can go through it
		if (SessionSearch->SearchResults.Num() > 0)
		{
			for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
			{
				if (SessionSearch->SearchResults[i].Session.OwningUserId != LocalPlayer->GetPreferredUniqueNetId())
				{
					SearchResult = SessionSearch->SearchResults[i];
					FUniqueNetIdWrapper UniqueNetIdWrapper = FUniqueNetIdWrapper(LocalPlayer->GetPreferredUniqueNetId());
					return JoinSession(UniqueNetIdWrapper.GetUniqueNetId(), GameSessionName, SearchResult);

					break;
				}
			}
		}
	}

	return false;
}

void UOGameInstance::DestroySession()
{
	const IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();
	if (OnlineSubsystemInterface)
	{
		IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystemInterface->GetSessionInterface();

		if (OnlineSessionInterface.IsValid())
		{
			OnlineSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
			OnlineSessionInterface->DestroySession(SessionName);
		}
	}
}

bool UOGameInstance::SendSessionInviteToFriend(const FString& arg_FriendUniqueNetId)
{
	const IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();
	if (OnlineSubsystemInterface)
	{
		IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystemInterface->GetSessionInterface();

		if (OnlineSessionInterface.IsValid())
		{
			const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
			if (LocalPlayer->IsValidLowLevelFast())
			{
				FUniqueNetId* FriendUniqueNetId = new FUniqueNetIdString(arg_FriendUniqueNetId);
				return OnlineSessionInterface->SendSessionInviteToFriend(LocalPlayer->GetControllerId(), GameSessionName, *FriendUniqueNetId);
			}
		}
	}

	return false;
}

bool UOGameInstance::CreateSession(TSharedPtr<const FUniqueNetId> arg_UserId, FName arg_SessionName, bool arg_bIsLAN, bool arg_bIsPresence, int32 arg_MaxNumPlayers)
{
	// Get the Online Subsystem to work with
	const IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();

	if (OnlineSubsystemInterface)
	{
		// Get the Session Interface, so we can call the "CreateSession" function on it
		IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystemInterface->GetSessionInterface();

		if (OnlineSessionInterface.IsValid() && arg_UserId.IsValid())
		{
			SessionSettings = MakeShareable(new FOnlineSessionSettings());

			SessionSettings->bIsLANMatch = arg_bIsLAN;
			SessionSettings->bUsesPresence = arg_bIsPresence;
			SessionSettings->NumPublicConnections = arg_MaxNumPlayers;
			SessionSettings->NumPrivateConnections = 0;
			SessionSettings->bAllowInvites = true;
			SessionSettings->bAllowJoinInProgress = true;
			SessionSettings->bShouldAdvertise = true;
			SessionSettings->bAllowJoinViaPresence = true;
			SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

			SessionSettings->Set(SETTING_MAPNAME, FString("FirstPersonExampleMap"), EOnlineDataAdvertisementType::ViaOnlineService);

			// Set the delegate to the Handle of the SessionInterface
			OnCreateSessionCompleteDelegateHandle = OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

			// Our delegate should get called when this is complete (doesn't need to be successful!)
			return OnlineSessionInterface->CreateSession(*arg_UserId, arg_SessionName, *SessionSettings);
		}
		else
		{
			return false;
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));

		return false;
	}
}

bool UOGameInstance::JoinSession(TSharedPtr<const FUniqueNetId> arg_UserId, FName arg_SessionName, const FOnlineSessionSearchResult & arg_SearchResult)
{
	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();

	if (OnlineSubsystemInterface)
	{
		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystemInterface->GetSessionInterface();

		if (OnlineSessionInterface.IsValid() && arg_UserId.IsValid())
		{
			// Set the Handle again
			OnJoinSessionCompleteDelegateHandle = OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

			// Call the "JoinSession" Function with the passed "SearchResult". The "SessionSearch->SearchResults" can be used to get such a
			// "FOnlineSessionSearchResult" and pass it. Pretty straight forward!
			return OnlineSessionInterface->JoinSession(*arg_UserId, arg_SessionName, arg_SearchResult);
		}
	}

	return false;
}

void UOGameInstance::FindSessions(TSharedPtr<const FUniqueNetId> arg_UserId, bool arg_bIsLAN, bool arg_bIsPresence)
{
	// Get the OnlineSubsystem we want to work with
	const IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();

	if (OnlineSubsystemInterface)
	{
		// Get the SessionInterface from our OnlineSubsystem
		IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystemInterface->GetSessionInterface();

		if (OnlineSessionInterface.IsValid() && arg_UserId.IsValid())
		{
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = arg_bIsLAN;
			SessionSearch->MaxSearchResults = 20;
			SessionSearch->PingBucketSize = 50;

			if (arg_bIsPresence)
			{
				SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, arg_bIsPresence, EOnlineComparisonOp::Equals);
			}

			TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();

			// Set the Delegate to the Delegate Handle of the FindSession function
			OnFindSessionsCompleteDelegateHandle = OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

			// Finally call the SessionInterface function. The Delegate gets called once this is finished
			OnlineSessionInterface->FindSessions(*arg_UserId, SearchSettingsRef);
		}
	}
	else
	{
		// If something goes wrong, just call the Delegate Function directly with "false".
		OnFindSessionsComplete(false);
	}
}

void UOGameInstance::OnDestroySessionComplete(FName arg_SessionName, bool arg_bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnDestroySessionComplete %s, %d"), *arg_SessionName.ToString(), arg_bWasSuccessful));

	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();
	if (OnlineSubsystemInterface)
	{
		// Get the SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystemInterface->GetSessionInterface();

		if (OnlineSessionInterface.IsValid())
		{
			// Clear the Delegate
			OnlineSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

			// If it was successful, we just load another level
			if (arg_bWasSuccessful)
			{
				UGameplayStatics::OpenLevel(GetWorld(), "EntryMap", true);
			}
		}
	}
}

void UOGameInstance::OnReadFriendsListComplete(int32 arg_LocalUserNum, bool arg_bWasSuccessful, const FString& arg_FriendsListName, const FString& arg_ErrorString)
{
	if (arg_bWasSuccessful)
	{
		IOnlineFriendsPtr FriendInterface = Online::GetFriendsInterface();

		if (FriendInterface.IsValid())
		{
			FriendInterface->GetFriendsList(arg_LocalUserNum, arg_FriendsListName, FriendsList);

			if (FriendsList.Num() > 0)
			{
				const IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();
				if (OnlineSubsystemInterface)
				{
					// Get the Session Interface to call the StartSession function
					IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystemInterface->GetSessionInterface();

					for (size_t i = 0; i < FriendsList.Num(); i++)
					{
						if (FriendsList[i].Get().GetRealName().Contains("Pingu"))
						{
							FUniqueNetIdWrapper UniqueNetIdWrapper = FUniqueNetIdWrapper(FriendsList[i].Get().GetUserId());

							const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();

							if (OnlineSessionInterface->SendSessionInviteToFriend(LocalPlayer->GetControllerId(), SessionName, *UniqueNetIdWrapper.GetUniqueNetId()))
							{
								GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Invited friend %d"), arg_bWasSuccessful));
							}
						}
					}
				}
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No friends"));
			}
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Failed to read friends: %s"), *arg_ErrorString));
	}
}

void UOGameInstance::OnSessionInviteReceivedComplete(const FUniqueNetId& arg_PersonInvited, const FUniqueNetId& arg_PersonInviting, const FString& arg_AppId, const FOnlineSessionSearchResult& arg_SessionToJoin)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Recevied invite")));
}

void UOGameInstance::OnSessionUserInviteAccepted(bool arg_bWasSuccesful, const int32 arg_LocalUserNum, TSharedPtr<const FUniqueNetId> arg_NetId, const FOnlineSessionSearchResult& arg_SessionSearchResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("OnSessionUserInviteAccepted"));

	if (arg_bWasSuccesful)
	{
		if (arg_SessionSearchResult.IsValid())
		{
			IOnlineSessionPtr SessionInt = IOnlineSubsystem::Get()->GetSessionInterface();
			SessionInt->JoinSession(arg_LocalUserNum, GameSessionName, arg_SessionSearchResult);
		}
	}
}

void UOGameInstance::OnCreateSessionComplete(FName arg_SessionName, bool arg_bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnCreateSessionComplete %s, %d"), *arg_SessionName.ToString(), arg_bWasSuccessful));

	// Get the OnlineSubsystem so we can get the Session Interface
	const IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();
	if (OnlineSubsystemInterface)
	{
		// Get the Session Interface to call the StartSession function
		IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystemInterface->GetSessionInterface();

		if (OnlineSessionInterface.IsValid())
		{
			// Clear the SessionComplete delegate handle, since we finished this call
			OnlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);

			if (arg_bWasSuccessful)
			{
				// Set the StartSession delegate handle
				OnStartSessionCompleteDelegateHandle = OnlineSessionInterface->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

				SessionName = arg_SessionName;
				OnlineSessionInterface->StartSession(SessionName);
			}
		}
	}
}

void UOGameInstance::OnStartSessionComplete(FName arg_SessionName, bool arg_bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnStartSessionComplete %s, %d"), *arg_SessionName.ToString(), arg_bWasSuccessful));

	// Get the Online Subsystem so we can get the Session Interface
	const IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();
	if (OnlineSubsystemInterface)
	{
		// Get the Session Interface to clear the Delegate
		IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystemInterface->GetSessionInterface();
		if (OnlineSessionInterface.IsValid())
		{
			// Clear the delegate, since we are done with this call
			OnlineSessionInterface->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}

	// If the start was successful, we can open a NewMap if we want. Make sure to use "listen" as a parameter!
	if (arg_bWasSuccessful)
	{
		UGameplayStatics::OpenLevel(GetWorld(), "FirstPersonExampleMap", true, "listen");

		IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystemInterface->GetSessionInterface();
		const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();

		OnSessionUserInviteAcceptedDelegateHandle = OnlineSessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(OnSessionUserInviteAcceptedDelegate);

		// Friend
		IOnlineFriendsPtr FriendInterface = Online::GetFriendsInterface();
		FriendInterface->ReadFriendsList(LocalPlayer->GetControllerId(), EFriendsLists::ToString(EFriendsLists::Default), OnReadFriendsListCompleteDelegate);
	}
}

void UOGameInstance::OnFindSessionsComplete(bool arg_bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OFindSessionsComplete bSuccess: %d"), arg_bWasSuccessful));

	// Get OnlineSubsystem we want to work with
	const IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();
	if (OnlineSubsystemInterface)
	{
		// Get SessionInterface of the OnlineSubsystem
		IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystemInterface->GetSessionInterface();
		if (OnlineSessionInterface.IsValid())
		{
			// Clear the Delegate handle, since we finished this call
			OnlineSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

			// Just debugging the Number of Search results. Can be displayed in UMG or something later on
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num Search Results: %d"), SessionSearch->SearchResults.Num()));

			// If we have found at least 1 session, we just going to debug them. You could add them to a list of UMG Widgets, like it is done in the BP version!
			if (SessionSearch->SearchResults.Num() > 0)
			{
				// "SessionSearch->SearchResults" is an Array that contains all the information. You can access the Session in this and get a lot of information.
				// This can be customized later on with your own classes to add more information that can be set and displayed
				for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
				{
					// OwningUserName is just the SessionName for now. I guess you can create your own Host Settings class and GameSession Class and add a proper GameServer Name here.
					// This is something you can't do in Blueprint for example!
					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Session Number: %d | Sessionname: %s "), SearchIdx + 1, *(SessionSearch->SearchResults[SearchIdx].Session.OwningUserName)));
				}
			}
		}
	}
}

void UOGameInstance::OnJoinSessionComplete(FName arg_SessionName, EOnJoinSessionCompleteResult::Type arg_Result)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnJoinSessionComplete %s, %d"), *arg_SessionName.ToString(), static_cast<int32>(arg_Result)));

	// Get the OnlineSubsystem we want to work with
	const IOnlineSubsystem* OnlineSubsystemInterface = IOnlineSubsystem::Get();
	if (OnlineSubsystemInterface)
	{
		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystemInterface->GetSessionInterface();

		if (OnlineSessionInterface.IsValid())
		{
			// Clear the Delegate again
			OnlineSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

			// Get the first local PlayerController, so we can call "ClientTravel" to get to the Server Map
			// This is something the Blueprint Node "Join Session" does automatically!
			APlayerController* PlayerController = GetFirstLocalPlayerController();

			// We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
			// String for us by giving him the SessionName and an empty String. We want to do this, because
			// Every OnlineSubsystem uses different TravelURLs
			FString TravelURL;

			if (PlayerController && OnlineSessionInterface->GetResolvedConnectString(arg_SessionName, TravelURL))
			{
				// Finally call the ClienTravel. If you want, you could print the TravelURL to see
				// how it really looks like
				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}
