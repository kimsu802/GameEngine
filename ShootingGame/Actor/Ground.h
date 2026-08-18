#pragma once
#include <Actor/Actor.h>

class Ground : public Craft::Actor
{
	// 커스텀 RTTI에 타입 설정.
	TYPE_DECLARATIONS(Ground, Actor)

public:
	Ground(const Craft::Vector2& position);
};


