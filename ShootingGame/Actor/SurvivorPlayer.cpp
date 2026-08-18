#include "SurvivorPlayer.h"
#include "SurvivorBullet.h"
#include "SurvivorEnemy.h"
#include <Input/Input.h>
#include <Level/Level.h>
#include <State/PlayerState.h>

using namespace Craft;

static const int kDirX[8] = {  0,  1,  1,  1,  0, -1, -1, -1 };
static const int kDirY[8] = { -1, -1,  0,  1,  1,  1,  0, -1 };

static int DirToIndex(int dx, int dy)
{
	for (int i = 0; i < 8; ++i)
	{
		if (kDirX[i] == dx && kDirY[i] == dy) return i;
	}
	return 0;
}

SurvivorPlayer::SurvivorPlayer(const Vector2& position)
	: Actor("P", position, Color::Green)
	, xPos(static_cast<float>(position.x))
	, yPos(static_cast<float>(position.y))
{
	sortingOrder = 10;
	fireTimer.SetTargetTime(fireInterval);
}

void SurvivorPlayer::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	if (PlayerState::Get().GetHp() <= 0) return;

	float dx = 0.f;
	float dy = 0.f;

	if (Input::Get().GetKey(VK_RIGHT))  dx = 1.f;
	if (Input::Get().GetKey(VK_LEFT))   dx = -1.f;
	if (Input::Get().GetKey(VK_UP))     dy = -1.f;
	if (Input::Get().GetKey(VK_DOWN))   dy = 1.f;

	if (dx != 0.f || dy != 0.f)
	{
		faceDirX = static_cast<int>(dx);
		faceDirY = static_cast<int>(dy);

		xPos += dx * moveSpeed * deltaTime;
		yPos += dy * moveSpeed * deltaTime;

		if (xPos < 0.f) xPos = 0.f;
		if (yPos < 0.f) yPos = 0.f;

		SetPosition(Vector2(static_cast<int>(xPos), static_cast<int>(yPos)));
	}

	fireTimer.Tick(deltaTime);
	if (fireTimer.IsTimeOut())
	{
		fireTimer.Reset();
		Fire();
	}
}

void SurvivorPlayer::OnCollision(const std::shared_ptr<Actor>& other)
{
	// 엔진의 CollisionSystem은 Swept AABB(이전+현재 프레임 합산)로 충돌을 판정하므로,
	// 빠르게 움직이는 적이 "지나간 경로"까지 충돌 범위에 포함되어
	// 실제로 닿지 않은 적에게도 OnCollision이 호출되는 문제가 있다.
	//
	// 따라서 SurvivorLevel::ProcessGridCollision()에서 "현재 위치" 기준의
	// 정확한 AABB 충돌만 처리하고, 여기서는 아무것도 하지 않는다.
}

void SurvivorPlayer::SpawnBullet(int dx, int dy, int damage)
{
	if (bulletPool)
	{
		// 풀에서 가져온다 (힙 할당 없음).
		SurvivorBullet* b = bulletPool->Acquire();
		if (b)
		{
			b->Reset(GetPosition(), dx, dy, damage);
		}
	}
	else
	{
		// 풀이 없으면 기존 방식 (폴백).
		auto owner = GetOwner();
		if (owner)
		{
			owner->SpawnActor<SurvivorBullet>(GetPosition(), dx, dy, damage);
		}
	}
}

void SurvivorPlayer::Fire()
{
	int weaponLevel = PlayerState::Get().GetWeaponLevel();

	switch (weaponLevel)
	{
	default:
	case 1: FireSingleShot(); break;
	case 2: FireFanShot();    break;
	case 3: FireOctoShot();   break;
	}
}

void SurvivorPlayer::FireSingleShot()
{
	SpawnBullet(faceDirX, faceDirY, PlayerState::Get().GetAttackPower());
}

void SurvivorPlayer::FireFanShot()
{
	int dmg = PlayerState::Get().GetAttackPower();
	int center = DirToIndex(faceDirX, faceDirY);

	for (int offset = -1; offset <= 1; ++offset)
	{
		int idx = (center + offset + 8) % 8;
		SpawnBullet(kDirX[idx], kDirY[idx], dmg);
	}
}

void SurvivorPlayer::FireOctoShot()
{
	int dmg = PlayerState::Get().GetAttackPower();
	for (int i = 0; i < 8; ++i)
	{
		SpawnBullet(kDirX[i], kDirY[i], dmg);
	}
}
