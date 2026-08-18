#pragma once
#include <memory> // std::weak_ptr 사용을 위해.
#include <Core/Core.h>
#include <Core/CraftObject.h>
#include <Math/Vector2.h>
#include <Math/Color.h>
#include <string>
#include <Type/EnumTypes.h>

namespace Craft
{
	// 전방 선언
	class Level;

	// 가상 공간에 배치될 모든 액터의 기본 클래스
	class CRAFT_API Actor : public CraftObject
	{
		TYPE_DECLARATIONS(Actor,CraftObject)

		public:
			Actor(
				const std::string& image = "",
				const Vector2& position = Vector2::Zero,
				Color color = Color::White,
				RenderSpace renderspace = RenderSpace::Game);

			virtual ~Actor();

			// 게임 플레이 이벤트 함수
			virtual void BeginPlay();
			virtual void Tick(float deltaTime);
			virtual void Draw();

			// 충돌 이벤트 함수
			virtual void OnCollision(const std::shared_ptr<Actor>& other);

			//액터 제거 함수
			void Destroy();

			//게임 엔진 종료 함수
			void QuitGame();

			// Getter/Setter
			inline bool HasBeganPlay() const { return hasBeganPlay; }
			inline bool IsActive() const { return isActive && !hasExpired; }
			inline bool HasExpired() const { return hasExpired;  }

			inline std::shared_ptr<Level> GetOwner() const { return owner.lock(); }
			inline void SetOwner(std::weak_ptr<Level> newOwner) { owner = newOwner; }

			inline Vector2 GetPosition() const { return position; }
			void SetPosition(const Vector2& newPosition);

			// 이전 위치 반환 함수.
			inline Vector2 GetPreviousPosition() const { return previousPosition; }

			// 프레임 종료 후 이전 프레임 위치 저장 함수
			inline void SavePreviousState() { previousPosition = position; }

			// 너비 반환 함수
			inline int GetWidth() const { return width; }

			// 충돌 참여 여부.
			// false인 액터는 CollisionSystem에서 완전히 건너뛴다.
			// 바닥/벽/NPC 등은 기본값(false)을 쓰고,
			// 총알/적/플레이어처럼 실제 충돌이 필요한 액터만 true로 세팅한다.
			inline bool IsCollidable() const { return isCollidable; }
			inline void SetCollidable(bool value) { isCollidable = value; }

			// 액터의 이미지 설정 함수
			inline void ChangeImage(const std::string& newImage)
			{
				// 이미지 길이 설정
				width = static_cast<int>(newImage.length());

				// 새로운 글자 값 설정.
				image = newImage;
			}

		protected:
			// BeginPlay 이벤트 처리 여부 플래그
			bool hasBeganPlay = false;

			// 액터 활성화 여부 플래그
			bool isActive = true;

			// 삭제 요청 여부 플래그
			bool hasExpired = false;

			// 충돌 검사 대상 여부. 기본값 false.
			bool isCollidable = false;

			// 오너십
			// weak_ptr -> 약참조 -> 실제 사용을 위해서는 해당 위치가 유효한지 확인해야함.
			std::weak_ptr<Level> owner;

			// 화면에 그릴 글자
			std::string image;

			// 글자 색상
			Color color = Color::White;

			// 글자 길이
			int width = 0;

			// 렌더링 순서
			int sortingOrder = 0;

			// 위치
			Vector2 position;

			// 이전 프레임 위치
			Vector2 previousPosition;

			// 어느 뷰포트에 렌더링 할지
			RenderSpace renderSpace;

	};
}

