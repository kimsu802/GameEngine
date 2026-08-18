#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <Actor/Actor.h>
#include <Level/Level.h>

// ==========================================
//  ActorPool<T> — 범용 액터 오브젝트 풀
// ==========================================
//
// # 왜 풀링이 필요한가?
//   SurvivorLevel에서 무기 Lv3(8방향)이면 초당 약 24회 SpawnActor/Destroy가 발생한다.
//   shared_ptr의 make_shared + 해제는 매번 힙 할당/해제를 동반하고,
//   Level::actorList에 push/erase가 반복되면서 메모리 단편화와 캐시 미스를 유발한다.
//
//   풀링은 "미리 만들어둔 객체를 비활성화했다가 필요할 때 재활성화"하는 방식으로,
//   런타임 할당을 최초 1회로 줄여 이 문제를 해결한다.
//
// # 사용법
//   // 풀 생성 (bulletPool이 Level::actorList에 직접 추가까지 해줌).
//   ActorPool<SurvivorBullet> bulletPool;
//   bulletPool.Prewarm(level, 200);
//
//   // 풀에서 가져오기 (부족하면 내부에서 자동 확장).
//   SurvivorBullet* b = bulletPool.Acquire(level);
//   b->Reset(pos, dirX, dirY, damage);   // T에 Reset()을 구현해야 함.
//
//   // 반환은 T::Tick()에서 lifeTime 초과 시 Release()를 호출.
//   bulletPool.Release(b);
//
// # 설계 결정
//   - shared_ptr 대신 raw pointer로 Acquire/Release를 돌린다.
//     풀이 shared_ptr의 소유권을 유지하고, 사용자는 raw pointer로만 접근한다.
//     이렇게 하면 shared_ptr 참조 카운트 증감이 Acquire/Release마다 발생하지 않는다.
//   - Actor::isActive 플래그를 재활용한다. 비활성화된 액터는
//     Level::Tick/Draw/CollisionSystem 모두에서 건너뛰므로 추가 비용이 0이다.
//   - Level::ProcessAddAndDestroys()의 hasExpired 기반 삭제와 충돌하지 않도록,
//     풀에 속한 액터는 Destroy() 대신 반드시 Release()를 사용해야 한다.
//

using namespace Craft;

template<typename T>
class ActorPool
{

public:
	ActorPool() = default;
	~ActorPool() = default;

	// 초기 풀 크기만큼 액터를 만들어 Level::actorList에 등록한다.
	// 모두 비활성화(Deactivate) 상태로 시작한다.
	template<typename... Args>
	void Prewarm(const std::shared_ptr<Craft::Level>& level, int count, Args&&... defaultArgs)
	{
		ownerLevel = level;

		pool.reserve(count);
		for (int i = 0; i < count; ++i)
		{
			auto actor = level->SpawnActor<T>(std::forward<Args>(defaultArgs)...);
			actor->Deactivate();
			pool.push_back(actor);
		}
	}

	// 비활성화된 액터 하나를 골라 반환한다.
	// 여유가 없으면 새로 만들어 풀에 추가한다(자동 확장).
	T* Acquire()
	{
		for (auto& actor : pool)
		{
			if (!actor->IsPoolActive())
			{
				actor->Activate();
				return actor.get();
			}
		}

		// --- 자동 확장 ---
		std::shared_ptr<Level> level = ownerLevel.lock();
		if (!level) return nullptr;

		auto actor = level->SpawnActor<T>();
		actor->Deactivate();
		actor->Activate();
		pool.push_back(actor);

		return actor.get();
	}

	// 사용이 끝난 액터를 풀에 반환한다 (비활성화만 한다).
	void Release(T* actor)
	{
		if (actor)
		{
			actor->Deactivate();
		}
	}

	// 활성화된 액터 수.
	int GetActiveCount() const
	{
		int count = 0;
		for (const auto& a : pool)
		{
			if (a->IsPoolActive()) ++count;
		}
		return count;
	}

	// 전체 풀 크기 (활성 + 비활성).
	int GetTotalCount() const
	{
		return static_cast<int>(pool.size());
	}

private:
	std::vector<std::shared_ptr<T>> pool;
	std::weak_ptr<Craft::Level> ownerLevel;
};
