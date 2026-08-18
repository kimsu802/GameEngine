#pragma once
#include <Level/Level.h>
#include <unordered_map>

class Player;
class Actor;

class RestLevel : public Craft::Level
{
	TYPE_DECLARATIONS(RestLevel, Level)

	struct TileInfo
	{
		bool isWall = false;
		bool isWalkable = false;
	};

public:
	RestLevel();
	~RestLevel() = default;

	void LoadMap(const std::string& filename);
	std::vector<std::string> LoadImage(const std::string& filename);
	bool CanMove(const Craft::Vector2& playerPosition, const Craft::Vector2& nextPosition);

	// 현재 상태를 SaveManager를 통해 파일에 저장한다.
	bool SaveGame();


private:
	virtual void OnInitialized() override;
	virtual void Tick(float deltaTime) override;
	virtual void Draw() override;

	bool CheckGameClear();
	int CountPlacedBoxes() const;
	void ReportSokobanProgress();


private:
	int targetScore = 0;
	std::shared_ptr<Player> player;

	bool isGameClear = false;

	// 정적인 타일 조회
	std::unordered_map<int64_t, TileInfo> tileMap;

	// 박스는 이동하는 액터라 별도로 소수만 추적 (전체 순회 X)
	std::vector<std::shared_ptr<Craft::Actor>> boxList;

	// NPC 목록 (상호작용 대상 탐색용).
	std::vector<std::shared_ptr<Craft::Actor>> npcList;

	// 목표 지점(T) 목록 캐싱.
	std::vector<std::shared_ptr<Craft::Actor>> targetList;

public:
	// position과 같은 칸에 있는 NPC를 찾는다. 없으면 nullptr.
	std::shared_ptr<Craft::Actor> FindNPCAt(const Craft::Vector2& position) const;

private:

	// 비트 연산으로 하나의 key를 만들기
	static int64_t EncodeTilePos(int x, int y)
	{
		return (static_cast<int64_t>(y) << 32) | static_cast<uint32_t>(x);
	}

	int mapWidth;
	int mapHeight;

};

