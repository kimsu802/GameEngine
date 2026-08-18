#pragma once
#include <Level/Level.h>
#include <Util/Timer.h>
#include <System/ActorPool.h>
#include <System/SpatialGrid.h>
#include <System/WaveManager.h>
#include <memory>

class SurvivorPlayer;
class SurvivorBullet;
class SurvivorEnemy;

class SurvivorLevel : public Craft::Level
{
	TYPE_DECLARATIONS(SurvivorLevel, Level)

public:
	SurvivorLevel();
	~SurvivorLevel() = default;

private:
	virtual void OnInitialized() override;
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;

	void SpawnEnemy();
	Craft::Vector2 GetRandomSpawnPosition() const;

	// SpatialGrid 기반 충돌 처리. 엔진 CollisionSystem보다 먼저 실행된다.
	void ProcessGridCollision();

	void DrawDungeonStatus();

private:
	std::shared_ptr<SurvivorPlayer> player;

	// --- 오브젝트 풀 ---
	ActorPool<SurvivorBullet> bulletPool;
	ActorPool<SurvivorEnemy> enemyPool;

	// --- 공간 분할 ---
	SpatialGrid grid;

	// --- 웨이브 매니저 ---
	WaveManager waveManager;
	Timer spawnTimer;

	float elapsedTime = 0.f;

	bool isGameOver = false;
	float gameOverTimer = 0.f;
	float gameOverDelay = 3.f;

	int mapWidth = 200;
	int mapHeight = 100;
};
