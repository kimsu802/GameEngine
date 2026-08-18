#include "PlayerState.h"
#include <Manager/DataFileUtil.h>
#include <algorithm>
#include <cstdlib>
#include <fstream>

PlayerState& PlayerState::Get()
{
	static PlayerState instance;
	return instance;
}

void PlayerState::TakeDamage(int amount)
{
	hp = std::max(0, hp - amount);
}

void PlayerState::Heal(int amount)
{
	hp = std::min(maxHp, hp + amount);
}

bool PlayerState::UpgradeMaxHp(int cost, int hpGain)
{
	if (gold < cost) return false;
	gold -= cost;
	maxHp += hpGain;
	hp += hpGain; // 강화 즉시 늘어난 만큼 회복
	return true;
}

bool PlayerState::UpgradeAttackPower(int cost, int atkGain)
{
	if (gold < cost) return false;
	gold -= cost;
	attackPower += atkGain;
	return true;
}

bool PlayerState::UpgradeWeapon(int cost)
{
	if (gold < cost) return false;
	if (weaponLevel >= maxWeaponLevel) return false;
	gold -= cost;
	++weaponLevel;
	return true;
}

const QuestProgress* PlayerState::FindQuestProgress(int questId) const
{
	auto it = questProgressMap.find(questId);
	return (it == questProgressMap.end()) ? nullptr : &it->second;
}

QuestProgress& PlayerState::GetOrCreateQuestProgress(int questId)
{
	return questProgressMap[questId];
}

QuestState PlayerState::GetQuestState(int questId) const
{
	const QuestProgress* p = FindQuestProgress(questId);
	return p ? p->state : QuestState::NotStarted;
}

int PlayerState::GetQuestCurrentCount(int questId) const
{
	const QuestProgress* p = FindQuestProgress(questId);
	return p ? p->currentCount : 0;
}

bool PlayerState::SaveToFile(const std::string& path) const
{
	std::ofstream file(path);
	if (!file.is_open()) return false;

	file << "HP: " << hp << "\n";
	file << "MAXHP: " << maxHp << "\n";
	file << "GOLD: " << gold << "\n";
	file << "ATK: " << attackPower << "\n";
	file << "WEAPON: " << weaponLevel << "\n";
	file << "TRACK: " << trackedQuestId << "\n";

	for (const auto& pair : questProgressMap)
	{
		file << "QUEST: " << pair.first << " "
			<< static_cast<int>(pair.second.state) << " "
			<< pair.second.currentCount << "\n";
	}

	return true;
}

bool PlayerState::LoadFromFile(const std::string& path)
{
	std::vector<std::string> lines = DataFileUtil::ReadAllLines(path);
	if (lines.empty()) return false;

	questProgressMap.clear();

	for (const std::string& rawLine : lines)
	{
		std::string key, value;
		if (!DataFileUtil::SplitKeyValue(rawLine, key, value)) continue;

		if (key == "HP")          hp = std::atoi(value.c_str());
		else if (key == "MAXHP")  maxHp = std::atoi(value.c_str());
		else if (key == "GOLD")   gold = std::atoi(value.c_str());
		else if (key == "ATK")    attackPower = std::atoi(value.c_str());
		else if (key == "WEAPON") weaponLevel = std::clamp(std::atoi(value.c_str()), 1, maxWeaponLevel);
		else if (key == "TRACK")  trackedQuestId = std::atoi(value.c_str());
		else if (key == "QUEST")
		{
			int qid = 0, sv = 0, cc = 0;
			if (sscanf_s(value.c_str(), "%d %d %d", &qid, &sv, &cc) == 3)
			{
				QuestProgress p;
				p.state = static_cast<QuestState>(sv);
				p.currentCount = cc;
				questProgressMap[qid] = p;
			}
		}
	}

	return true;
}

void PlayerState::Reset()
{
	hp = maxHp;
	gold = 0;
	attackPower = 1;
	weaponLevel = 1;
	questProgressMap.clear();
	trackedQuestId = -1;
}
