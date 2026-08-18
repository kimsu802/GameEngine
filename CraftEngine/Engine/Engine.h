#pragma once
#include <memory>
#include <Core/Core.h>

//CraftEngine 프로젝트 안의 클래스는 Craft 네임스페이스 사용
namespace Craft
{
	// 전방 선언
	class Level;
	class Input;
	class Renderer;
	class CollisionSystem;


	// 메인 엔진 클래스
	// 엔진 루프를 제공
	// 게임 엔진의 핵심 기능 제공
	class CRAFT_API Engine
	{
		//엔진 설정(데이터)
		struct Settings
		{
			// 목표 프레임 수
			float framerate = 120.f;

			int gameviewportwidth;

			int gameviewportheight;

			int uiviewportwidth;
			
			int uiviewportheight;

			int margin;

			int downmargin;
		};

	public:
		Engine();
		virtual ~Engine();

		//엔진 실행 함수
		void Run();

		//엔진 종료 함수
		void Quit();

		static Engine& Get();

		inline int GetGameWidth() const
		{
			return setting.gameviewportwidth;
		}
		inline int GetGameHeight() const
		{
			return setting.gameviewportheight;
		}
		inline int GetUIWidth() const
		{
			return setting.uiviewportwidth;
		}
		inline int GetUIHeight() const
		{
			return setting.uiviewportheight;
		}
		inline int GetMargin() const
		{
			return setting.margin;
		}
		inline int GetDownMargin() const
		{
			return setting.downmargin;
		}

		//레벨 추가 요청 함수
		template<typename T, typename = std::enable_if_t<std::is_base_of<Level,T>::value>>
		void AddNewLevel()
		{
			//추가 요청 레벨
			nextLevel = std::make_shared<T>();
		}

	protected:
		// 입력 처리 함수 (입력 폴링)
		void ProcessInput();

		//초기화 함수
		void OnInitialized();

		//게임 플레이 이벤트 함수
		
		//게임 플레이 초기화 함수
		void BeginPlay();

		//게임 플레이 업데이트 함수
		void Tick(float deltaTime);

		//레벨 그리기 함수
		void Draw();

		// 충돌 처리 함수
		void ProcessCollision();

		// 프레임 간 입력 값 저장을 위한 함수.
		void SavePreviousInputStates();

		//엔진 종료 시 정리가 필요할 때 사용할 함수.
		void Shutdown();

		void LoadEngineSetting();
	
	protected:
		//엔진 종료 요청 여부 플래그
		bool isQuit = false;

		//엔진 설정 함수
		Settings setting;

		// 전역 접근이 가능하도록 변수 선언
		static Engine* instance;

		// 메인 레벨
		std::shared_ptr<Level> mainLevel;

		// 추가 요청된 레벨
		std::shared_ptr<Level> nextLevel;

		// 입력 시스템 변수
		std::unique_ptr<Input> input;

		// 렌더러
		std::unique_ptr<Renderer> renderer;

		// 콜리전 시스템
		std::unique_ptr<CollisionSystem> collisionSystem;

	};
}


