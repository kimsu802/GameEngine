#pragma once
#include <Actor/Actor.h>
#include <Util/Timer.h>
#include <System/ActorPool.h>

class SurvivorBullet;

class SurvivorPlayer : public Craft::Actor
{
	TYPE_DECLARATIONS(SurvivorPlayer, Actor)

public:
	SurvivorPlayer(const Craft::Vector2& position);

	// SurvivorLevel이 생성 직후에 불릿 풀의 포인터를 넘겨준다.
	void SetBulletPool(ActorPool<SurvivorBullet>* pool) { bulletPool = pool; }

	inline int GetKillCount() const { return killCount; }
	inline void AddKillCount(int n = 1) { killCount += n; }

private:
	virtual void Tick(float deltaTime) override;
	virtual void OnCollision(const std::shared_ptr<Actor>& other) override;

	void Fire();
	void FireSingleShot();
	void FireFanShot();
	void FireOctoShot();

	// 풀에서 불릿을 꺼내 초기화하는 헬퍼.
	void SpawnBullet(int dx, int dy, int damage);

private:
	float xPos = 0.f;
	float yPos = 0.f;
	float moveSpeed = 15.f;

	int faceDirX = 0;
	int faceDirY = -1;

	Timer fireTimer;
	float fireInterval = 0.35f;

	int killCount = 0;

	// 외부에서 주입받는 불릿 풀 포인터 (소유권은 SurvivorLevel에 있다).
	ActorPool<SurvivorBullet>* bulletPool = nullptr;
};
