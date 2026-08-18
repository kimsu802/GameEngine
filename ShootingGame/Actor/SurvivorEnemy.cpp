#include "SurvivorEnemy.h"
#include "SurvivorBullet.h"
#include "SurvivorPlayer.h"
#include <Level/Level.h>
#include <State/PlayerState.h>
#include <Manager/QuestManager.h>
#include <Actor/DestroyEffect.h>
#include <cmath>

using namespace Craft;

SurvivorEnemy::SurvivorEnemy()
	: Actor("x", Vector2::Zero, Color::Red)
{
	sortingOrder = 6;
	isActive = false;
	poolActive = false;
}

SurvivorEnemy::SurvivorEnemy(const std::string& image, const Vector2& pos,
	int hp, float speed, int contactDamage)
	: Actor(image, pos, Color::Red)
	, hp(hp), maxHp(hp), moveSpeed(speed), contactDamage(contactDamage)
	, xPos(static_cast<float>(pos.x))
	, yPos(static_cast<float>(pos.y))
{
	sortingOrder = 6;
}

void SurvivorEnemy::Reset(const std::string& img, const Vector2& pos,
	int newHp, float speed, int dmg, int gold)
{
	ChangeImage(img);
	hp = newHp;
	maxHp = newHp;
	moveSpeed = speed;
	contactDamage = dmg;
	goldReward = gold;
	xPos = static_cast<float>(pos.x);
	yPos = static_cast<float>(pos.y);

	SetPosition(pos);
	color = Color::Red;

	fsm.Reset();
}

void SurvivorEnemy::TakeDamage(int amount)
{
	hp -= amount;
}

void SurvivorEnemy::Activate()
{
	isActive = true;
	poolActive = true;
}

void SurvivorEnemy::Deactivate()
{
	isActive = false;
	poolActive = false;
	SetPosition(Vector2(-999, -999));
}

void SurvivorEnemy::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	auto owner = GetOwner();
	if (!owner) return;

	auto player = owner->FindActor<SurvivorPlayer>();
	if (!player) return;

	Vector2 playerPos = player->GetPosition();
	Vector2 myPos = GetPosition();

	float dx = static_cast<float>(playerPos.x - myPos.x);
	float dy = static_cast<float>(playerPos.y - myPos.y);

	// 맨해튼 거리 (sqrt 연산을 피한다).
	float manhattanDist = std::abs(dx) + std::abs(dy);

	// --- FSM 갱신 ---
	float speedMult = fsm.Update(deltaTime, manhattanDist);

	if (speedMult <= 0.f) return; // Idle 상태: 이동 안 함.

	// 거리가 매우 가까우면 떨림 방지.
	if (manhattanDist < 1.f) return;

	// 8방향 이동.
	float dirX = (dx > 0.5f) ? 1.f : ((dx < -0.5f) ? -1.f : 0.f);
	float dirY = (dy > 0.5f) ? 1.f : ((dy < -0.5f) ? -1.f : 0.f);

	float actualSpeed = moveSpeed * speedMult;
	xPos += dirX * actualSpeed * deltaTime;
	yPos += dirY * actualSpeed * deltaTime;

	SetPosition(Vector2(static_cast<int>(xPos), static_cast<int>(yPos)));

	// Rush 상태일 때 색상 변경 (시각적 피드백).
	color = (fsm.currentState == EnemyState::Rush) ? Color::BrightWhite : Color::Red;
}

void SurvivorEnemy::OnCollision(const std::shared_ptr<Actor>& other)
{
	// 엔진 CollisionSystem은 Swept AABB를 사용하므로 오탐(false positive)이 발생한다.
	// 모든 충돌 처리는 SurvivorLevel::ProcessGridCollision()에서
	// "현재 프레임 위치" 기준으로만 수행한다.
	//
	// 이 함수를 비워두면:
	//   - 빠르게 이동하는 적이 지나간 경로에 있던 총알/플레이어에
	//     잘못 충돌 판정이 나는 문제가 해결된다.
	//   - 총알-적, 플레이어-적 충돌 모두 ProcessGridCollision() 한 곳에서
	//     일관되게 처리되어 디버깅이 쉬워진다.
}
