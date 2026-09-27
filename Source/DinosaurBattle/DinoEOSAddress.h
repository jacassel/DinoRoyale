#pragma once
#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "InternetAddrEOS.h"

namespace DinoEOSAddress
{
// OSS EOS returns a travel URL ([EOS:PUID]), not a bare socket address.
// Parse with the same FURL used by ClientTravel, then let the engine's EOS
// address implementation parse the host. EOS considers any FromString handle
// valid; this checks transport shape, not authentication. The URL must come
// from the joined named session. Never construct a peer ID or URL.
inline bool IsValid(const FString& ResolvedURL)
{
    if(ResolvedURL.IsEmpty())return false;
    const FURL TravelURL(nullptr,*ResolvedURL,TRAVEL_Absolute);
    if(!TravelURL.Valid||TravelURL.Host.Len()<=4)return false;
    FInternetAddrEOS Address;bool Parsed=false;
    Address.SetIp(*TravelURL.Host,Parsed);
    return Parsed&&Address.IsValid();
}
inline const TCHAR* Shape(const FString& URL)
{
    return URL.IsEmpty()?TEXT("empty"):URL.StartsWith(TEXT("[EOS:"))?TEXT("[EOS:<redacted>]"):
        URL.StartsWith(TEXT("EOS:"))?TEXT("EOS:<redacted>"):TEXT("non-EOS/redacted");
}
}
