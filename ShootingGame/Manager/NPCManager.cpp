#include "NPCManager.h"
#include "DataFileUtil.h"
#include <cstdlib>

// Assets/NPCs.txt 데이터 파일 포맷:
// (빈 줄로 NPC 블록을 구분할 필요는 없음. "NPC:" 키가 나오면 새 NPC 블록 시작으로 간주한다.)
//
//   NPC: <npcId>          -> 새 NPC 블록 시작. npcId는 정수.
//   NAME: <이름>           -> UI에 표시할 닉네임.
//   PORTRAIT: <파일명>     -> Assets/ 폴더에 있는 초상화(아스키 아트) txt 파일명.
//   LINE: <대사>           -> 평소(퀘스트와 무관한) 대사 한 줄. 여러 줄 가능.
//   QUEST: <questId>      -> (선택) 이 NPC가 퀘스트를 주는 NPC라면 연결할 퀘스트 id.
//
// 예시:
//   NPC: 0
//   NAME: 시고르자브 이장님
//   PORTRAIT: Dog.txt
//   LINE: 어서오게, 오랜만이구먼.
//   LINE: 밥은 먹었는가?
//   QUEST: 1

NPCManager& NPCManager::Get()
{
	static NPCManager instance;
	return instance;
}

void NPCManager::RegisterNPC(int npcId, const std::string& nickname,
	const std::vector<std::string>& dialoguelines,
	const std::vector<std::string>& portraitLines)
{
	NPCData data;
	data.nickname = nickname;
	data.dialogueLines = dialoguelines;
	data.portraitLines = portraitLines;
	data.questState = QuestState::NotStarted;

	// 같은 npcId로 다시 등록하면 덮어쓴다 (재시작/재로드 대비).
	npcDataMap[npcId] = data;
}

bool NPCManager::LoadFromFile(const std::string& filename)
{
	std::vector<std::string> lines = DataFileUtil::ReadAllLines("../Assets/" + filename);
	if (lines.empty())
	{
		return false;
	}

	bool hasPendingNPC = false;
	int npcId = -1;
	NPCData data;

	// 지금까지 읽은 블록을 map에 저장하고 상태를 초기화한다.
	auto Flush = [this, &hasPendingNPC, &npcId, &data]()
	{
		if (hasPendingNPC && npcId >= 0)
		{
			npcDataMap[npcId] = data;
		}

		hasPendingNPC = false;
		npcId = -1;
		data = NPCData();
	};

	for (const std::string& rawLine : lines)
	{
		std::string key, value;
		if (!DataFileUtil::SplitKeyValue(rawLine, key, value))
		{
			continue; // 빈 줄 등은 건너뛴다.
		}

		if (key == "NPC")
		{
			Flush(); // 새 NPC 블록이 시작됐으므로 이전 블록을 먼저 저장.
			hasPendingNPC = true;
			npcId = std::atoi(value.c_str());
		}
		else if (key == "NAME")
		{
			data.nickname = value;
		}
		else if (key == "PORTRAIT")
		{
			data.portraitLines = DataFileUtil::ReadAllLines("../Assets/" + value);
		}
		else if (key == "LINE")
		{
			data.dialogueLines.push_back(value);
		}
		else if (key == "QUEST")
		{
			data.questId = std::atoi(value.c_str());
		}
	}

	Flush(); // 마지막 블록 저장.
	return true;
}

const NPCData* NPCManager::FindNPCData(int npcId) const
{
	auto it = npcDataMap.find(npcId);
	if (it == npcDataMap.end())
	{
		return nullptr;
	}

	return &it->second;
}

void NPCManager::SetQuestState(int npcId, QuestState state)
{
	auto it = npcDataMap.find(npcId);
	if (it == npcDataMap.end())
	{
		return;
	}

	it->second.questState = state;
}

QuestState NPCManager::GetQuestState(int npcId) const
{
	auto it = npcDataMap.find(npcId);
	if (it == npcDataMap.end())
	{
		return QuestState::NotStarted;
	}

	return it->second.questState;
}

void NPCManager::Reset()
{
	npcDataMap.clear();
}
