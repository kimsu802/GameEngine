#include "Wall.h"

using namespace Craft;

Wall::Wall(const Craft::Vector2& position)
	:Actor("#", position, Color::White)
{
	// 그리기 우선순위 지정.
	sortingOrder = 0;
}
