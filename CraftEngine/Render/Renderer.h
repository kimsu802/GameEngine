#pragma once

#include <Core/Core.h>
#include <Math/Vector2.h>
#include <Math/Color.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <Type/EnumTypes.h>



namespace Craft
{
	struct Viewport
	{
		Viewport(const Vector2& pos, const Vector2& size)
			: origin(pos), size(size)
		{

		}

		Viewport() = default;
		~Viewport() = default;

		// 해당 영역의 시작 좌표
		Vector2 origin;

		// 해당 영역의 크기
		Vector2 size;
	};

	// 전방 선언
	class ScreenBuffer;

	// 그리기 기능을 전담하는 전문 객체.
	class CRAFT_API Renderer
	{
		// 프레임(이미지) 데이터 구조체
		struct Frame
		{
			Frame(int bufferCount);
			~Frame();

			//프레임 초기화 함수
			void Clear(const Vector2& screenSize);

			// 화면에 그릴 2차원 배열 문자값.
			std::unique_ptr<CHAR_INFO[]> charInfoArray;

			// 그리기 정렬 값 이차원 배열
			std::unique_ptr<int[]> sortingOrderArray;
		};

		// 화면에 그릴 데이터를 명령 단위로 저장하기 위한 구조체
		struct RenderCommand
		{
			// 화면에 그릴 문자 값.
			std::wstring image;

			// 위치
			Vector2 position = Vector2::Zero;

			// 색상
			Color color = Color::White;

			// 그리기 정렬 순서
			int sortingOrder = -1;

			// 그릴 영역의 왼쪽 끝
			int clipMinX = -1;

			// 그릴 영역의 오른쪽 끝;
			int clipMaxX = -1;

			// 그릴 영역의 상단부
			int clipMinY = -1;   // 추가

			// 그릴 영역의 하단부
			int clipMaxY = -1;   // 추가
		};

	public:
		Renderer(const Vector2& screenSize);
		Renderer(const Vector2& gameSize, const Vector2& uiSize, int margin,int downmargin);
		~Renderer();

		// 화면에 그릴 데이터를 제출(전달)하는 함수
		void Submit(const std::string& image, const Vector2& position, Color color = Color::White, int sortingOrder = 0,RenderSpace renderSpace = RenderSpace::Game);

		// Draw 이벤트 함수 - Engine에서 호출
		void Draw();

		// 전역 접근 함수
		static Renderer& Get();

		const Viewport& GetViewport(RenderSpace renderspace) const { return viewportMap.at(renderspace); }

		inline void SetOutlineVisible(bool bVisible) { bOutlineVisible = bVisible;  }

	private:
		// 그리기 작업을 시작할 때, 프레임(화면)을 지우는 함수.
		void Clear();

		// 비우고 나서 바로 뷰포트를 구분할 외곽선 그리기.
		void DrawBorderOutline(const Viewport& viewport);

		// 다이렉트로 프레임 안에 그려주는 함수.
		void WriteCharDirect(const Vector2& position, wchar_t ch, Color color = Color::Yellow);

		// 전달 받은 렌더 명령을 활용해 화면을 그리는 함수.
		void DrawRenderQueue();

		// 그린 결과를 화면에 표시하는 함수
		void Present();

		// Getter.
		const ScreenBuffer* const GetCurrentBuffer() const;

		std::wstring UTF8ToWide(const std::string& utf8);

		// 전각 문자(한글/CJK 등) 판별
		static bool IsWideChar(wchar_t ch);

	private:
		// 전역 접근이 가능하도록 변수 선언.
		static Renderer* instance;

		// 이번 프레임에 그릴 렌더 명령을 모아두는 배열
		// Queue 처럼 사용
		std::vector<RenderCommand> renderQueue;

		// 화면 크기
		Vector2 screenSize;

		// 게임 화면 크기
		Vector2 gameScreenSize;

		// UI 크기
		Vector2 uiScreenSize;

		// 여백
		int margin;

		// 대화 창 출력을 위한 하단 여백
		int downmargin;

		// 게임 뷰포트
		Viewport gameViewport;

		// UI 뷰포트
		Viewport uiViewport;

		// 대화창 뷰포트
		Viewport dialogueViewport;

		// 뷰포트 해시 맵
		std::unordered_map<RenderSpace, Viewport> viewportMap;

		// 글자/그리기 순서 2차원 배열을 관리하는 프레임 객체.
		std::unique_ptr<Frame> frame;

		// 이중 버퍼링 구현을 위한 화면 버퍼 2개
		std::unique_ptr<ScreenBuffer> screenBufferArray[2];

		// 버퍼 인덱스.
		int currentBufferIndex = 0;

		// 외곽선 표시 여부
		bool bOutlineVisible = true;
	};
}

