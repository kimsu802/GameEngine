#include "MenuLevel.h"
#include <cassert>
#include <Actor/Actor.h>
#include <Render/Renderer.h>
#include <Actor/Game.h>
#include <Input/Input.h>
#include <Manager/SaveManager.h>
#include <Level/RestLevel.h>

using namespace Craft;

MenuLevel::MenuLevel()
{
	// 메뉴 아이템 생성
	itemList.emplace_back(
		std::make_unique<MenuItem>(
			"Start Game",
			[]()
			{
				// 메뉴 토글 함수 호출
				Game& game = dynamic_cast<Game&> (Engine::Get());
				game.ToggleMenu();
			}
		));

	// 세이브 파일이 존재하면 "Load SaveData" 메뉴를 추가한다.
	if (SaveManager::SaveFileExists())
	{
		itemList.emplace_back(
			std::make_unique<MenuItem>(
				"Load SaveData",
				[]()
				{
					// 세이브 데이터를 로드하고 RestLevel로 전환한다.
					SaveManager::Get().Load();
					Engine::Get().AddNewLevel<RestLevel>();
				}
			));
	}

	itemList.emplace_back(
		std::make_unique<MenuItem>(
			"Exit Game",
			[]()
			{
				// 게임 종료 함수
				Engine::Get().Quit();
			}
		));
}

void MenuLevel::OnInitialized()
{
	super::OnInitialized();

	Renderer::Get().SetOutlineVisible(false);

	LoadTitle("Title.txt");
}

void MenuLevel::LoadTitle(const std::string& filename)
{
	////최종 경로 조립.
	std::string path = std::string("../Assets/") + filename;

	//파일 열기(C-Style)
	FILE* file = nullptr;
	fopen_s(&file, path.c_str(), "rt");

	if (!file)
	{
		assert(false && "failed to open a sokoban stage file.");
	}

	// 파일의 내용을 저장할 버퍼(데이터 저장공간) 확인.
	// 파일 길이 확인
	// -> 파일 위치를 제일 뒤로 이동 시킨 다음, 해당 위치 값 읽기.
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);

	// 파일 제일 끝 위치를 구한 다음에는 다시 처음으로 되돌리기
	rewind(file);

	// 앞에서 구한 위치를 사용해서 버퍼 생성
	char* buffer = new char[fileSize];

	// 데이터 읽기(파일 읽기)
	size_t readSize = fread(buffer, sizeof(char), fileSize, file);

	assert(readSize > 0 && "No Data in the stage file !");

	int index = 0;

	// 액터 생성에 사용할 위치 값.
	Vector2 position;

	while (true)
	{
		// 종료 조건 - 내용을 모두 읽었는지 파악.
		if (index >= fileSize)
		{
			break;
		}

		// 이번에 확인할 문자 값.
		char mapCharacter = buffer[index];

		// 인덱스 증가 처리.
		++index;

		// 현재 문자가 개행 문자라면 로직은 건너뛰고,
		// 위치 값만 설정.
		if (mapCharacter == '\n')
		{
			++position.y;
			position.x = 0;
			continue;
		}
		
		std::string chr(1,mapCharacter);

		SpawnActor<Craft::Actor>(chr, position, Color::White,RenderSpace::Full);

		// x 위치 업데이트.
		++position.x;
	}


	// 모두 사용한 버퍼 해제
	delete[] buffer;
	buffer = nullptr;

	//파일 닫기
	fclose(file);
}




void MenuLevel::Tick(float deltaTime)
{
	Level::Tick(deltaTime);

	// 입력 처리(위/아래 방향기, 엔터, ESC 키)
	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		Game& game = dynamic_cast<Game&>(Engine::Get());
		game.ToggleMenu();

		// 인덱스 초기화
		currentIndex = 0;
	}

	// 배열의 요소 개수
	const int length = static_cast<int>(itemList.size());
	if (Input::Get().GetKeyDown(VK_UP))
	{
		// 인덱스 돌리기 (-방향)
		currentIndex = (currentIndex - 1 + length) % length;
	}

	if (Input::Get().GetKeyDown(VK_DOWN))
	{
		// 인덱스 돌리기 (+방향)
		currentIndex = (currentIndex + 1) % length;
	}

	// 엔터 입력 처리 -> 현재 선택된 메누의 로직 실행
	if (Input::Get().GetKeyDown(VK_RETURN))
	{
		// assert
		assert(currentIndex >= 0 &&
			currentIndex < (int)itemList.size() &&
			itemList[currentIndex]->onSelected);

		//메뉴 아이템에 저장된 로직 실행
		itemList[currentIndex]->onSelected();
	}

}

void MenuLevel::Draw()
{
	super::Draw();

	// 메뉴 아이템 그리기
	const int count = static_cast<int>(itemList.size());

	for (int ix = 0; ix < count; ++ix)
	{
		// 선택/미선택된 아이템 색상 처리
		Color textColor = (ix == currentIndex) ? selectedColor : unselectedColor;

		// 화면 뷰포트 가져오기
		const Viewport& fullViewport = Renderer::Get().GetViewport(RenderSpace::Full);

		const Vector2& viewportPos = fullViewport.origin;
		const Vector2& viewportSize = fullViewport.size;

		int xCentralPos = (viewportSize.x / 2) - (itemList[ix]->text.length() / 2);
		int yCentralPos = viewportPos.y + (viewportSize.y / 2) - (itemList[ix]->text.length() / 2);
		int offset = 4;

		// 아이템 그리기
		Renderer::Get().Submit(itemList[ix]->text, Vector2(xCentralPos, yCentralPos + offset + ix), textColor);

	}
}