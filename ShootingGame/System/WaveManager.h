#pragma once

#include <vector>
#include <string>

// ==========================================
//  WaveConfig / WaveManager — 데이터 드리븐 웨이브
// ==========================================
//
// # 왜 데이터 드리븐인가?
//   기존에는 난이도가 코드에 하드코딩(`elapsedTime / 30.f`)되어 있어서,
//   밸런스를 바꾸려면 매번 컴파일-실행-확인을 반복해야 했다.
//   WaveConfig.txt 로 분리하면:
//     1) 기획자가 코드를 건드리지 않고 밸런스를 조절할 수 있다.
//     2) 여러 벌의 WaveConfig를 만들어 스테이지별 난이도를 다르게 할 수 있다.
//     3) "프로그래머-기획자 분업 구조를 이해하고 있다"는 포트폴리오 어필이 된다.
//
// # 파일 포맷 (Assets/WaveConfig.txt)
//   WAVE: <startTime> <enemyType> <count> <hp> <speed> <contactDmg> <gold> <spawnInterval>
//
//   startTime     : 이 웨이브가 시작되는 경과 시간(초).
//   enemyType     : 적 외형 인덱스 (0~5). -1이면 랜덤.
//   count         : 스폰 주기마다 한 번에 몇 마리씩 나올지.
//   hp            : 적 체력.
//   speed         : 적 이동 속도.
//   contactDmg    : 적 접촉 데미지.
//   gold          : 적 처치 보상 골드.
//   spawnInterval : 스폰 주기(초). 이 간격마다 count마리씩 나온다.
//

struct WaveEntry
{
	float startTime = 0.f;
	int enemyType = -1;      // -1 → 랜덤
	int spawnCount = 1;
	int enemyHp = 1;
	float enemySpeed = 3.f;
	int contactDamage = 1;
	int goldReward = 1;
	float spawnInterval = 1.5f;
};

class WaveManager
{
public:
	WaveManager() = default;

	// Assets/WaveConfig.txt를 읽어서 웨이브 테이블을 구성한다.
	// 파일이 없으면 기본 테이블을 사용한다.
	bool LoadFromFile(const std::string& filename = "WaveConfig.txt");

	// 현재 경과 시간에 해당하는 웨이브를 돌려준다.
	// 여러 웨이브가 겹칠 경우 가장 마지막(가장 높은 난이도)을 우선한다.
	const WaveEntry& GetCurrentWave(float elapsedTime) const;

	// 로드된 웨이브 수.
	inline int GetWaveCount() const { return static_cast<int>(waves.size()); }

	// 현재 경과 시간이 몇 번째 웨이브에 해당하는지 (1-based, UI 표시용).
	int GetCurrentWaveIndex(float elapsedTime) const;

private:
	void BuildDefaultWaves();

	std::vector<WaveEntry> waves;
};
