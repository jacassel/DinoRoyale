#pragma once
#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"

// The public protocol identifier. EOS transports integer attributes as Int64.
// Keep DefaultEngine.ini's BuildIdOverride in sync (covered by automation).
namespace DinoCompatibility
{
inline constexpr int32 Build = 2026100303;
inline const FName Key(TEXT("DINO_BUILD"));
struct FReadResult
{
    int64 Value = 0;
    const TCHAR* Source = TEXT("missing DINO_BUILD");
    bool bValid = false;
    bool IsCompatible() const { return bValid && Value == Build; }
};
inline FReadResult Read(const FOnlineSessionSettings& Settings)
{
    FReadResult Result;
    const auto* Setting = Settings.Settings.Find(Key);
    if (!Setting) return Result;
    switch (Setting->Data.GetType())
    {
    case EOnlineKeyValuePairDataType::Int64:
        Setting->Data.GetValue(Result.Value);
        Result.Source = TEXT("DINO_BUILD/Int64 (EOS wire)");
        break;
    case EOnlineKeyValuePairDataType::Int32:
        {
            int32 Value = 0;
            Setting->Data.GetValue(Value);
            Result.Value = Value;
            Result.Source = TEXT("DINO_BUILD/Int32 (local settings)");
        }
        break;
    default:
        Result.Source = TEXT("DINO_BUILD/unsupported type");
        return Result;
    }
    Result.bValid = Result.Value > 0 && Result.Value <= MAX_int32;
    return Result;
}
inline void Advertise(FOnlineSessionSettings& Settings)
{
    auto& Setting = Settings.Settings.FindOrAdd(Key);
    Setting.Data.SetValue(int64(Build));
    Setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
    Settings.BuildUniqueId = Build;
}
inline FString Bucket() { return FString::Printf(TEXT("DinosaurBattle-%d"), Build); }
}
