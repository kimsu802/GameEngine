#include "EnemyBullet.h"
#include <Engine/Engine.h>

using namespace Craft;

EnemyBullet::EnemyBullet(const Craft::Vector2& position, float moveSpeed)
	:Actor("#", position, Color::Blue),moveSpeed(moveSpeed), yPosition(static_cast<float>(position.y))
{
	isCollidable = true;
}

void EnemyBullet::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	// y 위치 업데이트 (아래로 이동 처리)
	yPosition += moveSpeed * deltaTime;

	// 좌표 검사.
	if (yPosition >= Engine::Get().GetGameHeight() - 1)
	{
		Destroy();
		return;
	}

	// 위치 설정
	SetPosition(Vector2(GetPosition().x, static_cast<int>(yPosition)));
}
