#pragma once
#include <string>
#include <functional>
#include <vector>

class DialogueManager
{
	using DialogueFinished = std::function<void()>;

public:
	DialogueManager();
	~DialogueManager();

	static DialogueManager& Get();

	// 대화 시작. speakerNickname은 UI 상단부에 표시할 이름.
	void StartDialogue(const std::vector<std::string>& dialogues, const std::string& speakerNickname = "");

	// 현재 라인에 등록된 완료 콜백 (대화가 전부 끝났을 때 1회 호출).
	void SetOnDialogueFinished(const DialogueFinished& callback) { onDialogueFinished = callback; }

	// 다음 줄로 넘어간다. 더 이상 줄이 없으면 대화를 종료한다.
	void NextDialogueLine();

	// 타이핑 도중이면 즉시 전체 텍스트를 표시(스킵)한다. 이미 다 표시된 상태면 다음 줄로 넘어간다.
	// Player의 상호작용 키 입력에서 호출하도록 만들어졌다.
	void AdvanceOrSkip();

	// 대화를 강제 종료한다.
	void EndDialogue();

	void Tick(float deltaTime);

	inline bool GetIsPlaying() { return isPlaying; }
	inline const std::string& GetSpeakerNickname() const { return speakerNickname; }

private:
	// 현재 표시 중인 줄의 타이핑 상태를 초기화하고 새로 시작한다.
	void BeginLine(int lineIndex);

	inline bool IsCurrentLineFullyRevealed() const
	{
		return currentDialogueLine.empty() || revealedCharCount >= static_cast<int>(currentDialogueLine.size());
	}

private:
	std::vector<std::string> dialogueLines;
	std::string currentDialogueLine;

	// 현재 줄에서 화면에 표시된 텍스트 (타이핑 효과 결과물).
	std::string dialogueTexts = "";

	// 현재 줄에서 몇 글자까지 공개되었는지.
	int revealedCharCount = 0;

	// 현재 재생 중인 줄의 인덱스. dialogueLines[currentDialogueIndex] 가 currentDialogueLine.
	int currentDialogueIndex = -1;

	bool isPlaying = false;
	float typeInterval = 0.05f;
	float elapsedTime = 0.f;

	// 대화 상대 이름 (UI 표시용).
	std::string speakerNickname;

	DialogueFinished onDialogueFinished;
};
