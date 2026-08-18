#pragma once

#include <string>
#include <unordered_map>
#include <Type/QuestTypes.h>

class PlayerState
{
public:
	static PlayerState& Get();

	// ---------- 기본 스탯 ----------
	inline int GetHp() const { return hp; }
	inline int GetMaxHp() const { return maxHp; }
	inline int GetGold() const { return gold; }
	inline int GetAttackPower() const { return attackPower; }
	inline int GetWeaponLevel() const { return weaponLevel; }
	inline int GetMaxWeaponLevel() const { return maxWeaponLevel; }

	void TakeDamage(int amount);
	void Heal(int amount);
	void AddGold(int amount) { gold += amount; }
	void SetAttackPower(int value) { attackPower = value; }

	// ---------- 강화 ----------
	// 골드가 충분하면 강화하고 true, 아니면 false.
	bool UpgradeMaxHp(int cost, int hpGain = 20);
	bool UpgradeAttackPower(int cost, int atkGain = 1);
	bool UpgradeWeapon(int cost);

	// ---------- 퀘스트 진행 상황 ----------
	const QuestProgress* FindQuestProgress(int questId) const;
	QuestProgress& GetOrCreateQuestProgress(int questId);
	QuestState GetQuestState(int questId) const;
	int GetQuestCurrentCount(int questId) const;

	inline const std::unordered_map<int, QuestProgress>& GetAllQuestProgress() const { return questProgressMap; }

	inline int GetTrackedQuestId() const { return trackedQuestId; }
	inline void SetTrackedQuestId(int questId) { trackedQuestId = questId; }

	// ---------- 세이브 / 로드 ----------
	bool SaveToFile(const std::string& path = "../Assets/Save.txt") const;
	bool LoadFromFile(const std::string& path = "../Assets/Save.txt");

	void Reset();

	// 던전에서 나올 때: HP만 최대로 회복 (스탯/골드/퀘스트는 유지)
	void FullHeal() { hp = maxHp; }

private:
	PlayerState() = default;

	int hp = 5;
	int maxHp = 100;
	int gold = 0;
	int attackPower = 1;
	int weaponLevel = 1;
	int maxWeaponLevel = 3;

	std::unordered_map<int, QuestProgress> questProgressMap;
	int trackedQuestId = -1;
};
