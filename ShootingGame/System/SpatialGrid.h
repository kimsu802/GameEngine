#pragma once

#include <vector>
#include <memory>
#include <Actor/Actor.h>

// ==========================================
//  SpatialGrid — 균일 그리드 공간 분할
// ==========================================
//
// # 왜 필요한가?
//   엔진 기본 CollisionSystem은 O(N²) 전수 검사를 한다.
//   적 50마리 + 총알 200발 = 250 액터이면 약 31,000회 비교가 매 프레임 발생한다.
//   SpatialGrid는 맵을 cellSize × cellSize 셀로 나누고,
//   같은 셀(또는 인접 셀)에 있는 액터끼리만 비교하므로
//   대부분의 프레임에서 비교 횟수가 O(N) 수준으로 떨어진다.
//
// # 엔진 CollisionSystem과의 관계
//   CraftEngine의 CollisionSystem은 수정 불가(DLL)이므로 그대로 둔다.
//   SurvivorLevel은 매 Tick에서 SpatialGrid를 이용해 **자체 충돌 처리**를 먼저 수행하고,
//   이미 처리된 액터는 Deactivate/Destroy 상태가 되어
//   이후 엔진 CollisionSystem의 IsActive() 검사에서 자동으로 걸러진다.
//
// # 사용법
//   grid.Clear();
//   grid.Insert(actor);          // 모든 활성 액터를 등록
//   auto neighbors = grid.Query(x, y);  // (x, y) 셀 + 인접 8셀의 액터 목록
//
class SpatialGrid
{
public:
	SpatialGrid() = default;

	// 맵 크기와 셀 크기를 지정하여 초기화한다.
	void Init(int mapWidth, int mapHeight, int cellSize = 5);

	// 매 프레임 시작 시 모든 셀을 비운다.
	void Clear();

	// 액터를 해당 위치의 셀에 등록한다.
	void Insert(const std::shared_ptr<Craft::Actor>& actor);

	// (x, y) 좌표가 속한 셀과 인접 8방향 셀에 등록된 액터를 모두 돌려준다.
	// 범위 밖 좌표는 빈 결과를 반환한다.
	std::vector<std::shared_ptr<Craft::Actor>> Query(int x, int y) const;

	// 특정 타입의 액터만 필터링해서 돌려주는 편의 함수.
	template<typename T>
	std::vector<std::shared_ptr<T>> QueryByType(int x, int y) const
	{
		auto all = Query(x, y);
		std::vector<std::shared_ptr<T>> result;
		result.reserve(all.size());

		for (const auto& actor : all)
		{
			if (actor && actor->IsActive() && actor->IsTypeOf<T>())
			{
				auto casted = Craft::Cast<T>(actor);
				if (casted) result.push_back(casted);
			}
		}

		return result;
	}

private:
	// (x, y) 월드 좌표 → 셀 인덱스.
	int GetCellIndex(int x, int y) const;

	int mapWidth = 0;
	int mapHeight = 0;
	int cellSize = 5;
	int gridCols = 0;
	int gridRows = 0;

	// 1차원 배열로 펼친 그리드. 각 셀은 해당 영역에 있는 액터 목록.
	std::vector<std::vector<std::shared_ptr<Craft::Actor>>> cells;
};
