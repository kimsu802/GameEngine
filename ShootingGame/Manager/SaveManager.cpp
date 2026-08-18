#include "SaveManager.h"
#include <Manager/DataFileUtil.h>
#include <State/PlayerState.h>
#include <Type/QuestTypes.h>
#include <fstream>
#include <cstdlib>
#include <cstdio>

using namespace Craft;

SaveManager& SaveManager::Get()
{
	static SaveManager instance;
	return instance;
}

bool SaveManager::Save(const Vector2& playerPos,
	const std::vector<Vector2>& boxPositions,
	const std::string& path)
{
	std::ofstream file(path);
	if (!file.is_open()) return false;

	const PlayerState& ps = PlayerState::Get();

	file << "HP: " << ps.GetHp() << "\n";
	file << "MAXHP: " << ps.GetMaxHp() << "\n";
	file << "GOLD: " << ps.GetGold() << "\n";
	file << "ATK: " << ps.GetAttackPower() << "\n";
	file << "WEAPON: " << ps.GetWeaponLevel() << "\n";
	file << "TRACK: " << ps.GetTrackedQuestId() << "\n";

	// 퀘스트 진행 상황.
	for (const auto& pair : ps.GetAllQuestProgress())
	{
		file << "QUEST: " << pair.first << " "
			<< static_cast<int>(pair.second.state) << " "
			<< pair.second.currentCount << "\n";
	}

	// 맵 상태.
	file << "PLAYERPOS: " << playerPos.x << " " << playerPos.y << "\n";

	for (const Vector2& boxPos : boxPositions)
	{
		file << "BOX: " << boxPos.x << " " << boxPos.y << "\n";
	}

	return true;
}

bool SaveManager::Load(const std::string& path)
{
	std::vector<std::string> lines = DataFileUtil::ReadAllLines(path);
	if (lines.empty()) return false;

	PlayerState& ps = PlayerState::Get();

	// PlayerState 내부 값을 직접 세팅하기 위해 LoadFromFile 로직을 여기서 직접 수행.
	// (PlayerState::LoadFromFile을 호출해도 되지만 박스/플레이어 위치를 같이 읽어야 하므로
	//  한 번에 처리하는 게 효율적.)

	savedBoxPositions.clear();
	savedPlayerPos = Vector2::Zero;
	hasLoadedData = false;

	// 임시 저장소.
	int hp = 5, maxHp = 100, gold = 0, atk = 1, weapon = 1, track = -1;
	std::vector<std::tuple<int, int, int>> questEntries; // id, state, count

	for (const std::string& rawLine : lines)
	{
		std::string key, value;
		if (!DataFileUtil::SplitKeyValue(rawLine, key, value)) continue;

		if (key == "HP")          hp = std::atoi(value.c_str());
		else if (key == "MAXHP")  maxHp = std::atoi(value.c_str());
		else if (key == "GOLD")   gold = std::atoi(value.c_str());
		else if (key == "ATK")    atk = std::atoi(value.c_str());
		else if (key == "WEAPON") weapon = std::atoi(value.c_str());
		else if (key == "TRACK")  track = std::atoi(value.c_str());
		else if (key == "QUEST")
		{
			int qid = 0, sv = 0, cc = 0;
			if (sscanf_s(value.c_str(), "%d %d %d", &qid, &sv, &cc) == 3)
			{
				questEntries.push_back({ qid, sv, cc });
			}
		}
		else if (key == "PLAYERPOS")
		{
			int px = 0, py = 0;
			if (sscanf_s(value.c_str(), "%d %d", &px, &py) == 2)
			{
				savedPlayerPos = Vector2(px, py);
			}
		}
		else if (key == "BOX")
		{
			int bx = 0, by = 0;
			if (sscanf_s(value.c_str(), "%d %d", &bx, &by) == 2)
			{
				savedBoxPositions.push_back(Vector2(bx, by));
			}
		}
	}

	// PlayerState에 적용.
	// Reset() 후 하나씩 세팅.
	ps.Reset();

	// Reset이 maxHp를 기본값으로 되돌리므로, UpgradeMaxHp를 cost=0으로 반복해서 맞추거나
	// 직접 세팅 함수를 쓴다. PlayerState에 직접 세터가 있으므로 LoadFromFile을 호출.
	// 여기서는 간단하게 LoadFromFile의 로직을 재현한다.
	// → PlayerState에 SetAll 함수를 추가하는 대신, 파일을 다시 써서 LoadFromFile을 호출.
	// 가장 깔끔한 방법: PlayerState::LoadFromFile을 호출한 뒤 맵 데이터만 여기서 처리.

	ps.LoadFromFile(path);

	hasLoadedData = true;
	return true;
}

bool SaveManager::SaveFileExists(const std::string& path)
{
	std::ifstream file(path);
	return file.good();
}
