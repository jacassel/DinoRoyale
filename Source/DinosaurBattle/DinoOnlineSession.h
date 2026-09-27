#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSessionSettings.h"
#include "Containers/Ticker.h"
#include "Engine/EngineBaseTypes.h"
#include "DinoCompatibility.h"
#include "DinoOnlineSession.generated.h"
class IOnlineSubsystem;

/** Provider boundary. Gameplay uses Unreal replication, never provider SDK IDs. */
UCLASS()
class DINOSAURBATTLE_API UDinoOnlineSession : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    static constexpr int32 BuildVersion=DinoCompatibility::Build;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    void SignIn();
    void Host(bool Teams,int32 Capacity,bool Bots,bool Public);
    void Search();
    void Join(int32 Index);
    void Leave(const FString& Reason=TEXT("Left multiplayer."));
    void InviteFriends();
    void UpdateHostedSettings(bool Teams,int32 Capacity,bool Bots);
    bool IsSignedIn() const;
    bool ValidateIdentity(const FUniqueNetIdRepl& ID) const;
    FString Nickname() const;
    void EnterNetworkWorld();
    bool bShowMenuOnReturn=false;
    bool bBusy=false,bInSession=false,bHosting=false;
    FString Status=TEXT("Sign in to Epic to play online.");
    FName Provider=TEXT("EOS");
    TArray<FOnlineSessionSearchResult> Results;
    FString ResultLabel(int32 Index) const;
private:
    IOnlineSubsystem* Online=nullptr;
    IOnlineSessionPtr Sessions;
    IOnlineIdentityPtr Identity;
    TSharedPtr<FOnlineSessionSearch> CurrentSearch;
    FDelegateHandle LoginHandle,CreateHandle,FindHandle,JoinHandle,DestroyHandle,InviteHandle,NetworkHandle,TravelHandle;
    FTSTicker::FDelegateHandle TickHandle;
    enum class EOperation:uint8 {None,Login,Create,Search,Join,Destroy};
    EOperation Operation=EOperation::None;
    double Deadline=0;
    bool bLeaving=false,bTimedOut=false,bPublic=true,bConnected=false;
    FString PendingOptions;
    TWeakObjectPtr<class UNetDriver> LastPendingDriver;
    bool EnsureProvider();
    void BeginOperation(EOperation Op,const FString& Message);
    bool Tick(float Dt);
    void ReturnToMenu();
    void JoinResult(const FOnlineSessionSearchResult& Result);
    void OnLogin(int32 User,bool Success,const FUniqueNetId& ID,const FString& Error);
    void OnCreate(FName Name,bool Success);
    void OnFind(bool Success);
    void OnJoin(FName Name,EOnJoinSessionCompleteResult::Type Result);
    void OnDestroy(FName Name,bool Success);
    void OnInvite(bool Success,int32 User,FUniqueNetIdPtr ID,const FOnlineSessionSearchResult& Result);
    void OnNetworkFailure(UWorld* World,UNetDriver* Driver,ENetworkFailure::Type Type,const FString& Error);
    void OnTravelFailure(UWorld* World,ETravelFailure::Type Type,const FString& Error);
};
