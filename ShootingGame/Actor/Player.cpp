#include "Player.h"
#include <Engine/Engine.h>
#include <Input/Input.h>
#include <Level/Level.h>
#include <Actor/PlayerBullet.h>
#include <Actor/EnemyBullet.h>
#include <Actor/DestroyEffect.h>
#include <Render/Renderer.h>
#include <Level/RestLevel.h>
#include <Level/GameLevel.h>
#include <Level/SurvivorLevel.h>
#include <State/PlayerState.h>
#include <Manager/SaveManager.h>
#include <Actor/Game.h>
#include <Manager/UIManager.h>
#include <Camera/Camera.h>
#include <Manager/DialogueManager.h>
#include <Manager/NPCManager.h>
#include <Manager/QuestManager.h>
#include <Actor/NPC.h>


using namespace Craft;

Player::Player()
	: Actor("<=A=>",Vector2::Zero,Color::Green),
	fireMode(FireMode::OneShot)
{
	// 생성 위치 설정

	int x = (Engine::Get().GetGameWidth() / 2) - (width / 2);
	int y = (Engine::Get().GetGameHeight() - 2);
	SetPosition(Vector2(x, y));

	// x 위치 저장
	xPosition = static_cast<float>(x);
	yPosition = static_cast<float>(y);

	// 연사 타이머 시간 설정.
	timer.SetTargetTime(fireInterval);
}

Player::Player(const Vector2& position)
	:Actor("P", position, Color::Green),xPosition(GetPosition().x),yPosition(GetPosition().y)
{
	//우선순위 설정..
	//액터 중에서 가장 높은 값.
	sortingOrder = 10;

	// GameLevel에서 EnemyBullet과 충돌 판정이 필요하다.
	// RestLevel에서는 tileMap 기반 이동 처리를 쓰므로 이 플래그가 있어도
	// 바닥/벽 액터(isCollidable=false)와는 충돌하지 않는다.
	isCollidable = true;

	// 연사 타이머 시간 설정.
	timer.SetTargetTime(fireInterval);
}

void Player::Fire()
{
	// 탄약 생성 위치 구하기
	// 플레이어의 가운데 위치.
	Vector2 bulletPosition(GetPosition().x + (width / 2), GetPosition().y);

	// 탄약 생성
	std::shared_ptr<Level> owner = GetOwner();
	if (owner)
	{
		owner->SpawnActor<PlayerBullet>(bulletPosition);
	}

}

void Player::FireInterval()
{
	if (!CanShoot())
	{
		return;
	}

	// 발사 처리
	Fire();

	// 경과 시간 초기화.
	timer.Reset();
}

void Player::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	// ESC 키 종료 처리
	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		if (GetOwner())
		{
			
		}
	}

	// 방향키 입력에 따른 이동 방향 설정
	// 오른쪽 방향 : 1 | 왼쪽 방향 : -1

	float xdirection = 0.f;
	float ydirection = 0.f;

	if (Input::Get().GetKey(VK_RIGHT))
	{
		xdirection = 1.f;
	}
	if (Input::Get().GetKey(VK_LEFT))
	{
		xdirection = -1.f;
	}
	if (Input::Get().GetKey(VK_UP))
	{
		ydirection = -1.f;
	}
	if (Input::Get().GetKey(VK_DOWN))
	{
		ydirection = 1.f;
	}

	// 가장 최근 눌린 이동 방향으로 바라보는 방향(facing)을 갱신.
	// (대화 중에는 이동 입력이 무시되므로 대화 중엔 갱신하지 않는다.)
	if (!DialogueManager::Get().GetIsPlaying())
	{
		if (xdirection != 0.f || ydirection != 0.f)
		{
			facingDirection = Vector2(static_cast<int>(xdirection), static_cast<int>(ydirection));
		}
	}

	// 이동 함수 호출 (대화 중이면 Move 내부에서 무시됨)
	Move(xdirection, ydirection, deltaTime);

	// 발사 타이머 업데이트
	timer.Tick(deltaTime);

	//if (fireMode == FireMode::OneShot)
	//{
	//	if (Input::Get().GetKeyDown(VK_SPACE))
	//	{
	//		Fire();
	//	}

	//}	
	//// 연사 모드 처리
	//else if (fireMode == FireMode::Repeat)
	//{
	//	if (Input::Get().GetKey(VK_SPACE))
	//	{
	//		// 연사 발사 함수 호출
	//		FireInterval();
	//	}
	//}

	////발사 모드 전환 처리
	//if (Input::Get().GetKeyDown('R'))
	//{
	//	fireMode = fireMode == FireMode::Repeat ? FireMode::OneShot : FireMode::Repeat;
	//}

	// 상호작용 키 (Enter) : NPC와 대화 시작 / 대화 진행(스킵, 다음 줄).
	if (Input::Get().GetKeyDown(VK_RETURN))
	{
		HandleInteract();
	}
}

void Player::HandleInteract()
{
	// 이미 대화 중이면 : 타이핑 스킵 또는 다음 줄로 진행.
	if (DialogueManager::Get().GetIsPlaying())
	{
		DialogueManager::Get().AdvanceOrSkip();
		return;
	}

	// 대화 중이 아니면 : 바라보고 있는 칸에 NPC가 있는지 확인 후 대화 시작.
	std::shared_ptr<RestLevel> level = Cast<RestLevel>(GetOwner());
	if (!level)
	{
		return;
	}

	Vector2 targetPosition = GetPosition() + facingDirection;
	std::shared_ptr<Actor> npcActor = level->FindNPCAt(targetPosition);
	if (!npcActor)
	{
		return;
	}

	std::shared_ptr<NPC> npc = Cast<NPC>(npcActor);
	if (!npc)
	{
		return;
	}

	const int interactingNpcId = npc->GetNpcId();

	const NPCData* npcData = NPCManager::Get().FindNPCData(interactingNpcId);
	if (!npcData)
	{
		return;
	}

	// DIALOGUE 타입 퀘스트 진행도 보고.
	QuestManager::Get().ReportDialogue(interactingNpcId);

	// --- 퀘스트 처리 ---
	if (npcData->questId >= 0)
	{
		const QuestData* quest = QuestManager::Get().FindQuestData(npcData->questId);
		if (quest)
		{
			int questId = npcData->questId;
			const QuestState questState = QuestManager::Get().GetQuestState(questId);

			if (questState == QuestState::NotStarted && !quest->offerDialogueLines.empty())
			{
				UIManager::Get().SetInteractingNPC(npcData->nickname, npcData->portraitLines);
				DialogueManager::Get().StartDialogue(quest->offerDialogueLines, npcData->nickname);
				DialogueManager::Get().SetOnDialogueFinished([questId]()
				{
					QuestManager::Get().StartQuest(questId);
				});
				return;
			}

			if (questState == QuestState::InProgress)
			{
				if (QuestManager::Get().IsQuestCompletable(questId) && !quest->completeDialogueLines.empty())
				{
					UIManager::Get().SetInteractingNPC(npcData->nickname, npcData->portraitLines);
					DialogueManager::Get().StartDialogue(quest->completeDialogueLines, npcData->nickname);
					DialogueManager::Get().SetOnDialogueFinished([questId]()
					{
						QuestManager::Get().CompleteQuest(questId);
					});
					return;
				}
				if (!quest->inProgressDialogueLines.empty())
				{
					UIManager::Get().SetInteractingNPC(npcData->nickname, npcData->portraitLines);
					DialogueManager::Get().StartDialogue(quest->inProgressDialogueLines, npcData->nickname);
					return;
				}
			}
		}
	}

	// --- NPC 0 (시고르자브 이장님) 전용: 던전 이동 / 강화 선택지 ---
	if (interactingNpcId == 0)
	{
		UIManager::Get().SetInteractingNPC(npcData->nickname, npcData->portraitLines);

		UIManager::Get().ShowChoiceUI(
			"무엇을 하겠는가?",
			{ "던전 이동", "강화하기", "대화하기" },
			[npcData](int idx)
			{
				if (idx == 0) // 던전 이동
				{
					Engine::Get().AddNewLevel<SurvivorLevel>();
				}
				else if (idx == 1) // 강화하기
				{
					// 강화 선택지를 다시 보여준다.
					std::string hpLabel = "체력 강화 (50G)  HP+" + std::to_string(20);
					std::string atkLabel = "공격력 강화 (50G)  ATK+1";
					std::string wpnLabel = "무기 강화 (100G)  Lv" + std::to_string(PlayerState::Get().GetWeaponLevel());

					UIManager::Get().ShowChoiceUI(
						"무엇을 강화할까?",
						{ hpLabel, atkLabel, wpnLabel, "취소" },
						[](int upIdx)
						{
							if (upIdx == 0)
							{
								if (!PlayerState::Get().UpgradeMaxHp(50))
								{
									DialogueManager::Get().StartDialogue({ "골드가 부족하구먼.." });
								}
								else
								{
									DialogueManager::Get().StartDialogue({ "체력이 강화됐네 ! 든든하군 !" });
								}
							}
							else if (upIdx == 1)
							{
								if (!PlayerState::Get().UpgradeAttackPower(50))
								{
									DialogueManager::Get().StartDialogue({ "골드가 부족하구먼.." });
								}
								else
								{
									DialogueManager::Get().StartDialogue({ "공격력이 올랐군 ! 몬스터를 더 세게 때릴 수 있겠어 !" });
								}
							}
							else if (upIdx == 2)
							{
								if (PlayerState::Get().GetWeaponLevel() >= PlayerState::Get().GetMaxWeaponLevel())
								{
									DialogueManager::Get().StartDialogue({ "무기는 이미 최고 레벨이라네 !" });
								}
								else if (!PlayerState::Get().UpgradeWeapon(100))
								{
									DialogueManager::Get().StartDialogue({ "골드가 부족하구먼.." });
								}
								else
								{
									DialogueManager::Get().StartDialogue({ "무기가 강화됐네 ! 발사 패턴이 바뀔걸세 !" });
								}
							}
							// idx == 3 : 취소 → 아무것도 안 함.
						}
					);
				}
				else // 대화하기
				{
					if (!npcData->dialogueLines.empty())
					{
						DialogueManager::Get().StartDialogue(npcData->dialogueLines, npcData->nickname);
					}
				}
			}
		);
		return;
	}

	// --- NPC 1 (미네르바 부엉이) 전용: 세이브 ---
	if (interactingNpcId == 1)
	{
		UIManager::Get().SetInteractingNPC(npcData->nickname, npcData->portraitLines);

		UIManager::Get().ShowChoiceUI(
			"게임을 저장할까?",
			{ "저장하기", "대화하기" },
			[npcData](int idx)
			{
				if (idx == 0) // 저장하기
				{
					UIManager::Get().ShowChoiceUI(
						"현재 상태를 저장합니다. 진행하시겠습니까?",
						{ "YES", "NO" },
						[npcData](int confirmIdx)
						{
							if (confirmIdx == 0) // YES
							{
								// RestLevel의 SaveGame()을 호출하여
								// 플레이어 위치, 박스 위치, PlayerState를 한꺼번에 저장한다.
								Game& game = dynamic_cast<Game&>(Engine::Get());
								auto level = std::dynamic_pointer_cast<RestLevel>(
									game.GetMainLevel());
								if (level && level->SaveGame())
								{
									DialogueManager::Get().StartDialogue(
										{ "저장 완료 ! 안심하고 모험을 계속하렴." },
										npcData->nickname);
								}
								else
								{
									DialogueManager::Get().StartDialogue(
										{ "어라.. 저장에 실패했어. 다시 해볼래?" },
										npcData->nickname);
								}
							}
							// NO → 아무것도 안 함.
						}
					);
				}
				else // 대화하기
				{
					if (!npcData->dialogueLines.empty())
					{
						DialogueManager::Get().StartDialogue(npcData->dialogueLines, npcData->nickname);
					}
				}
			}
		);
		return;
	}

	// 일반 NPC : 평소 대사를 보여준다.
	if (npcData->dialogueLines.empty())
	{
		return;
	}

	UIManager::Get().SetInteractingNPC(npcData->nickname, npcData->portraitLines);
	DialogueManager::Get().StartDialogue(npcData->dialogueLines, npcData->nickname);
}

void Player::OnCollision(const std::shared_ptr<Actor>& other)
{
	super::OnCollision(other);

	// 부딪힌 액터가 적 탄약이면 처리.
	if (other->IsTypeOf<EnemyBullet>())
	{
		// 플레이어 제거.
		Destroy();

		// 적 탄약 제거.
		other->Destroy();

		// 파괴 이펙트 생성.
		if (GetOwner())
		{
			GetOwner()->SpawnActor<DestroyEffect>(GetPosition());

			// 게임 오버(게임 종료).
			QuitGame();
		}
	}
}

void Player::Move(float xdirection, float ydirection, float deltaTime)
{
	std::shared_ptr<RestLevel> level = Cast<RestLevel>(GetOwner());

	if (UIManager::Get().IsShowing())
		return;

	if (DialogueManager::Get().GetIsPlaying())
		return;

	// 이동 입력이 없으면 수행하지 않음
	if (xdirection == 0.f && ydirection == 0.f)
		return;

	// 1. 기존 실수 좌표 및 정수 위치 보존
	float prevX = xPosition;
	float prevY = yPosition;
	Vector2 currentGridPos = GetPosition();

	// 2. 새로운 실수 위치 계산
	xPosition += xdirection * moveSpeed * deltaTime;
	yPosition += ydirection * moveSpeed * deltaTime;

	// 화면 경계 제한
	if (xPosition < 0) xPosition = 0.f;
	if (yPosition < 0) yPosition = 0.f;
	if (xPosition + width >= Camera::Get().GetMapWidth())
	{
		xPosition = static_cast<float>(Camera::Get().GetMapWidth() - width);
	}

	// 3. 변환될 정수 목표 좌표
	Vector2 targetGridPos(static_cast<int>(xPosition), static_cast<int>(yPosition));

	// 4. 위치 변화가 생긴 경우에만 이동 검사 수행
	if (currentGridPos != targetGridPos)
	{
		// [핵심 해결책 A] 한 프레임에 2칸 이상 멀어지는 터널링 방지 (축별 1칸씩 단계별 검사)
		Vector2 stepPos = currentGridPos;
		Vector2 stepDir(
			(targetGridPos.x > currentGridPos.x) ? 1 : ((targetGridPos.x < currentGridPos.x) ? -1 : 0),
			(targetGridPos.y > currentGridPos.y) ? 1 : ((targetGridPos.y < currentGridPos.y) ? -1 : 0)
		);

		bool canMoveAllSteps = true;

		// X축, Y축을 순차적으로 1칸씩 검사
		while (stepPos != targetGridPos)
		{
			Vector2 nextStep = stepPos;
			if (nextStep.x != targetGridPos.x) nextStep.x += stepDir.x;
			else if (nextStep.y != targetGridPos.y) nextStep.y += stepDir.y;

			if (level && level->CanMove(stepPos, nextStep))
			{
				stepPos = nextStep;
			}
			else
			{
				canMoveAllSteps = false;
				break; // 도중에 벽이나 장애물이 있으면 중단
			}
		}

		// 5. 이동 가능한 단계까지만 적용, 안되면 실수 좌표 롤백
		if (canMoveAllSteps)
		{
			SetPosition(targetGridPos);
		}
		else
		{
			// [핵심 해결책 B] 이동 불가능 시 실수 좌표를 막히기 전(또는 성공한 단계) 좌표로 롤백!
			SetPosition(stepPos);
			xPosition = static_cast<float>(stepPos.x);
			yPosition = static_cast<float>(stepPos.y);
		}
	}
}