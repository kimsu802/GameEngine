#include "QuestManager.h"
#include "DataFileUtil.h"
#include <State/PlayerState.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace
{
	QuestType ParseQuestType(const std::string& value)
	{
		std::string upper = value;
		std::transform(upper.begin(), upper.end(), upper.begin(),
			[](unsigned char c) { return static_cast<char>(std::toupper(c)); });
		if (upper == "SOKOBAN")  return QuestType::SOKOBAN;
		if (upper == "DIALOGUE") return QuestType::DIALOGUE;
		return QuestType::KILL;
	}
}

QuestManager& QuestManager::Get()
{
	static QuestManager instance;
	return instance;
}

bool QuestManager::LoadQuestFromFile(int questId, const std::string& filename)
{
	std::vector<std::string> lines = DataFileUtil::ReadAllLines("../Assets/" + filename);
	if (lines.empty()) return false;

	QuestData data;
	data.questId = questId;

	for (const std::string& rawLine : lines)
	{
		std::string key, value;
		if (!DataFileUtil::SplitKeyValue(rawLine, key, value)) continue;

		if      (key == "TYPE")       data.type = ParseQuestType(value);
		else if (key == "TITLE")      data.title = value;
		else if (key == "NPC")        data.offerDialogueLines.push_back(value);
		else if (key == "QUEST")      data.description = value;
		else if (key == "FAIL")       data.inProgressDialogueLines.push_back(value);
		else if (key == "CLEAR")      data.completeDialogueLines.push_back(value);
		else if (key == "REQUIRED")   data.requiredCount = std::atoi(value.c_str());
		else if (key == "REWARD")     data.rewardGold = std::atoi(value.c_str());
		else if (key == "TARGET_NPC") data.targetNpcId = std::atoi(value.c_str());
	}

	if (data.type == QuestType::DIALOGUE) data.requiredCount = 1;
	else if (data.type == QuestType::KILL && data.requiredCount <= 0) data.requiredCount = 1;

	questDataMap[questId] = data;
	return true;
}

const QuestData* QuestManager::FindQuestData(int questId) const
{
	auto it = questDataMap.find(questId);
	return (it == questDataMap.end()) ? nullptr : &it->second;
}

std::string QuestManager::GetQuestTitle(int questId) const
{
	const QuestData* d = FindQuestData(questId);
	if (!d) return "";
	return d->title.empty() ? d->description : d->title;
}

void QuestManager::StartQuest(int questId)
{
	if (questDataMap.find(questId) == questDataMap.end()) return;
	QuestProgress& p = PlayerState::Get().GetOrCreateQuestProgress(questId);
	if (p.state != QuestState::NotStarted) return;
	p.state = QuestState::InProgress;
	p.currentCount = 0;
	PlayerState::Get().SetTrackedQuestId(questId);
}

void QuestManager::SetCurrentCountClamped(int questId, int value)
{
	const QuestData* d = FindQuestData(questId);
	if (!d) return;
	QuestProgress& p = PlayerState::Get().GetOrCreateQuestProgress(questId);
	p.currentCount = std::max(0, std::min(value, d->requiredCount));
}

void QuestManager::ReportKill(int amount)
{
	for (const auto& pair : questDataMap)
	{
		if (pair.second.type != QuestType::KILL) continue;
		if (PlayerState::Get().GetQuestState(pair.first) != QuestState::InProgress) continue;
		SetCurrentCountClamped(pair.first, PlayerState::Get().GetQuestCurrentCount(pair.first) + amount);
	}
}

void QuestManager::ReportSokoban(int placedCount, int totalCount)
{
	for (auto& pair : questDataMap)
	{
		if (pair.second.type != QuestType::SOKOBAN) continue;
		if (PlayerState::Get().GetQuestState(pair.first) != QuestState::InProgress) continue;
		if (pair.second.requiredCount <= 0) pair.second.requiredCount = totalCount;
		SetCurrentCountClamped(pair.first, placedCount);
	}
}

void QuestManager::ReportDialogue(int npcId)
{
	for (const auto& pair : questDataMap)
	{
		if (pair.second.type != QuestType::DIALOGUE) continue;
		if (pair.second.targetNpcId != npcId) continue;
		if (PlayerState::Get().GetQuestState(pair.first) != QuestState::InProgress) continue;
		SetCurrentCountClamped(pair.first, pair.second.requiredCount);
	}
}

void QuestManager::AddProgress(int questId, int amount)
{
	if (PlayerState::Get().GetQuestState(questId) != QuestState::InProgress) return;
	SetCurrentCountClamped(questId, PlayerState::Get().GetQuestCurrentCount(questId) + amount);
}

bool QuestManager::IsQuestCompletable(int questId) const
{
	const QuestData* d = FindQuestData(questId);
	if (!d) return false;
	return PlayerState::Get().GetQuestState(questId) == QuestState::InProgress
		&& PlayerState::Get().GetQuestCurrentCount(questId) >= d->requiredCount;
}

void QuestManager::CompleteQuest(int questId)
{
	const QuestData* d = FindQuestData(questId);
	if (!d) return;
	QuestProgress& p = PlayerState::Get().GetOrCreateQuestProgress(questId);
	if (p.state != QuestState::InProgress) return;
	p.state = QuestState::Completed;
	if (d->rewardGold > 0) PlayerState::Get().AddGold(d->rewardGold);
	if (PlayerState::Get().GetTrackedQuestId() == questId)
		PlayerState::Get().SetTrackedQuestId(FindFirstInProgressQuestId());
}

QuestState QuestManager::GetQuestState(int questId) const
{
	return PlayerState::Get().GetQuestState(questId);
}

int QuestManager::GetCurrentCount(int questId) const
{
	return PlayerState::Get().GetQuestCurrentCount(questId);
}

int QuestManager::GetRequiredCount(int questId) const
{
	const QuestData* d = FindQuestData(questId);
	return d ? d->requiredCount : 0;
}

int QuestManager::FindFirstInProgressQuestId() const
{
	int found = -1;
	for (const auto& pair : questDataMap)
	{
		if (PlayerState::Get().GetQuestState(pair.first) != QuestState::InProgress) continue;
		if (found < 0 || pair.first < found) found = pair.first;
	}
	return found;
}

void QuestManager::Reset()
{
	questDataMap.clear();
}
