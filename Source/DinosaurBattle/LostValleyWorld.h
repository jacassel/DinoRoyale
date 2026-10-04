#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LostValleyWorld.generated.h"
class UProceduralMeshComponent;
class UHierarchicalInstancedStaticMeshComponent;

struct FValleyObstacle { FVector2D Center; float Radius; bool bTree=false; };

/** Seeded terrain and a clearance-aware A* grid share the same obstacle definitions. */
UCLASS()
class DINOSAURBATTLE_API ALostValleyWorld : public AActor
{
    GENERATED_BODY()
public:
    ALostValleyWorld();
    bool bPerformanceMap=false;
    void SetPerformanceMap(bool Enabled);
    static void EnsureLocalScene(UWorld* World);
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    UPROPERTY(VisibleAnywhere) UProceduralMeshComponent* Terrain;
    UPROPERTY(VisibleAnywhere) UProceduralMeshComponent* Water;
    UPROPERTY(VisibleAnywhere) UHierarchicalInstancedStaticMeshComponent* Trunks;
    UPROPERTY(VisibleAnywhere) UHierarchicalInstancedStaticMeshComponent* Canopies;
    UPROPERTY(VisibleAnywhere) UHierarchicalInstancedStaticMeshComponent* Rocks;
    UPROPERTY(VisibleAnywhere) UHierarchicalInstancedStaticMeshComponent* Ferns;
    UPROPERTY(VisibleAnywhere) UHierarchicalInstancedStaticMeshComponent* Grass;
    UPROPERTY(VisibleAnywhere) UHierarchicalInstancedStaticMeshComponent* BankStones;
    static float HeightAt(float X,float Y);
    static float CreekY(float X);
    static float PondRadius(float X,float Y);
    static bool WaterAt(float X,float Y,float& Surface);
    static FString RegionName(const FVector& P);
    static TArray<FVector> Landmarks();
    FVector GroundPoint(float X,float Y,float Clearance=0) const;
    bool IsWalkable(const FVector& P,float Radius=180) const;
    bool SegmentClear(const FVector& A,const FVector& B,float Radius=180) const;
    bool FindPath(const FVector& Start,const FVector& End,TArray<FVector>& Out) const;
    FVector NearestWalkable(const FVector& P) const;
    FVector AvoidObstacles(const FVector& Position,const FVector& Desired,float Radius) const;
    TArray<FValleyObstacle> Obstacles;
    TArray<FVector> FeedingSpots;
    TArray<FVector> FoodSpawnPoints;
    int32 PathRequests=0,PathFailures=0;
private:
    void Generate();
    void BuildGrid();
    TArray<uint8> Blocked;
    static constexpr int32 GridN=97;
    static constexpr float Cell=600.f;
    static constexpr float Half=28800.f;
    int32 CellIndex(const FVector& P) const;
    FVector CellPoint(int32 I) const;
};
