// Copyright (c) 2019 Jasper Drescher.

#pragma once

#include "CoreMinimal.h"
#include "UnrealNetwork.h"
#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/GameInstance.h"
#include "OGameInstance.generated.h"

UCLASS()
class UNREALONLINECPP_API UOGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UOGameInstance(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	bool HostSession(FName arg_SessionName, bool arg_bIsLAN, bool arg_bIsPresence, int32 arg_MaxNumPlayers);

	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	bool FindSessions(bool arg_bIsLAN, bool arg_bIsPresence);

	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	bool JoinSession();

	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	void DestroySession();

	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	bool SendSessionInviteToFriend(const FString& arg_FriendUniqueNetId);

	/**
	* Function to call create session.
	*
	* @param UserID: user that started the request.
	* @param SessionName: name of the session.
	* @param bIsLAN: is this is LAN Game?
	* @param bIsPresence: is the Session to create a presence session.
	* @param MaxNumPlayers: number of Maximum allowed players on this session.
	*/
	bool CreateSession(TSharedPtr<const FUniqueNetId> arg_UserId, FName arg_SessionName, bool arg_bIsLAN, bool arg_bIsPresence, int32 arg_MaxNumPlayers);

	/**
	* Joins a session via a search result.
	*
	* @param SessionName: name of session.
	* @param SearchResult: session to join.
	* @returns true if successful, false otherwise.
	*/
	bool JoinSession(TSharedPtr<const FUniqueNetId> arg_UserId, FName arg_SessionName, const FOnlineSessionSearchResult& arg_SearchResult);

	/**
	* Find an online session.
	*
	* @param UserId: user that initiated the request
	* @param bIsLAN: are we searching LAN matches
	* @param bIsPresence: are we searching presence sessions
	*/
	void FindSessions(TSharedPtr<const FUniqueNetId> arg_UserId, bool arg_bIsLAN, bool arg_bIsPresence);

private:
	/**
	* Function fired when a session create request has completed.
	*
	* @param SessionName: the name of the session this callback is for.
	* @param bWasSuccessful: true if the async action completed without error, false if there was an error.
	*/
	void OnCreateSessionComplete(FName arg_SessionName, bool arg_bWasSuccessful);

	/**
	* Function fired when a session start request has completed.
	*
	* @param SessionName: the name of the session this callback is for.
	* @param bWasSuccessful: true if the async action completed without error, false if there was an error.
	*/
	void OnStartSessionComplete(FName arg_SessionName, bool arg_bWasSuccessful);

	/**
	* Delegate fired when a session search query has completed.
	*
	* @param bWasSuccessful: true if the async action completed without error, false if there was an error.
	*/
	void OnFindSessionsComplete(bool arg_bWasSuccessful);

	/**
	* Delegate fired when a session join request has completed
	*
	* @param SessionName: the name of the session this callback is for
	* @param bWasSuccessful: true if the async action completed without error, false if there was an error
	*/
	void OnJoinSessionComplete(FName arg_SessionName, EOnJoinSessionCompleteResult::Type arg_Result);

	/**
	* Delegate fired when a destroying an online session has completed
	*
	* @param SessionName: the name of the session this callback is for
	* @param bWasSuccessful: true if the async action completed without error, false if there was an error
	*/
	void OnDestroySessionComplete(FName arg_SessionName, bool arg_bWasSuccessful);

	void OnReadFriendsListComplete(int32 arg_LocalUserNum, bool arg_bWasSuccessful, const FString& arg_FriendsListName, const FString& arg_ErrorString);

private:
	TSharedPtr<class FOnlineSessionSettings> SessionSettings;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	TArray<TSharedRef<FOnlineFriend>> FriendsList;
	FName SessionName;

	// Delegate called when session created
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;

	// Delegate called when session started 
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;

	// Delegate for searching for sessions
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;

	// Delegate for joining a session
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

	// Delegate for destroying a session
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	FOnReadFriendsListComplete OnReadFriendsListCompleteDelegate;

	// Handles to registered delegates for creation
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;

	// Handles to registered delegates for starting
	FDelegateHandle OnStartSessionCompleteDelegateHandle;

	// Handle to registered delegate for searching a session
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	// Handle to registered delegate for joining a session
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	// Handle to registered delegate for destroying a session
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
};
