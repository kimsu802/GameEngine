#include "SurvivorLevel.h"
#include <Actor/SurvivorPlayer.h>
#include <Actor/SurvivorBullet.h>
#include <Actor/SurvivorEnemy.h>
#include <Actor/DestroyEffect.h>
#include <Camera/Camera.h>
#include <Render/Renderer.h>
#include <Engine/Engine.h>
#include <Input/Input.h>
#include <State/PlayerState.h>
#include <Manager/UIManager.h>
#include <Manager/QuestManager.h>
#include <Util/Util.h>
#include <Level/RestLevel.h>
#include <algorithm>
#include <cmath>

using namespace Craft;

static const char* kEnemyImages[] =
{
	"x", "oOo", "<X>", "zWz", "^V^", "dQb",
};
static const int kEnemyImageCount = sizeof(kEnemyImages) / sizeof(kEnemyImages[0]);

SurvivorLevel::SurvivorLevel()
{
}

void SurvivorLevel::OnInitialized()
{
	super::OnInitialized();

	PlayerState::Get().FullHeal();

	// 플레이어 스폰.
	Vector2 startPos(mapWidth / 2, mapHeight / 2);
	player = SpawnActor<SurvivorPlayer>(startPos);

	// --- 오브젝트 풀 예열 ---
	// 총알: Lv3(8방향) × 발사 주기 0.35초 × 수명 2초 ≈ 46발이 동시 활성화.
	// 넉넉하게 200발 풀을 만든다. 부족하면 자동 확장된다.
	bulletPool.Prewarm(shared_from_this(), 200);
	player->SetBulletPool(&bulletPool);

	// 적: 후반 웨이브에서 최대 10마리/0.3초 = 초당 33마리.
	// 수명(화면 통과)을 감안하면 동시 100마리 정도. 넉넉하게 150.
	enemyPool.Prewarm(shared_from_this(), 150);

	// --- 공간 분할 그리드 초기화 ---
	// 셀 크기 5: 충돌 판정 범위(총알 폭 1 + 적 폭 3 = 4)보다 크면서
	// 너무 크지 않아 불필요한 비교를 줄이는 값.
	grid.Init(mapWidth, mapHeight, 5);

	// --- 웨이브 데이터 로드 ---
	waveManager.LoadFromFile("WaveConfig.txt");
	const WaveEntry& firstWave = waveManager.GetCurrentWave(0.f);
	spawnTimer.SetTargetTime(firstWave.spawnInterval);

	Camera::Get().SetMapBounds(mapWidth, mapHeight);
	Renderer::Get().SetOutlineVisible(true);
}

void SurvivorLevel::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	UIManager::Get().Tick(deltaTime);

	if (isGameOver)
	{
		gameOverTimer += deltaTime;
		if (gameOverTimer >= gameOverDelay)
		{
			Engine::Get().AddNewLevel<RestLevel>();
		}
		return;
	}

	if (PlayerState::Get().GetHp() <= 0)
	{
		isGameOver = true;
		gameOverTimer = 0.f;
		return;
	}

	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		Engine::Get().AddNewLevel<RestLevel>();
		return;
	}

	elapsedTime += deltaTime;

	// 카메라 추적.
	const Viewport& gv = Renderer::Get().GetViewport(RenderSpace::Game);
	Camera::Get().FollowTarget(player->GetPosition(), gv.size.x, gv.size.y);

	// --- 공간 분할 기반 충돌 처리 ---
	// 엔진의 O(N²) CollisionSystem이 돌기 전에 먼저 처리한다.
	// 이미 비활성화된 액터는 엔진 CollisionSystem에서 건너뛰므로 이중 처리가 발생하지 않는다.
	ProcessGridCollision();

	// --- 웨이브 기반 스폰 ---
	const WaveEntry& wave = waveManager.GetCurrentWave(elapsedTime);
	spawnTimer.SetTargetTime(wave.spawnInterval);

	spawnTimer.Tick(deltaTime);
	if (spawnTimer.IsTimeOut())
	{
		spawnTimer.Reset();

		for (int i = 0; i < wave.spawnCount; ++i)
		{
			SpawnEnemy();
		}
	}
}

void SurvivorLevel::ProcessGridCollision()
{
	// 1) 그리드 재구축.
	grid.Clear();

	for (const auto& actor : actorList)
	{
		if (!actor || !actor->IsActive()) continue;

		// 풀링 액터는 IsActive()로 걸러지므로 추가 검사 불필요.
		grid.Insert(actor);
	}

	// 2) 총알 → 적 충돌. 각 총알의 위치 셀에서 적만 찾아 비교한다.
	//    O(N_bullet × K_neighbors) 이며 K는 셀 당 평균 적 수 (보통 0~3).
	for (const auto& actor : actorList)
	{
		if (!actor || !actor->IsActive()) continue;
		if (!actor->IsTypeOf<SurvivorBullet>()) continue;

		auto bullet = Cast<SurvivorBullet>(actor);
		if (!bullet || !bullet->IsPoolActive()) continue;

		Vector2 bPos = bullet->GetPosition();
		auto neighbors = grid.QueryByType<SurvivorEnemy>(bPos.x, bPos.y);

		for (auto& enemy : neighbors)
		{
			if (!enemy || !enemy->IsPoolActive()) continue;

			// 간단한 AABB: y 같고 x 범위 겹침.
			Vector2 ePos = enemy->GetPosition();
			if (bPos.y != ePos.y) continue;

			int eWidth = enemy->GetWidth();
			if (bPos.x >= ePos.x && bPos.x < ePos.x + eWidth)
			{
				// 충돌!
				enemy->TakeDamage(bullet->GetDamage());
				bullet->Deactivate();

				if (enemy->IsDead())
				{
					PlayerState::Get().AddGold(enemy->GetGoldReward());
					QuestManager::Get().ReportKill();
					SpawnActor<DestroyEffect>(ePos);
					enemy->Deactivate();
				}

				break; // 이 총알은 이미 소모됨.
			}
		}
	}

	// 3) 플레이어 → 적 충돌.
	if (player && player->IsActive())
	{
		Vector2 pPos = player->GetPosition();
		auto enemies = grid.QueryByType<SurvivorEnemy>(pPos.x, pPos.y);

		for (auto& enemy : enemies)
		{
			if (!enemy || !enemy->IsPoolActive()) continue;

			Vector2 ePos = enemy->GetPosition();
			if (pPos.y != ePos.y) continue;

			int eWidth = enemy->GetWidth();
			if (pPos.x >= ePos.x && pPos.x < ePos.x + eWidth)
			{
				PlayerState::Get().TakeDamage(enemy->GetContactDamage());
				SpawnActor<DestroyEffect>(ePos);
				enemy->Deactivate();
			}
		}
	}
}

void SurvivorLevel::Draw()
{
	super::Draw();

	DrawDungeonStatus();
	UIManager::Get().Draw();

	if (isGameOver)
	{
		Vector2 offset = Camera::Get().GetOffset();
		const Viewport& gv = Renderer::Get().GetViewport(RenderSpace::Game);
		int cx = gv.size.x / 2 - 5 + offset.x;
		int cy = gv.size.y / 2 + offset.y;
		Renderer::Get().Submit("GAME  OVER", Vector2(cx, cy), Color::Red, 100, RenderSpace::Game);
	}
}

Vector2 SurvivorLevel::GetRandomSpawnPosition() const
{
	const Viewport& gv = Renderer::Get().GetViewport(RenderSpace::Game);
	Vector2 pPos = player ? player->GetPosition() : Vector2(mapWidth / 2, mapHeight / 2);

	int halfW = gv.size.x / 2 + 5;
	int halfH = gv.size.y / 2 + 5;

	int side = Util::RandomRange(0, 3);
	int x = 0, y = 0;

	switch (side)
	{
	case 0: x = Util::RandomRange(pPos.x - halfW, pPos.x + halfW); y = pPos.y - halfH; break;
	case 1: x = Util::RandomRange(pPos.x - halfW, pPos.x + halfW); y = pPos.y + halfH; break;
	case 2: x = pPos.x - halfW; y = Util::RandomRange(pPos.y - halfH, pPos.y + halfH); break;
	case 3: x = pPos.x + halfW; y = Util::RandomRange(pPos.y - halfH, pPos.y + halfH); break;
	}

	return Vector2(std::clamp(x, 1, mapWidth - 2), std::clamp(y, 1, mapHeight - 2));
}

void SurvivorLevel::SpawnEnemy()
{
	const WaveEntry& wave = waveManager.GetCurrentWave(elapsedTime);

	int imgIdx = (wave.enemyType >= 0 && wave.enemyType < kEnemyImageCount)
		? wave.enemyType
		: Util::RandomRange(0, kEnemyImageCount - 1);

	Vector2 pos = GetRandomSpawnPosition();

	// 풀에서 꺼내 재활용한다.
	SurvivorEnemy* enemy = enemyPool.Acquire();
	if (enemy)
	{
		enemy->Reset(kEnemyImages[imgIdx], pos,
			wave.enemyHp, wave.enemySpeed, wave.contactDamage, wave.goldReward);
	}
}

void SurvivorLevel::DrawDungeonStatus()
{
	int y = 2;

	std::string hpText = "HP : " + std::to_string(PlayerState::Get().GetHp())
		+ " / " + std::to_string(PlayerState::Get().GetMaxHp());
	Renderer::Get().Submit(hpText, Vector2(2, y), Color::Yellow, 1, RenderSpace::UI);
	++y;

	Renderer::Get().Submit("GOLD : " + std::to_string(PlayerState::Get().GetGold()),
		Vector2(2, y), Color::Yellow, 1, RenderSpace::UI);
	++y;

	Renderer::Get().Submit("ATK : " + std::to_string(PlayerState::Get().GetAttackPower()),
		Vector2(2, y), Color::Yellow, 1, RenderSpace::UI);
	++y;

	Renderer::Get().Submit("Weapon Lv : " + std::to_string(PlayerState::Get().GetWeaponLevel()),
		Vector2(2, y), Color::Cyan, 1, RenderSpace::UI);
	++y;
	++y;

	int minutes = static_cast<int>(elapsedTime) / 60;
	int seconds = static_cast<int>(elapsedTime) % 60;
	char timeBuf[16];
	snprintf(timeBuf, sizeof(timeBuf), "TIME  %02d:%02d", minutes, seconds);
	Renderer::Get().Submit(timeBuf, Vector2(2, y), Color::White, 1, RenderSpace::UI);
	++y;

	int waveIdx = waveManager.GetCurrentWaveIndex(elapsedTime);
	Renderer::Get().Submit("WAVE : " + std::to_string(waveIdx),
		Vector2(2, y), Color::Red, 1, RenderSpace::UI);
	++y;
	++y;

	// 풀 상태 (포트폴리오 디버그 표시용).
	Renderer::Get().Submit("Pool B:" + std::to_string(bulletPool.GetActiveCount())
		+ "/" + std::to_string(bulletPool.GetTotalCount())
		+ " E:" + std::to_string(enemyPool.GetActiveCount())
		+ "/" + std::to_string(enemyPool.GetTotalCount()),
		Vector2(2, y), Color::White, 1, RenderSpace::UI);
	++y;
	++y;

	Renderer::Get().Submit("[Arrows] Move", Vector2(2, y), Color::White, 1, RenderSpace::UI);
	++y;
	Renderer::Get().Submit("[ESC] Return", Vector2(2, y), Color::White, 1, RenderSpace::UI);
}
