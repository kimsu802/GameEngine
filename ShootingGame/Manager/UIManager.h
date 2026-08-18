#pragma once
#include <string>
#include <vector>
#include <functional>

enum class ButtonType
{
	YES,
	NO,
};

class UIManager
{
public:
	static UIManager& Get();

	void ShowConfirmUI(const std::string& text, std::function<void(ButtonType)> callback);

	// 숫자 키(1,2,3...)로 고르는 선택지 메뉴.
	// ex) ShowChoiceUI("무엇을 하시겠습니까?", {"강화하기","상점","이동하기"}, [](int idx){ ... });
	// 콜백의 idx는 0부터 시작 (0 = 첫 번째 옵션).
	void ShowChoiceUI(const std::string& title, const std::vector<std::string>& options, std::function<void(int)> callback);

	void Tick(float deltaTime);
	void Draw();
	void DrawPlayerStatus();

	// 대화가 시작될 때 상대 NPC의 초상화/닉네임을 등록해서
	// UI 상단부에 그릴 수 있게 한다.
	void SetInteractingNPC(const std::string& nickname, const std::vector<std::string>& portraitLines);

	// 확인창(Y/N)이나 선택지 메뉴 중 하나라도 떠 있으면 true.
	// (플레이어 이동 차단 등에 사용)
	inline bool IsShowing() const { return isShowing || isShowingChoice; }

private:
	// 상단부: NPC/플레이어 초상화 + 닉네임을 그린다.
	void DrawSpeakerPortraits();

	// 대화창 영역에 선택지 메뉴(제목 + 번호 목록)를 그린다.
	void DrawChoiceUI();

	// UI 뷰포트에 현재 추적 중인 퀘스트 제목/진행도를 그린다.
	void DrawQuestInfo();
	std::string MakeProgressBar(int current, int required, int barWidth = 12) const;

private:
	bool isShowing = false;
	std::string confirmText;
	std::function<void(ButtonType)> onResult;

	bool isShowingChoice = false;
	std::string choiceTitle;
	std::vector<std::string> choiceOptions;
	std::function<void(int)> onChoiceResult;

	std::string npcNickname;
	std::vector<std::string> npcPortraitLines;
};
