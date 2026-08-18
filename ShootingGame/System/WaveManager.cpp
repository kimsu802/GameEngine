#include "WaveManager.h"
#include <Manager/DataFileUtil.h>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

bool WaveManager::LoadFromFile(const std::string& filename)
{
	std::vector<std::string> lines = DataFileUtil::ReadAllLines("../Assets/" + filename);
	if (lines.empty())
	{
		BuildDefaultWaves();
		return false;
	}

	waves.clear();

	for (const std::string& rawLine : lines)
	{
		std::string key, value;
		if (!DataFileUtil::SplitKeyValue(rawLine, key, value)) continue;

		if (key == "WAVE")
		{
			WaveEntry entry;
			int parsed = sscanf_s(value.c_str(), "%f %d %d %d %f %d %d %f",
				&entry.startTime,
				&entry.enemyType,
				&entry.spawnCount,
				&entry.enemyHp,
				&entry.enemySpeed,
				&entry.contactDamage,
				&entry.goldReward,
				&entry.spawnInterval);

			if (parsed >= 4) // 최소 startTime, type, count, hp가 있어야 유효.
			{
				waves.push_back(entry);
			}
		}
	}

	// 시간순 정렬.
	std::sort(waves.begin(), waves.end(),
		[](const WaveEntry& a, const WaveEntry& b) { return a.startTime < b.startTime; });

	if (waves.empty())
	{
		BuildDefaultWaves();
	}

	return true;
}

const WaveEntry& WaveManager::GetCurrentWave(float elapsedTime) const
{
	// 가장 늦게 시작하면서 현재 시간 이하인 웨이브를 찾는다.
	int best = 0;
	for (int i = 0; i < static_cast<int>(waves.size()); ++i)
	{
		if (waves[i].startTime <= elapsedTime)
		{
			best = i;
		}
	}

	return waves[best];
}

int WaveManager::GetCurrentWaveIndex(float elapsedTime) const
{
	int idx = 0;
	for (int i = 0; i < static_cast<int>(waves.size()); ++i)
	{
		if (waves[i].startTime <= elapsedTime)
		{
			idx = i;
		}
	}
	return idx + 1; // 1-based.
}

void WaveManager::BuildDefaultWaves()
{
	// 파일이 없을 때 사용할 기본 테이블.
	// 대략 WaveConfig.txt의 기본값과 동일.
	waves.clear();

	//       time  type cnt  hp  spd  dmg gold interval
	waves.push_back({  0.f, -1, 1,  1, 3.0f, 1, 1, 1.5f });
	waves.push_back({ 30.f, -1, 2,  2, 4.0f, 1, 2, 1.2f });
	waves.push_back({ 60.f, -1, 3,  4, 5.0f, 1, 3, 1.0f });
	waves.push_back({ 90.f, -1, 4,  6, 5.5f, 2, 4, 0.8f });
	waves.push_back({120.f, -1, 5,  8, 6.0f, 2, 5, 0.6f });
	waves.push_back({150.f, -1, 6, 10, 6.5f, 2, 6, 0.5f });
	waves.push_back({180.f, -1, 8, 14, 7.0f, 3, 8, 0.4f });
	waves.push_back({240.f, -1,10, 20, 8.0f, 3,10, 0.3f });
}
