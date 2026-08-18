#pragma once
#include <Actor/Actor.h>
#include <System/EnemyFSM.h>

class SurvivorEnemy : public Craft::Actor
{
	TYPE_DECLARATIONS(SurvivorEnemy, Actor)

public:
	SurvivorEnemy();
	SurvivorEnemy(const std::string& image, const Craft::Vector2& pos,
		int hp = 1, float speed = 3.f, int contactDamage = 1);

	// 풀에서 재활용할 때 호출.
	void Reset(const std::string& image, const Craft::Vector2& pos,
		int hp, float speed, int contactDamage, int goldReward);

	void TakeDamage(int amount);
	inline bool IsDead() const { return hp <= 0; }
	inline int GetContactDamage() const { return contactDamage; }
	inline int GetGoldReward() const { return goldReward; }
	inline void SetGoldReward(int value) { goldReward = value; }

	// FSM 현재 상태 (UI 디버그용).
	inline EnemyState GetFSMState() const { return fsm.currentState; }

	// 풀링용 활성화/비활성화.
	void Activate();
	void Deactivate();
	inline bool IsPoolActive() const { return poolActive; }

private:
	virtual void Tick(float deltaTime) override;
	virtual void OnCollision(const std::shared_ptr<Actor>& other) override;

private:
	int hp = 1;
	int maxHp = 1;
	int contactDamage = 1;
	int goldReward = 1;
	float moveSpeed = 3.f;
	float xPos = 0.f;
	float yPos = 0.f;

	EnemyFSM fsm;

	bool poolActive = false;
};
