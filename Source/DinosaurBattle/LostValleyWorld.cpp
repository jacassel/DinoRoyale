#include "LostValleyWorld.h"
#include "ProceduralMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include <queue>

ALostValleyWorld::ALostValleyWorld()
{
    RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Terrain=CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Terrain"));Terrain->SetupAttachment(RootComponent);
    Terrain->bUseComplexAsSimpleCollision=true;Terrain->SetCollisionObjectType(ECC_WorldStatic);
    Water=CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Creek"));Water->SetupAttachment(RootComponent);Water->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Trunks=CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("TreeTrunks"));
    Canopies=CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("TreeCanopies"));
    Rocks=CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Boulders"));
    Ferns=CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Ferns"));
    Grass=CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Grass"));
    for(auto* C:{Trunks,Canopies,Rocks,Ferns,Grass})
    {
        C->SetupAttachment(RootComponent);C->SetCanEverAffectNavigation(false);C->SetCollisionObjectType(ECC_WorldStatic);
        C->SetCollisionEnabled((C==Trunks||C==Rocks)?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);
        C->SetCullDistances(0,(C==Grass)?8000:(C==Ferns)?14000:140000);
    }
    Grass->SetCastShadow(false);Ferns->SetCastShadow(false);
    for(int32 I=0;I<4;++I)
    {
        auto* Wall=CreateDefaultSubobject<UBoxComponent>(*FString::Printf(TEXT("Boundary%d"),I));Wall->SetupAttachment(RootComponent);
        Wall->SetBoxExtent(FVector(I<2?200:30000,I<2?30000:200,6000));
        Wall->SetRelativeLocation(FVector(I==0?-29500:I==1?29500:0,I==2?-29500:I==3?29500:0,2500));
        Wall->SetCollisionProfileName(TEXT("BlockAll"));Wall->SetHiddenInGame(true);Wall->SetCanEverAffectNavigation(false);
    }
}
float ALostValleyWorld::CreekY(float X){return -5750+1300*FMath::Sin(X/6000);}
float ALostValleyWorld::PondRadius(float X,float Y){return FMath::Sqrt(FMath::Square((X-3000)/2750)+FMath::Square((Y-4500)/1900));}
bool ALostValleyWorld::WaterAt(float X,float Y,float& Surface)
{
    if(PondRadius(X,Y)<1.f){Surface=180;return true;}
    if(FMath::Abs(Y-CreekY(X))<360){Surface=HeightAt(X,CreekY(X))+80;return true;}
    Surface=-10000;return false;
}
float ALostValleyWorld::HeightAt(float X,float Y)
{
    float H=120*FMath::Sin(X/8500)*FMath::Cos(Y/10000)+52.5f*FMath::Sin((X+Y)/4500);
    H+=950*FMath::Exp(-FMath::Square((X-11750)/6750)-FMath::Square((Y-9750)/7750));
    H+=275*FMath::Exp(-FMath::Square((X+14000)/8500)-FMath::Square((Y-8000)/8500));
    float Creek=FMath::Exp(-FMath::Square((Y-CreekY(X))/1250));
    H-=210*Creek;
    float Edge=FMath::Clamp((FMath::Max(FMath::Abs(X),FMath::Abs(Y))-25500)/4250,0.f,1.f);
    H+=Edge*Edge*(1400+500*FMath::Sin(X/2350)*FMath::Cos(Y/3300));
    const float Pond=PondRadius(X,Y);
    if(Pond<1.3f)
    {
        const float Basin=180-650*(1-FMath::Square(Pond));
        float Blend=FMath::Clamp((1.3f-Pond)/.3f,0.f,1.f);Blend=Blend*Blend*(3-2*Blend);
        H=FMath::Lerp(H,Basin,Blend);
    }
    return H;
}
FVector ALostValleyWorld::GroundPoint(float X,float Y,float C) const{return FVector(X,Y,HeightAt(X,Y)+C);}
FString ALostValleyWorld::RegionName(const FVector& P)
{
    if(PondRadius(P.X,P.Y)<1.4f)return TEXT("MIRROR POND");
    if(FMath::Abs(P.Y-CreekY(P.X))<2200)return TEXT("RIBBON CREEK");
    if(P.X<-7000&&P.Y>0)return TEXT("FERNWOOD FOREST");
    if(P.X>6500&&P.Y>3000)return TEXT("REDSTONE RIDGE");
    if(P.Y<-7500&&P.X<1500)return TEXT("AMBER FEEDING GROVE");
    if(P.X>8000&&P.Y<3000)return TEXT("THE HUNTING GROUNDS");
    return TEXT("SUNGRASS PLAINS");
}
TArray<FVector> ALostValleyWorld::Landmarks(){return {FVector(0,0,0),FVector(-12500,8500,0),FVector(11500,9500,0),FVector(2500,-5750,0),FVector(-7500,-11500,0),FVector(13500,-2500,0),FVector(3000,4500,0)};}
void ALostValleyWorld::OnConstruction(const FTransform& T){Super::OnConstruction(T);Generate();}
void ALostValleyWorld::BeginPlay(){Super::BeginPlay();if(Obstacles.IsEmpty())Generate();BuildGrid();}
void ALostValleyWorld::Generate()
{
    Obstacles.Empty();FeedingSpots.Empty();for(auto* C:{Trunks,Canopies,Rocks,Ferns,Grass})C->ClearInstances();
    struct FMeshSetup{UHierarchicalInstancedStaticMeshComponent* C;const TCHAR* Path;const TCHAR* Fallback;};
    for(auto S:{FMeshSetup{Trunks,TEXT("/Game/World/SM_ConiferTrunk.SM_ConiferTrunk"),TEXT("/Engine/BasicShapes/Cylinder.Cylinder")},
        FMeshSetup{Canopies,TEXT("/Game/World/SM_ConiferCanopy.SM_ConiferCanopy"),TEXT("/Engine/BasicShapes/Cone.Cone")},
        FMeshSetup{Rocks,TEXT("/Game/World/SM_Boulder.SM_Boulder"),TEXT("/Engine/BasicShapes/Sphere.Sphere")},
        FMeshSetup{Ferns,TEXT("/Game/World/SM_Fern.SM_Fern"),TEXT("/Engine/BasicShapes/Cone.Cone")},
        FMeshSetup{Grass,TEXT("/Game/World/SM_Grass.SM_Grass"),TEXT("/Engine/BasicShapes/Cone.Cone")}})
    {
        auto* M=LoadObject<UStaticMesh>(nullptr,S.Path);if(!M)M=LoadObject<UStaticMesh>(nullptr,S.Fallback);S.C->SetStaticMesh(M);
    }
    TArray<FVector> V,N;TArray<int32> I;TArray<FVector2D> UV;TArray<FLinearColor> C;TArray<FProcMeshTangent> Tangent;
    constexpr int32 Res=192;constexpr float Size=60000.f;
    for(int32 Y=0;Y<=Res;++Y)for(int32 X=0;X<=Res;++X)
    {
        float WX=-Size/2+Size*X/Res,WY=-Size/2+Size*Y/Res,H=HeightAt(WX,WY);
        V.Add(FVector(WX,WY,H));UV.Add(FVector2D(WX/1600,WY/1600));
        FVector Normal(HeightAt(WX-50,WY)-HeightAt(WX+50,WY),HeightAt(WX,WY-50)-HeightAt(WX,WY+50),100);Normal.Normalize();N.Add(Normal);Tangent.Add(FProcMeshTangent(1,0,0));
        float Moist=FMath::Exp(-FMath::Square((WY-CreekY(WX))/1800));
        FLinearColor Col=FLinearColor::LerpUsingHSV(FLinearColor(.23f,.30f,.12f),FLinearColor(.13f,.20f,.12f),FMath::Clamp((-WX-4000)/12000,0.f,1.f));
        Col=FMath::Lerp(Col,FLinearColor(.29f,.20f,.12f),FMath::Clamp((H-550)/2000,0.f,1.f));
        Col=FMath::Lerp(Col,FLinearColor(.35f,.30f,.19f),Moist*.85f);C.Add(Col);
        if(X<Res&&Y<Res){int32 A=Y*(Res+1)+X;I.Append({A,A+Res+1,A+1,A+1,A+Res+1,A+Res+2});}
    }
    Terrain->CreateMeshSection_LinearColor(0,V,I,N,UV,C,Tangent,true);
    Terrain->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_ValleyTerrain.M_ValleyTerrain")));
    V.Empty();I.Empty();N.Empty();UV.Empty();C.Empty();Tangent.Empty();
    for(int32 K=0;K<=240;++K)
    {
        float X=-29250+K*243.75f,Y=CreekY(X);
        for(int32 Side=0;Side<2;++Side)
        {
            float YY=Y+(Side?360:-360);V.Add(FVector(X,YY,HeightAt(X,Y)+80));N.Add(FVector::UpVector);
            UV.Add(FVector2D(K*.5f,Side));C.Add(FLinearColor::White);Tangent.Add(FProcMeshTangent(1,0,0));
        }
        if(K<240){int32 A=K*2;I.Append({A,A+1,A+2,A+1,A+3,A+2});}
    }
    Water->CreateMeshSection_LinearColor(0,V,I,N,UV,C,Tangent,false);
    Water->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_ValleyWater.M_ValleyWater")));
    V.Empty();I.Empty();N.Empty();UV.Empty();C.Empty();Tangent.Empty();
    V.Add(FVector(3000,4500,180));N.Add(FVector::UpVector);UV.Add(FVector2D(5,5));C.Add(FLinearColor::White);Tangent.Add(FProcMeshTangent(1,0,0));
    for(int32 K=0;K<=96;++K)
    {
        float A=K*2*PI/96;
        V.Add(FVector(3000+2750*FMath::Cos(A),4500+1900*FMath::Sin(A),180));N.Add(FVector::UpVector);UV.Add(FVector2D((1+FMath::Cos(A))*5,(1+FMath::Sin(A))*5));C.Add(FLinearColor::White);Tangent.Add(FProcMeshTangent(1,0,0));
        if(K<96)I.Append({0,K+2,K+1});
    }
    Water->CreateMeshSection_LinearColor(1,V,I,N,UV,C,Tangent,false);Water->SetMaterial(1,Water->GetMaterial(0));
    FRandomStream R(83021);
    for(int32 K=0;K<360;++K)
    {
        float X=R.FRandRange(-27000,27000),Y=R.FRandRange(-27000,27000);
        bool Forest=X<-7000&&Y>0;
        if(PondRadius(X,Y)<1.15f)continue;
        if(!Forest&&R.FRand()>.19f)continue;
        // Broad roads link every region; central spawn remains clear.
        if(FVector2D(X,Y).Size()<2600||FMath::Abs(Y)<1400||FMath::Abs(X)<1400||FMath::Abs(Y-CreekY(X))<1800)continue;
        float Scale=R.FRandRange(.8f,1.55f),Yaw=R.FRandRange(0,360);
        FVector P=GroundPoint(X,Y,-10);Trunks->AddInstance(FTransform(FRotator(0,Yaw,0),P,FVector(Scale)));
        Canopies->AddInstance(FTransform(FRotator(0,Yaw,0),P,FVector(Scale)));
        Obstacles.Add({FVector2D(X,Y),90*Scale});
    }
    for(int32 K=0;K<65;++K)
    {
        float X=R.FRandRange(-27500,27500),Y=R.FRandRange(-27500,27500);
        if(PondRadius(X,Y)<1.15f)continue;
        if(FVector2D(X,Y).Size()<3000||FMath::Abs(Y)<1600||FMath::Abs(X)<1600||FMath::Abs(Y-CreekY(X))<1600)continue;
        float S=R.FRandRange(1.f,3.5f);if(X>7000&&Y>3500)S*=1.6f;
        FVector Scale(S,R.FRandRange(.7f,1.2f)*S,R.FRandRange(.65f,1.8f)*S);
        Rocks->AddInstance(FTransform(FRotator(0,R.FRandRange(0,360),0),GroundPoint(X,Y,-45),Scale));
        Obstacles.Add({FVector2D(X,Y),float(285*FMath::Max(Scale.X,Scale.Y))});
    }
    for(int32 K=0;K<1100;++K)
    {
        float X=R.FRandRange(-27750,27750),Y=R.FRandRange(-27750,27750);
        if(PondRadius(X,Y)<1.08f||FMath::Abs(Y-CreekY(X))<620||!IsWalkable(FVector(X,Y,0),80))continue;
        float S=R.FRandRange(.6f,1.8f);
        auto* Comp=(K%4==0||X<-7000)?Ferns:Grass;
        Comp->AddInstance(FTransform(FRotator(0,R.FRandRange(0,360),0),GroundPoint(X,Y,-2),FVector(S)));
    }
    for(FVector P:{FVector(-7500,-11500,0),FVector(-9500,-10500,0),FVector(-6000,-13000,0),FVector(4000,-3500,0),FVector(6000,-3000,0),FVector(-1750,1250,0),FVector(2000,2250,0),FVector(-13500,7000,0),FVector(13500,8000,0)})
        FeedingSpots.Add(NearestWalkable(P));
}
bool ALostValleyWorld::IsWalkable(const FVector& P,float Radius) const
{
    if(FMath::Abs(P.X)>28250||FMath::Abs(P.Y)>28250)return false;
    for(const auto& O:Obstacles)if(FVector2D::DistSquared(FVector2D(P),O.Center)<FMath::Square(O.Radius+Radius))return false;
    float DX=HeightAt(P.X+300,P.Y)-HeightAt(P.X-300,P.Y),DY=HeightAt(P.X,P.Y+300)-HeightAt(P.X,P.Y-300);
    return FMath::Sqrt(DX*DX+DY*DY)/600<.6f;
}
bool ALostValleyWorld::SegmentClear(const FVector& A,const FVector& B,float Radius) const
{
    if(!IsWalkable(B,Radius))return false;
    FVector2D Start(A),End(B),Delta=End-Start;float L=Delta.SizeSquared();
    for(const auto& O:Obstacles)
    {
        float T=L>1?FMath::Clamp(FVector2D::DotProduct(O.Center-Start,Delta)/L,0.f,1.f):0;
        if(FVector2D::DistSquared(Start+T*Delta,O.Center)<FMath::Square(O.Radius+Radius))return false;
    }
    return true;
}
FVector ALostValleyWorld::NearestWalkable(const FVector& Input) const
{
    FVector P(FMath::Clamp(Input.X,-27700.,27700.),FMath::Clamp(Input.Y,-27700.,27700.),0);
    if(IsWalkable(P,240))return GroundPoint(P.X,P.Y);
    for(int32 Ring=1;Ring<=16;++Ring)for(int32 K=0;K<16;++K)
    {
        float A=K*PI/8;FVector Q=P+FVector(FMath::Cos(A),FMath::Sin(A),0)*(Ring*500);
        if(IsWalkable(Q,260))return GroundPoint(Q.X,Q.Y);
    }
    return GroundPoint(0,0);
}
int32 ALostValleyWorld::CellIndex(const FVector& P) const
{
    int32 X=FMath::Clamp(FMath::RoundToInt((P.X+Half)/Cell),0,GridN-1),Y=FMath::Clamp(FMath::RoundToInt((P.Y+Half)/Cell),0,GridN-1);return Y*GridN+X;
}
FVector ALostValleyWorld::CellPoint(int32 I) const{return GroundPoint((I%GridN)*Cell-Half,(I/GridN)*Cell-Half);}
void ALostValleyWorld::BuildGrid()
{
    Blocked.SetNum(GridN*GridN);for(int32 I=0;I<Blocked.Num();++I)Blocked[I]=!IsWalkable(CellPoint(I),430);
}
bool ALostValleyWorld::FindPath(const FVector& Start,const FVector& End,TArray<FVector>& Out) const
{
    Out.Empty();FVector Goal=NearestWalkable(End);
    if(SegmentClear(Start,Goal,220)){Out.Add(Goal);return true;}
    if(Blocked.IsEmpty())return false;
    int32 S=CellIndex(Start),E=CellIndex(Goal);
    // Find clear grid endpoints without assuming the actor starts at a cell centre.
    auto ClearCell=[this](int32 Id){if(!Blocked[Id])return Id;for(int32 R=1;R<7;++R)for(int32 Y=-R;Y<=R;++Y)for(int32 X=-R;X<=R;++X){int32 XX=Id%GridN+X,YY=Id/GridN+Y;if(XX>=0&&YY>=0&&XX<GridN&&YY<GridN&&!Blocked[YY*GridN+XX])return YY*GridN+XX;}return Id;};
    S=ClearCell(S);E=ClearCell(E);
    struct Node{int32 Id;float F;bool operator<(const Node& O)const{return F>O.F;}};
    std::priority_queue<Node> Open;TArray<float> G;G.Init(FLT_MAX,Blocked.Num());TArray<int32> Parent;Parent.Init(-1,Blocked.Num());TArray<uint8> Closed;Closed.Init(0,Blocked.Num());
    G[S]=0;Open.push({S,0});bool Found=false;int32 Iter=0;
    while(!Open.empty()&&++Iter<30000)
    {
        int32 A=Open.top().Id;Open.pop();if(Closed[A])continue;Closed[A]=1;if(A==E){Found=true;break;}
        int32 AX=A%GridN,AY=A/GridN;
        for(int32 DY=-1;DY<=1;++DY)for(int32 DX=-1;DX<=1;++DX)
        {
            if(!DX&&!DY)continue;int32 X=AX+DX,Y=AY+DY;if(X<0||Y<0||X>=GridN||Y>=GridN)continue;
            int32 B=Y*GridN+X;if(Blocked[B]||Closed[B])continue;
            if(DX&&DY&&(Blocked[AY*GridN+X]||Blocked[Y*GridN+AX]))continue;
            float Cost=G[A]+(DX&&DY?1.414214f:1.f);
            if(Cost<G[B]){G[B]=Cost;Parent[B]=A;float HX=X-E%GridN,HY=Y-E/GridN;Open.push({B,Cost+FMath::Sqrt(HX*HX+HY*HY)});}
        }
    }
    if(!Found)return false;
    TArray<FVector> Reverse;for(int32 A=E;A!=S&&A>=0;A=Parent[A])Reverse.Add(CellPoint(A));
    Reverse.Add(CellPoint(S));FVector Last=Start;int32 K=Reverse.Num()-1;
    while(K>=0)
    {
        int32 Best=K;for(int32 J=0;J<K;++J)if(SegmentClear(Last,Reverse[J],220)){Best=J;break;}
        Out.Add(Reverse[Best]);Last=Reverse[Best];K=Best-1;
    }
    if(SegmentClear(Last,Goal,220))Out.Add(Goal);
    return !Out.IsEmpty();
}
FVector ALostValleyWorld::AvoidObstacles(const FVector& P,const FVector& Desired,float Radius) const
{
    FVector Steer=Desired;
    for(const auto& O:Obstacles)
    {
        FVector Offset(P.X-O.Center.X,P.Y-O.Center.Y,0);float Dist=Offset.Size();float Safe=O.Radius+Radius+130;
        if(Dist<Safe+350&&Dist>1)
        {
            FVector Away=Offset/Dist;float Facing=FMath::Max(0.f,-FVector::DotProduct(Desired,Away));
            Steer+=Away*FMath::Clamp((Safe+350-Dist)/350,0.f,2.f)*(Facing+.25f)*1.3f;
        }
    }
    return Steer.GetSafeNormal2D();
}
