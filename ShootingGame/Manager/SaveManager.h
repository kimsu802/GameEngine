#pragma once

#include <string>
#include <vector>
#include <Math/Vector2.h>

// ==========================================
//  SaveManager — 세이브/로드 시스템
// ==========================================
//
// 저장 대상:
//   1) PlayerState  : HP, MaxHP, Gold, AttackPower, WeaponLevel
//   2) 퀘스트 진행도 : questId, state, currentCount (모든 퀘스트)
//   3) 맵 상태      : 플레이어 위치, 박스 위치들
//
// 파일 포맷 (Assets/Save.txt):
//   HP: 80
//   MAXHP: 120
//   GOLD: 350
//   ATK: 3
//   WEAPON: 2
//   TRACK: 2
//   QUEST: 1 2 20          (questId, state, currentCount)
//   QUEST: 2 1 3
//   PLAYERPOS: 15 10       (x, y)
//   BOX: 22 8              (x, y) — 박스 개수만큼 반복
//   BOX: 25 12
//
class SaveManager
{
public:
	static SaveManager& Get();

	// 현재 상태를 파일에 저장한다.
	// playerPos : 플레이어의 현재 위치.
	// boxPositions : 소코반 박스들의 현재 위치.
	bool Save(const Craft::Vector2& playerPos,
		const std::vector<Craft::Vector2>& boxPositions,
		const std::string& path = "../Assets/Save.txt");

	// 파일에서 상태를 읽어 PlayerState를 복원한다.
	// 맵 상태(플레이어 위치, 박스 위치)는 멤버 변수에 캐싱해 둔다.
	// RestLevel::OnInitialized()에서 이 값을 가져가 액터 위치를 덮어쓴다.
	bool Load(const std::string& path = "../Assets/Save.txt");

	// 세이브 파일이 존재하는지 확인한다.
	static bool SaveFileExists(const std::string& path = "../Assets/Save.txt");

	// Load() 후 캐싱된 맵 상태 접근자.
	inline const Craft::Vector2& GetSavedPlayerPos() const { return savedPlayerPos; }
	inline const std::vector<Craft::Vector2>& GetSavedBoxPositions() const { return savedBoxPositions; }

	// 로드 성공 여부. RestLevel이 이 플래그를 확인하고 박스/플레이어 위치를 덮어쓴다.
	inline bool HasLoadedData() const { return hasLoadedData; }

	// 로드 데이터를 사용했으면 플래그를 꺼서 다음 레벨 전환 시 중복 적용을 방지한다.
	inline void ClearLoadedFlag() { hasLoadedData = false; }

private:
	SaveManager() = default;

	Craft::Vector2 savedPlayerPos;
	std::vector<Craft::Vector2> savedBoxPositions;
	bool hasLoadedData = false;
};
