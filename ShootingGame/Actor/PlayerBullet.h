#pragma once

#include <Actor/Actor.h>

class PlayerBullet : public Craft::Actor
{
	TYPE_DECLARATIONS(PlayerBullet, Actor)

public:
	PlayerBullet(const Craft::Vector2& position);

private:
	// 이벤트 함수
	virtual void Tick(float deltaTime) override;

private:
	// 이동 속도 (빠르기 - 단위 : 초)
	float moveSpeed = 30.f;

	// 위치 갱신을 할 때 사용할 변수
	float yPosition = 0.f;

};

