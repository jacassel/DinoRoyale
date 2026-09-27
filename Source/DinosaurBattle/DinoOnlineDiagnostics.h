#pragma once
#include "CoreMinimal.h"
#include "DinoEOSAddress.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NetDriverEOS.h"
#include "Sockets.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemEOSTypesPublic.h"

namespace DinoOnlineDiagnostics
{
inline void Identity(const IOnlineIdentityPtr& Interface,const TCHAR* Stage)
{
    const auto ID=Interface?Interface->GetUniquePlayerId(0):nullptr;
    bool ProductUserValid=false;
    if(ID&&ID->IsValid()&&ID->GetType()==FName(TEXT("EOS")))
        ProductUserValid=FInternetAddrEOS(static_cast<const IUniqueNetIdEOS&>(*ID).GetProductUserId()).IsValid();
    UE_LOG(LogTemp,Display,TEXT("[DINO_EOS] identity stage=%s interface=%d loginStatus=%d idValid=%d productUserValid=%d"),
        Stage,Interface.IsValid(),Interface?int32(Interface->GetLoginStatus(0)):-1,ID&&ID->IsValid(),ProductUserValid);
}
inline void Session(const FOnlineSession* S,const TCHAR* Stage)
{
    if(!S){UE_LOG(LogTemp,Display,TEXT("[DINO_EOS] session stage=%s exists=0"),Stage);return;}
    const auto& Settings=S->SessionSettings;
    UE_LOG(LogTemp,Display,TEXT("[DINO_EOS] session stage=%s exists=1 ownerValid=%d infoPresent=%d infoValid=%d infoIdType=%s lan=%d lobby=%d presence=%d advertise=%d invites=%d joinInProgress=%d public=%d private=%d openPublic=%d openPrivate=%d build=%d"),
        Stage,S->OwningUserId&&S->OwningUserId->IsValid(),S->SessionInfo.IsValid(),S->SessionInfo&&S->SessionInfo->IsValid(),
        S->SessionInfo?*S->SessionInfo->GetSessionId().GetType().ToString():TEXT("none"),Settings.bIsLANMatch,Settings.bUseLobbiesIfAvailable,
        Settings.bUsesPresence,Settings.bShouldAdvertise,Settings.bAllowInvites,Settings.bAllowJoinInProgress,
        Settings.NumPublicConnections,Settings.NumPrivateConnections,S->NumOpenPublicConnections,S->NumOpenPrivateConnections,Settings.BuildUniqueId);
}
inline void NamedSession(const IOnlineSessionPtr& Interface,const TCHAR* Stage)
{
    UE_LOG(LogTemp,Display,TEXT("[DINO_EOS] namedSession stage=%s name=GameSession interface=%d state=%s"),
        Stage,Interface.IsValid(),Interface?EOnlineSessionState::ToString(Interface->GetSessionState(NAME_GameSession)):TEXT("unavailable"));
    Session(Interface?Interface->GetNamedSession(NAME_GameSession):nullptr,Stage);
}
inline void World(UWorld* World,const TCHAR* Stage,UNetDriver* DriverOverride=nullptr)
{
    auto* Driver=DriverOverride?DriverOverride:World?World->GetNetDriver():nullptr;
    auto* EOSDriver=Cast<UNetDriverEOS>(Driver);
    const bool SocketCreated=EOSDriver&&EOSDriver->GetSocket();
    // Read the driver's actual LocalAddr. FSocketEOS::GetAddress assigns via
    // the FInternetAddr base reference and does not copy the derived EOS ID.
    const auto LocalAddress=Driver?Driver->GetLocalAddr():nullptr;
    UE_LOG(LogTemp,Display,TEXT("[DINO_EOS] world stage=%s netMode=%d listen=%d map=%s urlListen=%d urlLAN=%d urlIPSockets=%d gameNetDriver=%d driver=%s eos=%d passthrough=%d socket=%d localEOSAddressValid=%d clients=%d serverConnection=%d"),
        Stage,World?int32(World->GetNetMode()):-1,World&&World->GetNetMode()==NM_ListenServer,
        World?*World->URL.Map:TEXT("none"),World&&World->URL.HasOption(TEXT("listen")),World&&World->URL.HasOption(TEXT("bIsLanMatch")),
        World&&World->URL.HasOption(TEXT("bUseIPSockets")),Driver&&Driver->NetDriverName==NAME_GameNetDriver,
        Driver?*Driver->GetClass()->GetPathName():TEXT("none"),EOSDriver!=nullptr,EOSDriver&&EOSDriver->bIsPassthrough,
        SocketCreated,LocalAddress&&LocalAddress->IsValid()&&LocalAddress->GetProtocolType()==FName(TEXT("EOS")),Driver?Driver->ClientConnections.Num():0,Driver&&Driver->ServerConnection!=nullptr);
}
inline void Resolution(bool Resolved,const FString& URL,const TCHAR* Stage)
{
    const FURL Parsed(nullptr,*URL,TRAVEL_Absolute);
    UE_LOG(LogTemp,Display,TEXT("[DINO_EOS] resolve stage=%s session=GameSession port=GamePort returned=%d shape=%s urlValid=%d eosAddressValid=%d legacyPrefixAccepted=%d"),
        Stage,Resolved,DinoEOSAddress::Shape(URL),Parsed.Valid,DinoEOSAddress::IsValid(URL),URL.StartsWith(TEXT("EOS:"),ESearchCase::IgnoreCase));
}
}
