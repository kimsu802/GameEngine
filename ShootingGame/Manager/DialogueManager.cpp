#include "DialogueManager.h"
#include <cassert>
#include <Render/Renderer.h>
#include <System/StringUtil.h>

using namespace Craft;


DialogueManager::DialogueManager()
	:elapsedTime(0.f), typeInterval(0.05f), dialogueTexts(""), currentDialogueIndex(-1)
{

}

DialogueManager::~DialogueManager()
{
}

DialogueManager& DialogueManager::Get()
{
	static DialogueManager instance;
	return instance;
}

void DialogueManager::StartDialogue(const std::vector<std::string>& dialogues, const std::string& speakerNickname)
{
	if (dialogues.empty())
	{
		return;
	}

	dialogueLines = dialogues;
	this->speakerNickname = speakerNickname;
	isPlaying = true;

	BeginLine(0);
}

void DialogueManager::BeginLine(int lineIndex)
{
	currentDialogueIndex = lineIndex;
	currentDialogueLine = dialogueLines[lineIndex];

	// 이전 줄에서 타이핑되던 내용이 다음 줄에 이어붙는 버그가 있었기 때문에
	// 줄이 바뀔 때마다 반드시 초기화한다.
	dialogueTexts.clear();
	revealedCharCount = 0;
	elapsedTime = 0.f;
}

void DialogueManager::NextDialogueLine()
{
	if (dialogueLines.empty())
	{
		return;
	}

	int nextIndex = currentDialogueIndex + 1;

	if (nextIndex >= static_cast<int>(dialogueLines.size()))
	{
		EndDialogue();
		return;
	}

	BeginLine(nextIndex);
}

void DialogueManager::AdvanceOrSkip()
{
	if (!isPlaying)
	{
		return;
	}

	if (!IsCurrentLineFullyRevealed())
	{
		// 타이핑 중이면 즉시 전체 문장을 보여준다 (스킵).
		revealedCharCount = static_cast<int>(currentDialogueLine.size());
		dialogueTexts = currentDialogueLine;
		return;
	}

	// 이미 다 보여준 상태라면 다음 줄로 진행.
	NextDialogueLine();
}

void DialogueManager::EndDialogue()
{
	bool wasPlaying = isPlaying;

	isPlaying = false;
	dialogueLines.clear();
	currentDialogueLine.clear();
	dialogueTexts.clear();
	revealedCharCount = 0;
	currentDialogueIndex = -1;
	speakerNickname.clear();

	// 콜백을 로컬로 옮겨서 실행 후 즉시 비운다.
	// (onDialogueFinished를 지우지 않으면, 이후 어떤 이유로 EndDialogue가
	// 다시 호출될 때 이전 대화의 콜백이 한 번 더 실행될 수 있다.)
	DialogueFinished callback = onDialogueFinished;
	onDialogueFinished = nullptr;

	// std::function이 비어있는 상태에서 호출하면 std::bad_function_call로 크래시하므로
	// 반드시 콜백이 설정되어 있는지 확인한다.
	if (wasPlaying && callback)
	{
		callback();
	}
}

void DialogueManager::Tick(float deltaTime)
{
	if (!isPlaying)
		return;

	if (currentDialogueLine.empty())
		return;

	// 콘솔 표시 폭 기준으로 가운데 정렬 좌표를 계산한다.
	// (std::string::length()는 UTF-8 바이트 수를 돌려주므로
	//  한글 등 CJK 문자가 섞이면 위치가 어긋난다.)
	int viewportWidth = Renderer::Get().GetViewport(RenderSpace::Dialogue).size.x;
	int centralposition = StringUtil::GetCenteredX(viewportWidth, dialogueTexts);

	if (IsCurrentLineFullyRevealed())
	{
		Renderer::Get().Submit(dialogueTexts, Vector2(centralposition, 1), Color::White, 1, RenderSpace::Dialogue);
		return;
	}

	elapsedTime += deltaTime;

	if (elapsedTime > typeInterval)
	{
		// UTF-8 문자 단위로 한 글자를 추출하여 추가한다.
		// (바이트 단위로 접근하면 한글 등 멀티바이트 문자가 깨진다.)
		if (revealedCharCount < static_cast<int>(currentDialogueLine.size()))
		{
			unsigned char c = static_cast<unsigned char>(currentDialogueLine[revealedCharCount]);
			int charBytes = 1;
			if (c < 0x80)        charBytes = 1;
			else if ((c & 0xE0) == 0xC0) charBytes = 2;
			else if ((c & 0xF0) == 0xE0) charBytes = 3;
			else if ((c & 0xF8) == 0xF0) charBytes = 4;

			dialogueTexts += currentDialogueLine.substr(revealedCharCount, charBytes);
			revealedCharCount += charBytes;
		}
		elapsedTime = 0.f;
	}

	Renderer::Get().Submit(dialogueTexts, Vector2(centralposition, 1), Color::White, 1, RenderSpace::Dialogue);
}
