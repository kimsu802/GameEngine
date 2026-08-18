#pragma once
#include <Core/Core.h>
#include <Math/Vector2.h>

namespace Craft
{
	class CRAFT_API Camera
	{
	public:
		static Camera& Get();

		// 카메라가 움직일 수 있는 맵 전체 크기 등록
		void SetMapBounds(int width, int height);

		// 타겟(플레이어)을 뷰포트 정중앙에 오도록 오프셋 계산
		void FollowTarget(const Vector2& targetposition, int viewportwidth, int viewportheight);

		inline const Vector2& GetOffset() const { return offset;}
		inline void SetOffset(const Vector2& value) { offset = value; }

		inline int GetMapWidth() const { return mapWidth; }
		inline int GetMapHeight() const { return mapHeight; }

	private:
		Camera() = default;
		~Camera() = default;

		Vector2 offset;

		int mapWidth = 0;
		int mapHeight = 0;
	};

}

