#include "Engine.h"
#include <Level/Level.h>
#include <Input/Input.h>
#include <Render/Renderer.h>
#include <Physics/CollisionSystem.h>
#include <Type/EnumTypes.h>

#include <iostream>
#include <windows.h>
#include <cassert>

namespace Craft
{
	//전역 변수 초기화
	Engine* Engine::instance = nullptr;

	Engine::Engine()
	{
		assert(!instance);
		instance = this;

		// 엔진 설정 로드.
		LoadEngineSetting();

		//입력 객체 생성
		input = std::make_unique<Input>();

		//렌더러 객체 생성
		renderer = std::make_unique<Renderer>(Vector2(setting.gameviewportwidth,setting.gameviewportheight),
											  Vector2(setting.uiviewportwidth,setting.uiviewportheight),
											  setting.margin,setting.downmargin);

		// 콜리전 시스템 객체 생성.
		collisionSystem = std::make_unique<CollisionSystem>();
	}

	Engine::~Engine()
	{
		instance = nullptr;
	}

	void Engine::Run()
	{
		// 고해상도 타이머 사용.
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);

		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);

		//프레임 계산을 위한 변수
		int64_t current = counter.QuadPart;
		int64_t previous = current;

		//엔진 루프	
		while (true)
		{
			// 종료 조건
			if (isQuit)
			{
				break;
			}

			// 프레임 처리
			
			// 입력 처리
			ProcessInput();

			//프레임 시간 계산

			// 현재 시간 읽기
			QueryPerformanceCounter(&counter);
			current = counter.QuadPart;

			// 고정 프레임으로 만들기 위한 값
			float oneFrameTime = 1.f / setting.framerate;

			// (현재 시간 - 이전 시간) / 시간 단위(해상도) - > '초' 단위로 변환.
			float deltaTime = static_cast<float>(current - previous) / static_cast<float>(frequency.QuadPart);

			// 고정 프레임 처리
			// 프레임 사이에 걸린 시간이 목표 시간보다 더 많이 지났으면 
			// 프레임 처리
			if (deltaTime >= oneFrameTime)
			{
				// 게임 이벤트 함수 호출
				OnInitialized();

				// 게임 이벤트의 초기화 함수(1번만 호출).
				BeginPlay();

				// 게임 업데이트
				Tick(deltaTime);

				// 충돌 처리
				ProcessCollision();

				// 화면 그리기
				Draw();

				//이 위까지 호출이 완료되면 프레임 처리 완료됨.

				//레벨 전환 처리.
				if (nextLevel)
				{
					// 기존 레벨 정리.
					if (mainLevel)
					{
						mainLevel.reset();
					}

					//추가 요청된 레벨을 메인 레벨로 설정.
					mainLevel = nextLevel;

					//포인터 정리
					nextLevel.reset();
				}

				// 추가/제거 요청된 액터 정리
				if (mainLevel)
				{
					mainLevel->ProcessAddAndDestroys();

					// 액터의 이전 상태 저장 처리
					mainLevel->SavePreviousActorStates();
				}

				// 입력 상태 저장
				SavePreviousInputStates();

				previous = current;

				char buffer[20];
				snprintf(buffer, sizeof(buffer), "DeltaTime = %f", deltaTime);

				std::string str = buffer;

				//Renderer::Get().Submit(str, Vector2(0, 1), Color::White, 0, RenderSpace::UI);
				SetConsoleTitleA(str.c_str());

			}
		}

		// 종료 처리 함수 호출.
		Shutdown();
	}
	void Engine::Quit()
	{
		// 엔진 종료 플래그 설정
		isQuit = true;
	}
	Engine& Engine::Get()
	{
		assert(instance);
		return *instance;
	}
	void Engine::ProcessInput()
	{
		assert(input && "input should not be null here !");
		if (!input)
		{
			return;
		}

		input->ProcessInput();
	}
	void Engine::OnInitialized()
	{
		//레벨 초기화 처리
		//예외 처리
		if (!mainLevel || mainLevel->HasInitialized())
		{
			return;
		}

		// 초기화 이벤트 호출
		mainLevel->OnInitialized();
	}
	void Engine::BeginPlay()
	{
		if (!mainLevel)
		{
			return;
		}

		mainLevel->BeginPlay();
	}
	void Engine::Tick(float deltaTime)
	{
		if (!mainLevel)
		{
			return;
		}

		mainLevel->Tick(deltaTime);
	}
	void Engine::Draw()
	{
		if (!mainLevel)
		{
			return;
		}
		mainLevel->Draw();

		if (!renderer)
			return;

		renderer->Draw();
	}
	void Engine::ProcessCollision()
	{
		//예외처리
		if (!mainLevel || !collisionSystem)
		{
			return;
		}

		// 충돌 처리.
		// 의존성 주입(Dependency Injection)
		collisionSystem->ProcessCollision(mainLevel->actorList);
	}
	void Engine::SavePreviousInputStates()
	{
		assert(input && "input should not be null here !");

		if (!input)
		{
			return;
		}

		input->SavePreviousStates();
	}
	void Engine::Shutdown()
	{
	}

	void Engine::LoadEngineSetting()
	{
		FILE* file = nullptr;
		fopen_s(&file, "../Config/Setting.txt", "rt");

		if (!file)
		{
			std::cout << "Failed to open engine setting file/\n";

				//디버그 모드에서 강제 중단 시키는 기능
				__debugbreak();

			return;
		}

		const int bufferSize = 2048;
		char buffer[bufferSize] = {};

		size_t readSize = fread(buffer, sizeof(char), bufferSize, file);
		
		// 값 저장을 위해 서식 해석 (Parsing)
		// 문자열 자르기(Split)

		char* context = nullptr;
		char* token = nullptr;

		// 파일에서 읽은 전체 문자열을 개행(\n)문자 기준으로 처음 자르기
		token = strtok_s(buffer, "\n", &context);

		// 반복해서 자르기
		while (token)
		{
			// 공백 전까지 읽은 문자열을 저장할 변수
			char key[25] = {};
			
			// 포맷을 지정한 문자열 읽기
			sscanf_s(token, "%s", key, 25);

			// 키 값을 비교해서 값 설정
			if (strcmp(key, "framerate") == 0)
			{
				sscanf_s(token, "framerate = %f", &setting.framerate);
			}
			else if (strcmp(key, "gameviewportwidth") == 0)
			{
				sscanf_s(token, "gameviewportwidth = %d", &setting.gameviewportwidth);
			}
			else if (strcmp(key, "gameviewportheight") == 0)
			{
				sscanf_s(token, "gameviewportheight = %d", &setting.gameviewportheight);
			}
			else if (strcmp(key, "uiviewportwidth") == 0)
			{
				sscanf_s(token, "uiviewportwidth = %d", &setting.uiviewportwidth);
			}
			else if (strcmp(key, "uiviewportheight") == 0)
			{
				sscanf_s(token, "uiviewportheight = %d", &setting.uiviewportheight);
			}
			else if (strcmp(key, "margin") == 0)
			{
				sscanf_s(token, "margin = %d", &setting.margin);
			}
			else if (strcmp(key, "downmargin") == 0)
			{
				sscanf_s(token, "downmargin = %d", &setting.downmargin);
			}


			token = strtok_s(nullptr, "\n", &context);
		}

		//strtok_s 에서 token은 자른 문자열, context는 다음에 어디서부터 잘라야 하는지" 그 위치를 기억해 두는 포인터(이중 포인터 상태)

		fclose(file);
		file = nullptr;	
	}
}