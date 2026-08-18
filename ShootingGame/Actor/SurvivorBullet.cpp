#include "SurvivorBullet.h"

using namespace Craft;

SurvivorBullet::SurvivorBullet()
	: Actor("*", Vector2::Zero, Color::Yellow)
{
	sortingOrder = 8;
	isActive = false;
	poolActive = false;
}

SurvivorBullet::SurvivorBullet(const Vector2& position, int dirX, int dirY, int damage)
	: Actor("*", position, Color::Yellow)
	, dirX(dirX), dirY(dirY), damage(damage)
	, xPos(static_cast<float>(position.x))
	, yPos(static_cast<float>(position.y))
{
	sortingOrder = 8;
}

void SurvivorBullet::Reset(const Vector2& pos, int dx, int dy, int dmg)
{
	dirX = dx;
	dirY = dy;
	damage = dmg;
	xPos = static_cast<float>(pos.x);
	yPos = static_cast<float>(pos.y);
	elapsed = 0.f;

	SetPosition(pos);
}

void SurvivorBullet::Activate()
{
	isActive = true;
	poolActive = true;
}

void SurvivorBullet::Deactivate()
{
	isActive = false;
	poolActive = false;

	// 화면 밖으로 치워서 불필요한 렌더링/충돌을 방지한다.
	SetPosition(Vector2(-999, -999));
}

void SurvivorBullet::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	elapsed += deltaTime;
	if (elapsed >= lifeTime)
	{
		// 풀링: Destroy() 대신 Deactivate()로 풀에 반환한다.
		Deactivate();
		return;
	}

	xPos += static_cast<float>(dirX) * moveSpeed * deltaTime;
	yPos += static_cast<float>(dirY) * moveSpeed * deltaTime;

	SetPosition(Vector2(static_cast<int>(xPos), static_cast<int>(yPos)));
}
