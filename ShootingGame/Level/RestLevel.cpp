#include "RestLevel.h"
#include <Actor/Player.h>
#include <Actor/Wall.h>
#include <Actor/Box.h>
#include <Actor/Ground.h>
#include <Actor/Target.h>
#include <cassert>
#include <Camera/Camera.h>
#include <Render/Renderer.h>
#include <Actor/Actor.h>
#include <Manager/UIManager.h>
#include <Manager/DialogueManager.h>
#include <Manager/NPCManager.h>
#include <Manager/QuestManager.h>
#include <Manager/SaveManager.h>
#include <Actor/NPC.h>

using namespace Craft;

void RestLevel::OnInitialized()
{
	super::OnInitialized();

	// 맵을 읽기 전에 NPC 데이터(대사, 초상화)와 퀘스트 데이터를 먼저 등록해둔다.
	// 맵 파일의 'N' 문자를 만나면 등록된 순서(0,1,2...)대로 NPC 액터를 생성한다.
	// (이전에는 여기서 RegisterNPC를 직접 하드코딩해서 호출했는데, Assets/NPCs.txt / Assets/Q1.txt
	//  텍스트 파일에서 읽어오도록 데이터 기반으로 바꿨다.)
	NPCManager::Get().LoadFromFile("NPCs.txt");
	QuestManager::Get().LoadQuestFromFile(1, "Q1.txt");
	QuestManager::Get().LoadQuestFromFile(2, "Q2.txt");
	QuestManager::Get().LoadQuestFromFile(3, "Q3.txt");

		LoadMap("RestArea_Sokoban.txt");

		//mapWidth = mapsize.x;
		//mapHeight = mapsize.y;

		Camera::Get().SetMapBounds(90, 40);
		Renderer::Get().SetOutlineVisible(true);

	// --- 세이브 데이터 복원 ---
	// SaveManager에 로드된 데이터가 있으면 플레이어/박스 위치를 덮어쓴다.
	if (SaveManager::Get().HasLoadedData())
	{
		// 플레이어 위치 복원.
		const Craft::Vector2& savedPos = SaveManager::Get().GetSavedPlayerPos();
		if (player && (savedPos.x != 0 || savedPos.y != 0))
		{
			player->SetPosition(savedPos);
		}

		// 박스 위치 복원. 세이브 파일의 박스 수와 맵의 박스 수가 같을 때만 적용.
		const auto& savedBoxes = SaveManager::Get().GetSavedBoxPositions();
		if (savedBoxes.size() == boxList.size())
		{
			for (size_t i = 0; i < boxList.size(); ++i)
			{
				boxList[i]->SetPosition(savedBoxes[i]);
			}
		}

		// 소코반 진행도를 복원된 박스 위치 기준으로 다시 계산.
		ReportSokobanProgress();

		// 플래그를 꺼서 다음 레벨 전환 시 중복 적용을 방지한다.
		SaveManager::Get().ClearLoadedFlag();
	}
}

void RestLevel::Tick(float deltaTime)
{
	super::Tick(deltaTime);
	
	const Viewport& gameViewport = Renderer::Get().GetViewport(RenderSpace::Game); // 저번에 추가한 뷰포트 크기 getter

	Camera::Get().FollowTarget(player->GetPosition(), gameViewport.size.x, gameViewport.size.y);

	UIManager::Get().Tick(deltaTime);
	DialogueManager::Get().Tick(deltaTime);
}

void RestLevel::Draw()
{
	super::Draw();

	UIManager::Get().Draw();
}

RestLevel::RestLevel()
{
}

std::shared_ptr<Craft::Actor> RestLevel::FindNPCAt(const Craft::Vector2& position) const
{
	for (const auto& npc : npcList)
	{
		if (npc->GetPosition() == position)
		{
			return npc;
		}
	}

	return nullptr;
}

bool RestLevel::CanMove(const Craft::Vector2& playerPosition, const Craft::Vector2& nextPosition)
{
	if (isGameClear) return false;
	if (UIManager::Get().IsShowing()) return false;

	// 정적 타일 조회 - O(1)
	auto tile = tileMap.find(EncodeTilePos(nextPosition.x, nextPosition.y));
	if (tile == tileMap.end() || tile->second.isWall)
	{
		return false;
	}

	// 박스는 개수가 적으니 선형 탐색해도 충분히 빠름
	std::shared_ptr<Craft::Actor> boxActor = nullptr;
	for (const auto& box : boxList)
	{
		if (box->GetPosition() == nextPosition)
		{
			boxActor = box;
			CheckGameClear();
			break;
		}
	}

	if (!boxActor)
	{
		return true; // 벽도 아니고 박스도 없으면 바로 이동 가능
	}

	Vector2 direction = nextPosition - playerPosition;
	Vector2 pushPosition = boxActor->GetPosition() + direction;

	auto pushTile = tileMap.find(EncodeTilePos(pushPosition.x, pushPosition.y));
	if (pushTile == tileMap.end() || pushTile->second.isWall)
	{
		return false;
	}

	for (const auto& other : boxList)
	{
		if (other != boxActor && other->GetPosition() == pushPosition)
		{
			return false;
		}
	}

	boxActor->SetPosition(pushPosition);
	ReportSokobanProgress();
	return true;
}

int RestLevel::CountPlacedBoxes() const
{
	int count = 0;
	for (const auto& box : boxList)
	{
		for (const auto& target : targetList)
		{
			if (box->GetPosition() == target->GetPosition())
			{
				++count;
				break;
			}
		}
	}
	return count;
}

void RestLevel::ReportSokobanProgress()
{
	QuestManager::Get().ReportSokoban(CountPlacedBoxes(), targetScore);
}

void RestLevel::LoadMap(const std::string& filename)
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

	// 읽은 데이터를 기반으로 로직 제작.
	// 1. 화면에 액터를 그리기.

	// 문자열에 저장된 값을 접근할 때 사용할 인덱스.
	int index = 0;
	int npcId = 0;

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

		// 읽은 문자 별로 처리.
		switch (mapCharacter)
		{
		case '#':
			SpawnActor<Wall>(position);
			tileMap[EncodeTilePos(position.x, position.y)] = { true, false };
			break;

		case '.':
			SpawnActor<Ground>(position);
			tileMap[EncodeTilePos(position.x, position.y)] = { false, true };
			break;

		case 'T':
		{
			auto target = SpawnActor<Target>(position);
			targetList.emplace_back(target);
			tileMap[EncodeTilePos(position.x, position.y)] = { false, true };
			++targetScore;
			break;
		}

		case 'B':
		{
			SpawnActor<Ground>(position);
			tileMap[EncodeTilePos(position.x, position.y)] = { false, true };
			auto box = SpawnActor<Box>(position);
			boxList.emplace_back(box); // 박스는 여기서만 추가 관리
			break;
		}

		case 'P':
			player = SpawnActor<Player>(position);
			SpawnActor<Ground>(position);
			tileMap[EncodeTilePos(position.x, position.y)] = { false, true };
			break;

		case 'N':
		{
			// NPC가 서 있는 칸은 플레이어가 통과/겹치지 못하도록 벽처럼 막아둔다.
			// (인접한 칸에서만 상호작용하도록 하기 위함)
			SpawnActor<Ground>(position);
			tileMap[EncodeTilePos(position.x, position.y)] = { true, false };

			// npcId 0번으로 고정 (OnInitialized에서 등록한 데이터를 그대로 사용).
			// NPC가 여러 마리 필요하면 맵 문자를 'N','M' 등으로 늘리고 각각 다른 id로 등록하면 된다.
			auto npc = SpawnActor<NPC>(position, npcId);
			npcList.emplace_back(npc);
			npcId++;
			break;
		}
		}

		// x 위치 업데이트.
		++position.x;
	}


	// 모두 사용한 버퍼 해제
	delete[] buffer;
	buffer = nullptr;

	//파일 닫기
	fclose(file);
}

std::vector<std::string> RestLevel::LoadImage(const std::string& filename)
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
	std::vector<std::string> lines;
	std::string line = "";

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
			lines.push_back(line);
			line.clear();
			continue;
		}

		line += mapCharacter;
		
		// x 위치 업데이트.
		++position.x;
	}


	// 모두 사용한 버퍼 해제
	delete[] buffer;
	buffer = nullptr;

	//파일 닫기
	fclose(file);

	return lines;
}

bool RestLevel::CheckGameClear()
{
	// 점수 확인용 변수.
	int currentScore = 0;

	// 하고 싶은 일 : 박스가 타겟 위치에 모두 배치됐는지 확인.

	// 박스 목록/타겟 목록 저장.
	std::vector<std::shared_ptr<Craft::Actor>> boxList;
	std::vector<std::shared_ptr<Craft::Actor>> targetList;

	// 게임 레벨의 모든 액터를 순회하면서 박스와 타겟 목록에 저장.
	for (const std::shared_ptr<Craft::Actor>& actor : actorList)
	{
		// 박스인 경우 박스 목록에 추가.
		if (actor->IsTypeOf<Box>())
		{
			boxList.emplace_back(actor);
			continue;
		}

		// 타겟인 경우 타겟 목록에 추가.
		if (actor->IsTypeOf<Target>())
		{
			targetList.emplace_back(actor);
		}
	}

	// 목표지점에 배치된 박스 수 확인.
	for (const std::shared_ptr<Craft::Actor>& box : boxList)
	{
		for (const std::shared_ptr<Craft::Actor>& target : targetList)
		{
			if (box->GetPosition() == target->GetPosition())
			{
				currentScore += 1;
			}
		}
	}


	// 목표 지점에 배치된 박스의 수가 타겟 수(목표 점수)와 같은지 확인.
	return currentScore == targetScore;
}

bool RestLevel::SaveGame()
{
	Craft::Vector2 playerPos;
	if (player)
	{
		playerPos = player->GetPosition();
	}

	std::vector<Craft::Vector2> boxPositions;
	for (const auto& box : boxList)
	{
		if (box)
		{
			boxPositions.push_back(box->GetPosition());
		}
	}

	return SaveManager::Get().Save(playerPos, boxPositions);
}
