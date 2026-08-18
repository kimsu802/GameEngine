#include "Camera.h"
#include <algorithm>
#include <utility>
#include <cassert>
#include <Engine/Engine.h>

using namespace Craft;

Camera& ::Camera::Get()
{
	static Camera instance;
	return instance;
}

void Craft::Camera::SetMapBounds(int width, int height)
{
	mapWidth = width;
	mapHeight = height;
}

void Craft::Camera::FollowTarget(const Vector2& targetposition, int viewportwidth, int viewportheight)
{
	int x = targetposition.x - (viewportwidth / 2);
	int y = targetposition.y - (viewportheight / 2);

	x = std::clamp(x, 0, max(0, mapWidth - viewportwidth));
	y = std::clamp(y, 0, max(0, mapHeight - viewportheight));

	offset = Vector2(x, y);
}
