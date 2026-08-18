#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <Type/QuestTypes.h>

struct QuestData
{
	int questId = -1;
	QuestType type = QuestType::KILL;
	std::string title;
	std::string description;
	std::vector<std::string> offerDialogueLines;
	std::vector<std::string> inProgressDialogueLines;
	std::vector<std::string> completeDialogueLines;
	int requiredCount = 0;
	int rewardGold = 0;
	int targetNpcId = -1;
};

class QuestManager
{
public:
	static QuestManager& Get();

	bool LoadQuestFromFile(int questId, const std::string& filename);
	const QuestData* FindQuestData(int questId) const;
	std::string GetQuestTitle(int questId) const;

	void StartQuest(int questId);

	void ReportKill(int amount = 1);
	void ReportSokoban(int placedCount, int totalCount);
	void ReportDialogue(int npcId);
	void AddProgress(int questId, int amount = 1);

	bool IsQuestCompletable(int questId) const;
	void CompleteQuest(int questId);

	QuestState GetQuestState(int questId) const;
	int GetCurrentCount(int questId) const;
	int GetRequiredCount(int questId) const;
	int FindFirstInProgressQuestId() const;

	void Reset();

private:
	QuestManager() = default;
	void SetCurrentCountClamped(int questId, int value);
	std::unordered_map<int, QuestData> questDataMap;
};
