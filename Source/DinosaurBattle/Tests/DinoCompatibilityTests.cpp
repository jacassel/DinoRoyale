#include "../DinoCompatibility.h"
#include "Misc/AutomationTest.h"
#include "OnlineSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDinoCompatibilityTest,"DinoRoyale.Online.Compatibility",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FDinoCompatibilityTest::RunTest(const FString&)
{
    FOnlineSessionSettings Host;
    DinoCompatibility::Advertise(Host);
    TestTrue(TEXT("Host metadata has the canonical value"),DinoCompatibility::Read(Host).IsCompatible());
    TestEqual(TEXT("Host BuildUniqueId agrees"),Host.BuildUniqueId,DinoCompatibility::Build);
    TestEqual(TEXT("Packaged OSS override agrees (EOS overwrites on create)"),GetBuildUniqueId(),DinoCompatibility::Build);
    TestEqual(TEXT("Metadata is advertised"),int32(Host.Settings[DinoCompatibility::Key].AdvertisementType),int32(EOnlineDataAdvertisementType::ViaOnlineService));

    // Reproduce UE's actual typed getter failure using the Int64 result emitted
    // by FOnlineSessionEOS::CopyLobbyAttributes, not a stand-in JSON parser.
    int32 OldRead=-1;
    TestTrue(TEXT("Old Get reports the key exists"),Host.Get(DinoCompatibility::Key,OldRead));
    TestEqual(TEXT("Old Int32 getter reads EOS Int64 as zero: original defect"),OldRead,0);
    AddInfo(FString::Printf(TEXT("Reproduction: host wire=%lld; old guest read=%d; corrected guest read=%lld"),
        int64(DinoCompatibility::Build),OldRead,DinoCompatibility::Read(Host).Value));
    auto& Value=Host.Settings[DinoCompatibility::Key].Data;
    Value.SetValue(int32(DinoCompatibility::Build));
    TestTrue(TEXT("Local Int32 source also accepted"),DinoCompatibility::Read(Host).IsCompatible());
    Value.SetValue(int64(DinoCompatibility::Build));
    TestTrue(TEXT("EOS Int64 accepted for browser and invite gate"),DinoCompatibility::Read(Host).IsCompatible());
    Value.SetValue(int64(DinoCompatibility::Build)-1);
    TestFalse(TEXT("Older build rejected"),DinoCompatibility::Read(Host).IsCompatible());
    Value.SetValue(int64(DinoCompatibility::Build)+1);
    TestFalse(TEXT("Newer incompatible build rejected"),DinoCompatibility::Read(Host).IsCompatible());
    Value.SetValue(int64(DinoCompatibility::Build)+(int64(1)<<32));
    TestFalse(TEXT("Overflow cannot truncate into current ID"),DinoCompatibility::Read(Host).IsCompatible());
    Value.SetValue(int64(-1));
    TestFalse(TEXT("Negative value is invalid"),DinoCompatibility::Read(Host).bValid);
    Value.SetValue(int64(0));
    TestFalse(TEXT("Zero is invalid"),DinoCompatibility::Read(Host).bValid);
    Value.SetValue(FString::FromInt(DinoCompatibility::Build));
    TestFalse(TEXT("String cannot bypass typed numeric metadata"),DinoCompatibility::Read(Host).bValid);
    Value.SetValue(double(DinoCompatibility::Build));
    TestFalse(TEXT("Float cannot bypass typed numeric metadata"),DinoCompatibility::Read(Host).bValid);
    Value.SetValue(true);
    TestFalse(TEXT("Boolean is invalid"),DinoCompatibility::Read(Host).bValid);
    Host.Settings.Remove(DinoCompatibility::Key);
    TestFalse(TEXT("Missing field rejected even when BuildUniqueId matches"),DinoCompatibility::Read(Host).IsCompatible());
    return true;
}
#endif
