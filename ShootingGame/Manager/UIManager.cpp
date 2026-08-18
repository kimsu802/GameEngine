#include "UIManager.h"
#include <Input/Input.h>
#include <Render/Renderer.h>
#include <Math/Vector2.h>
#include <State/PlayerState.h>
#include <Manager/DialogueManager.h>
#include <Manager/QuestManager.h>
#include <Manager/NPCManager.h>
#include <System/StringUtil.h>
#include <algorithm>

using namespace Craft;

namespace
{
	// 플레이어 쪽 초상화/닉네임은 NPC처럼 데이터 등록 시스템이 없으므로
	// 여기서 간단히 상수로 관리한다.
	const std::string kPlayerNickname = "나";

	const std::vector<std::string> kPlayerPortraitLines =
	{
		" (^o^) ",
		" /|||\\ ",
		"  / \\  "
	};
}

UIManager& UIManager::Get()
{
	static UIManager instance;
	return instance;
}

void UIManager::ShowConfirmUI(const std::string& text, std::function<void(ButtonType)> callback)
{
	confirmText = text;
	onResult = callback;
	isShowing = true;
}

void UIManager::ShowChoiceUI(const std::string& title, const std::vector<std::string>& options, std::function<void(int)> callback)
{
	if (!options.empty())
	{
		choiceTitle = title;
		choiceOptions = options;
		onChoiceResult = callback;
		isShowingChoice = true;
	}
}

void UIManager::Tick(float deltaTime)
{
	if (isShowingChoice)
	{
		// 옵션 개수만큼 1,2,3... 숫자 키를 검사한다 (최대 9개).
		const int optionCount = static_cast<int>(choiceOptions.size());
		const int maxCheckable = optionCount < 9 ? optionCount : 9;

		for (int i = 0; i < maxCheckable; ++i)
		{
			if (Input::Get().GetKeyDown('1' + i))
			{
				std::function<void(int)> callback = onChoiceResult;
				int selectedIndex = i;

				// 콜백 호출 전에 먼저 상태를 정리한다.
				// (콜백 안에서 다시 ShowChoiceUI를 호출하는 경우를 대비)
				isShowingChoice = false;
				choiceTitle.clear();
				choiceOptions.clear();
				onChoiceResult = nullptr;

				if (callback)
				{
					callback(selectedIndex);
				}

				return;
			}
		}

		return;
	}

	if (!isShowing) return;

	if (Input::Get().GetKeyDown('Y'))
	{
		isShowing = false;
		if (onResult) onResult(ButtonType::YES);
	}

	else if (Input::Get().GetKeyDown('N'))
	{
		isShowing = false;
		if (onResult) onResult(ButtonType::NO);
	}
}

void UIManager::SetInteractingNPC(const std::string& nickname, const std::vector<std::string>& portraitLines)
{
	npcNickname = nickname;
	npcPortraitLines = portraitLines;
}

void UIManager::Draw()
{
	// UI 뷰포트를 상/하로 나눠서 그린다.
	// 상단부 : 대화 중인 NPC + 플레이어의 아스키아트/닉네임
	// 하단부 : 플레이어 스테이터스
	DrawSpeakerPortraits();
	DrawPlayerStatus();
	DrawQuestInfo();

	DrawChoiceUI();

	if (!isShowing) return;
	Renderer::Get().Submit(confirmText + "(Y / N)", Vector2(0,1), Color::Blue, 100, RenderSpace::Dialogue);

}

void UIManager::DrawChoiceUI()
{
	if (!isShowingChoice)
	{
		return;
	}

	// 대화창(Dialogue) 영역은 세로로 3줄 정도밖에 안 되므로
	// 제목은 0번 줄, 선택지는 전부 한 줄에 이어붙여서 1번 줄에 표시한다.
	Renderer::Get().Submit(choiceTitle, Vector2(0, 0), Color::White, 100, RenderSpace::Dialogue);

	std::string optionsLine;
	for (size_t i = 0; i < choiceOptions.size(); ++i)
	{
		optionsLine += std::to_string(i + 1) + ". " + choiceOptions[i] + "   ";
	}

	Renderer::Get().Submit(optionsLine, Vector2(0, 1), Color::Yellow, 100, RenderSpace::Dialogue);
}

void UIManager::DrawSpeakerPortraits()
{
	// 대화 중이거나 (대화 후) 선택지 메뉴가 떠 있는 동안은 계속 상단부에 표시한다.
	if (!DialogueManager::Get().GetIsPlaying() && !isShowingChoice)
	{
		return;
	}

	const Viewport& uiViewport = Renderer::Get().GetViewport(RenderSpace::UI);
	const int halfWidth = uiViewport.size.x / 2;

	// --- 왼쪽 : 상호작용 중인 NPC ---
	const int npcNameX = 2;
	int npcArtY = 1;

	int uiviewportwidth = Renderer::Get().GetViewport(RenderSpace::UI).size.x;

	// 닉네임을 UI 뷰포트 기준 가운데 정렬.
	// (기존 코드는 상수 npcNameX를 나누고 있어서 닉네임 길이를 반영하지 못했음.)
	int nicknameX = StringUtil::GetCenteredX(uiviewportwidth, npcNickname);
	Renderer::Get().Submit(npcNickname, Vector2(nicknameX, 0), Color::Cyan, 1, RenderSpace::UI);
	for (const std::string& line : npcPortraitLines)
	{
		Renderer::Get().Submit(line, Vector2(npcNameX, npcArtY), Color::Cyan, 1, RenderSpace::UI);
		++npcArtY;
	}

	//// --- 오른쪽 : 플레이어 ---
	//const int playerNameX = halfWidth + 2;
	//int playerArtY = 1;

	//Renderer::Get().Submit(kPlayerNickname, Vector2(playerNameX, 0), Color::Green, 1, RenderSpace::UI);
	//for (const std::string& line : kPlayerPortraitLines)
	//{
	//	Renderer::Get().Submit(line, Vector2(playerNameX, playerArtY), Color::Green, 1, RenderSpace::UI);
	//	++playerArtY;
	//}


}

void UIManager::DrawPlayerStatus()
{
	// 하단부에 그리기 위해 UI 뷰포트 높이의 절반만큼 y를 내려서 그린다.
	const Viewport& uiViewport = Renderer::Get().GetViewport(RenderSpace::UI);
	const int statusStartY = uiViewport.size.y / 2 + 1;

	// 플레이어 스테이터스 표시
	Renderer::Get().Submit("HP : " + std::to_string(PlayerState::Get().GetHp()), Vector2(2, statusStartY), Color::Yellow, 1, RenderSpace::UI);
	Renderer::Get().Submit("GOLD : " + std::to_string(PlayerState::Get().GetGold()), Vector2(2, statusStartY + 1), Color::Yellow, 1, RenderSpace::UI);
	Renderer::Get().Submit("AttackPower : " + std::to_string(PlayerState::Get().GetAttackPower()), Vector2(2, statusStartY + 2), Color::Yellow, 1, RenderSpace::UI);
}

std::string UIManager::MakeProgressBar(int current, int required, int barWidth) const
{
	if (required <= 0) required = 1;
	if (current < 0) current = 0;
	if (current > required) current = required;

	int filled = (current * barWidth) / required;
	std::string bar = "[";
	for (int i = 0; i < barWidth; ++i)
		bar += (i < filled) ? '#' : '-';
	bar += "]";
	return bar;
}

void UIManager::DrawQuestInfo()
{
	const Viewport& uiViewport = Renderer::Get().GetViewport(RenderSpace::UI);
	int y = uiViewport.size.y / 2 + 5;

	// 진행 중인 퀘스트를 전부 모은다.
	std::vector<int> activeQuestIds;
	const auto& allProgress = PlayerState::Get().GetAllQuestProgress();
	for (const auto& pair : allProgress)
	{
		if (pair.second.state == QuestState::InProgress)
		{
			activeQuestIds.push_back(pair.first);
		}
	}

	if (activeQuestIds.empty()) return;

	// questId 오름차순 정렬 (받은 순서대로 표시).
	std::sort(activeQuestIds.begin(), activeQuestIds.end());

	Renderer::Get().Submit("---- QUEST ----", Vector2(2, y), Color::White, 1, RenderSpace::UI);
	++y;

	for (int questId : activeQuestIds)
	{
		const QuestData* quest = QuestManager::Get().FindQuestData(questId);
		if (!quest) continue;

		// 제목.
		Renderer::Get().Submit(QuestManager::Get().GetQuestTitle(questId),
			Vector2(2, y), Color::Yellow, 1, RenderSpace::UI);
		++y;

		const int current = QuestManager::Get().GetCurrentCount(questId);
		const int required = quest->requiredCount;
		const bool completable = QuestManager::Get().IsQuestCompletable(questId);

		// 타입별 진행도.
		switch (quest->type)
		{
		case QuestType::KILL:
		case QuestType::SOKOBAN:
			Renderer::Get().Submit(
				std::to_string(current) + " / " + std::to_string(required)
				+ "  " + MakeProgressBar(current, required, 8),
				Vector2(4, y), Color::White, 1, RenderSpace::UI);
			++y;
			break;

		case QuestType::DIALOGUE:
		{
			std::string targetName = "???";
			const NPCData* npc = NPCManager::Get().FindNPCData(quest->targetNpcId);
			if (npc && !npc->nickname.empty()) targetName = npc->nickname;
			Renderer::Get().Submit(
				targetName + (completable ? " [V]" : " [ ]"),
				Vector2(4, y), Color::White, 1, RenderSpace::UI);
			++y;
			break;
		}
		}

		if (completable)
		{
			Renderer::Get().Submit(">> Complete !", Vector2(4, y), Color::Green, 1, RenderSpace::UI);
			++y;
		}

		// 뷰포트 아래로 넘어가면 더 그리지 않는다.
		if (y >= uiViewport.size.y - 1) break;
	}
}
