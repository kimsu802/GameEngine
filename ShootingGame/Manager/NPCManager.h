#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <Manager/QuestManager.h> // QuestState는 여기서 정의된 것을 그대로 사용한다 (중복 정의 방지).

struct NPCData
{
	std::string nickname;
	std::vector<std::string> dialogueLines;

	// UI 상단부에 표시할 초상화(아스키 아트). 여러 줄로 구성.
	std::vector<std::string> portraitLines;

	// 이 NPC가 퀘스트를 주는 NPC라면 연결된 퀘스트 id (QuestManager에 등록된 id). 없으면 -1.
	int questId = -1;

	QuestState questState = QuestState::NotStarted;
};

class NPCManager
{
public:
	static NPCManager& Get();

	// 게임 시작할 때 딱 한번만 등록 (코드에서 직접 등록하고 싶을 때 사용).
	void RegisterNPC(int npcId, const std::string& nickname, const std::vector<std::string>& dialoguelines,
		const std::vector<std::string>& portraitLines = {});

	// Assets/ 폴더 기준 상대경로의 NPC 데이터 파일을 읽어서 여러 NPC를 한 번에 등록한다.
	// 파일 포맷은 NPCManager.cpp 상단 주석 참고 (예: Assets/NPCs.txt).
	// 같은 npcId가 이미 있으면 덮어쓴다. 파일을 못 읽으면 false 반환.
	bool LoadFromFile(const std::string& filename);

	const NPCData* FindNPCData(int npcId) const;

	void SetQuestState(int npcId, QuestState state);
	QuestState GetQuestState(int npcId) const;

	void Reset();

private:
	NPCManager() = default;

	std::unordered_map<int, NPCData> npcDataMap;
};

