#pragma once
#include <Actor/Actor.h>

// 풀링 대응 투사체.
// ActorPool에서 Acquire/Release로 재활용되므로 Destroy()를 호출하지 않는다.
// 대신 Deactivate()/Activate() + Reset()으로 상태를 초기화한다.
class SurvivorBullet : public Craft::Actor
{
	TYPE_DECLARATIONS(SurvivorBullet, Actor)

public:
	SurvivorBullet();
	SurvivorBullet(const Craft::Vector2& position, int dirX, int dirY, int damage = 1);

	// 풀에서 재활용할 때 호출. 위치/방향/데미지를 재설정한다.
	void Reset(const Craft::Vector2& position, int dirX, int dirY, int damage);

	inline int GetDamage() const { return damage; }

	// 풀링용 활성화/비활성화.
	void Activate();
	void Deactivate();
	inline bool IsPoolActive() const { return poolActive; }

private:
	virtual void Tick(float deltaTime) override;

private:
	int dirX = 0;
	int dirY = 0;
	int damage = 1;

	float xPos = 0.f;
	float yPos = 0.f;
	float moveSpeed = 25.f;
	float lifeTime = 2.f;
	float elapsed = 0.f;

	bool poolActive = false;
};
