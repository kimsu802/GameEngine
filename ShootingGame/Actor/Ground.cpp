#include "Ground.h"

Ground::Ground(const Craft::Vector2& position)
	:Actor("", position)
{
	// 우선순위 설정
	// 바닥 액터는 다른 액터랑 겹쳤을 때, 덮어쓰기 되어야 함.
	sortingOrder = 0;
}
