#pragma once

// ==========================================
//  EnemyState / EnemyFSM — 적 AI 상태 머신
// ==========================================
//
// # 왜 FSM인가?
//   기존 SurvivorEnemy는 Tick()에 if/else가 얽힌 "무조건 추적" 한 가지 행동밖에 없었다.
//   FSM으로 바꾸면:
//     1) 행동 추가가 상태 하나 넣는 것으로 끝난다 (OCP).
//     2) 전이 조건이 코드로 명시되어 디버깅이 쉽다.
//     3) 엘리트/보스처럼 패턴이 다른 적을 FSM 상속 없이
//        전이 조건(거리 임계값, 쿨다운)만 바꿔서 찍어낼 수 있다.
//
// # 상태 목록
//   Idle  — 스폰 직후. 잠깐 대기한다 (눈에 보이게 "등장 연출" 역할).
//   Chase — 플레이어와의 거리가 detectRange 이내면 전환. 플레이어를 향해 직선 이동.
//   Rush  — 거리가 rushRange 이내면 전환. 속도가 1.5배로 올라간다.
//
//   Idle → Chase : 거리 ≤ detectRange
//   Chase → Rush : 거리 ≤ rushRange
//
//   상태가 뒤로 돌아가는 전이는 없다. 한번 돌진(Rush)에 들어가면 끝까지 간다.
//   이 구조는 뱀서 장르의 "밀려오는 물량" 느낌을 유지하면서도,
//   코드상 FSM 패턴을 명확하게 보여주기 위한 설계다.
//
enum class EnemyState
{
	Idle,   // 스폰 직후 대기.
	Chase,  // 플레이어를 향해 이동.
	Rush,   // 가까이 오면 가속 돌진.
};

struct EnemyFSM
{
	EnemyState currentState = EnemyState::Idle;

	float idleTimer = 0.f;
	float idleDuration = 0.5f;     // Idle 상태 유지 시간.

	float detectRange = 60.f;      // Chase 전이 거리 (맨해튼 거리).
	float rushRange = 10.f;        // Rush 전이 거리.
	float rushSpeedMultiplier = 1.5f;

	// 프레임마다 호출. 현재 상태를 갱신하고 "이번 프레임의 속도 배율"을 돌려준다.
	float Update(float deltaTime, float distToPlayer)
	{
		switch (currentState)
		{
		case EnemyState::Idle:
		{
			idleTimer += deltaTime;
			if (idleTimer >= idleDuration || distToPlayer <= detectRange)
			{
				currentState = EnemyState::Chase;
			}
			return 0.f; // Idle 중에는 이동하지 않는다.
		}

		case EnemyState::Chase:
		{
			if (distToPlayer <= rushRange)
			{
				currentState = EnemyState::Rush;
				return rushSpeedMultiplier;
			}
			return 1.f;
		}

		case EnemyState::Rush:
		{
			return rushSpeedMultiplier;
		}
		}

		return 1.f;
	}

	void Reset()
	{
		currentState = EnemyState::Idle;
		idleTimer = 0.f;
	}
};
