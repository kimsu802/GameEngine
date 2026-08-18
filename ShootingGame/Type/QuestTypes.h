#pragma once

enum class QuestState
{
	NotStarted,
	InProgress,
	Completed
};

enum class QuestType
{
	KILL,
	SOKOBAN,
	DIALOGUE,
};

struct QuestProgress
{
	QuestState state = QuestState::NotStarted;
	int currentCount = 0;
};
