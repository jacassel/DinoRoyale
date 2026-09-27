#include "../DinoEOSAddress.h"
#include "Misc/AutomationTest.h"
#include "NetDriverEOS.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDinoEOSAddressTest,"DinoRoyale.Online.EOSResolvedURL",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool FDinoEOSAddressTest::RunTest(const FString&)
{
    // Synthetic parser fixture, never an account used for authentication/networking.
    FInternetAddrEOS Address(FString::ChrN(32,TCHAR('1')));
    TestTrue(TEXT("Engine accepts synthetic EOS address fixture"),Address.IsValid());
    // FOnlineSessionEOS::GetConnectStringFromSessionInfo wraps ToString(false)
    // in square brackets in this installed UE version. Keep that exact shape.
    const FString Resolved=FString::Printf(TEXT("[%s]"),*Address.ToString(false));
    TestFalse(TEXT("QA1 prefix guard rejects the engine's resolved URL"),Resolved.StartsWith(TEXT("EOS:"),ESearchCase::IgnoreCase));
    const FURL Travel(nullptr,*Resolved,TRAVEL_Absolute);
    TestTrue(TEXT("Unreal accepts the resolved travel URL"),Travel.Valid!=0);
    TestEqual(TEXT("FURL extracts the EOS host consumed by NetDriverEOS"),Travel.Host,Address.ToString(false));
    TestTrue(TEXT("Corrected engine-based guard accepts bracketed EOS URL"),DinoEOSAddress::IsValid(Resolved));
    const FString WithBuild=Resolved+TEXT("?DinoBuild=2026092201");
    TestTrue(TEXT("Compatibility option preserves EOS endpoint"),DinoEOSAddress::IsValid(WithBuild));
    const FURL TravelWithBuild(nullptr,*WithBuild,TRAVEL_Absolute);
    TestEqual(TEXT("Travel preserves compatibility option"),FString(TravelWithBuild.GetOption(TEXT("DinoBuild="),TEXT(""))),FString(TEXT("2026092201")));
    TestEqual(TEXT("Travel preserves original endpoint"),TravelWithBuild.Host,Travel.Host);
    TestFalse(TEXT("Empty result rejected"),DinoEOSAddress::IsValid(TEXT("")));
    TestFalse(TEXT("IP fallback rejected"),DinoEOSAddress::IsValid(TEXT("127.0.0.1:7777")));
    TestFalse(TEXT("Bracketed IP rejected"),DinoEOSAddress::IsValid(TEXT("[127.0.0.1]:7777")));
    TestFalse(TEXT("Missing EOS identifier rejected"),DinoEOSAddress::IsValid(TEXT("[EOS:]")));
    TestFalse(TEXT("Non-EOS protocol rejected"),DinoEOSAddress::IsValid(TEXT("[OTHER:invalid]")));
    TestFalse(TEXT("Obsolete socket/channel form rejected by current engine"),DinoEOSAddress::IsValid(Resolved.LeftChop(1)+TEXT(":GameNetDriver:0]")));
    TestTrue(TEXT("Driver uses current SocketSubsystemEOS module"),GetDefault<UNetDriverEOS>()->GetClass()->GetPathName()==TEXT("/Script/SocketSubsystemEOS.NetDriverEOS"));
    AddInfo(TEXT("Reproduced: resolved shape=[EOS:<redacted>], QA1 prefix=reject, FURL+FInternetAddrEOS=accept. No live network connection was attempted by this test."));
    return true;
}
#endif
