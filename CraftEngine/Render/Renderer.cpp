#include "Renderer.h"
#include "ScreenBuffer.h"
#include <cassert>
#include <iostream>
#include <Windows.h>
#include <algorithm>
#include <Camera/Camera.h>

namespace Craft
{
	//static 변수 초기화
	Renderer* Renderer::instance = nullptr;

	Renderer::Renderer(const Vector2& screenSize)
		:screenSize(screenSize)
	{
		assert(!instance && "instance should be null !");
		instance = this;

		// 프레임 객체 생성
		const int bufferCount = screenSize.x * screenSize.y;
		frame = std::make_unique<Frame>(bufferCount);

		// 생성 후 프레임 지우기.
		frame->Clear(screenSize);

		// 이중 버퍼 구현을 위한 콘솔 버퍼 생성 및 초기화
		screenBufferArray[0] = std::make_unique<ScreenBuffer>(screenSize);
		screenBufferArray[0]->Clear();


		screenBufferArray[1] = std::make_unique<ScreenBuffer>(screenSize);
		screenBufferArray[1]->Clear();

		//화면에 0번 콘솔 버퍼 활성화.
		SetConsoleActiveScreenBuffer(screenBufferArray[0]->GetBuffer());
	}

	Renderer::Renderer(const Vector2& gameSize, const Vector2& uiSize, int margin, int downmargin)
		:gameScreenSize(gameSize),uiScreenSize(uiSize),margin(margin), downmargin(downmargin)
	{
		int totalWidth = margin + gameSize.x + margin + uiSize.x + margin;
		int totalHeight = margin + max(gameSize.y, uiScreenSize.y) + downmargin + 5;

		screenSize = Vector2(totalWidth, totalHeight);

		viewportMap.emplace(std::make_pair(RenderSpace::Game, Viewport(Vector2(margin, margin), gameScreenSize)));
		viewportMap.emplace(std::make_pair(RenderSpace::UI, Viewport(Vector2(margin + gameSize.x + margin, margin), uiSize)));
		viewportMap.emplace(std::make_pair(RenderSpace::Dialogue, Viewport(Vector2(margin, margin + gameSize.y + 1), Vector2(gameSize.x + margin + uiSize.x, downmargin))));
		viewportMap.emplace(std::make_pair(RenderSpace::Full, Viewport(Vector2(margin, margin), Vector2(totalWidth, totalHeight))));

		assert(!instance && "instance should be null !");
		instance = this;

		// 프레임 객체 생성
		const int bufferCount = screenSize.x * screenSize.y;
		frame = std::make_unique<Frame>(bufferCount);

		// 생성 후 프레임 지우기.
		frame->Clear(screenSize);

		// 이중 버퍼 구현을 위한 콘솔 버퍼 생성 및 초기화
		screenBufferArray[0] = std::make_unique<ScreenBuffer>(screenSize);
		screenBufferArray[0]->Clear();


		screenBufferArray[1] = std::make_unique<ScreenBuffer>(screenSize);
		screenBufferArray[1]->Clear();

		//화면에 0번 콘솔 버퍼 활성화.
		SetConsoleActiveScreenBuffer(screenBufferArray[0]->GetBuffer());
	}

	Renderer::~Renderer()
	{
		instance = nullptr;

		// 콘솔 창을 원래대로 복구
		SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
	}

	void Renderer::Submit(const std::string& image, const Vector2& position, Color color, int sortingOrder,RenderSpace renderSpace)
	{
		const Viewport& viewport = viewportMap[renderSpace];

		Vector2 renderPosition = position;

		if (renderSpace == RenderSpace::Game)
		{
			renderPosition = renderPosition - Camera::Get().GetOffset();
		}

		// 렌더 명령 생성 및 값 설정
		RenderCommand command;
		command.image = UTF8ToWide(image);
		command.position = viewport.origin + renderPosition;
		command.color = color;
		command.sortingOrder = sortingOrder;

		command.clipMinX = viewport.origin.x;
		command.clipMaxX = viewport.origin.x + viewport.size.x - 1;

		command.clipMinY = viewport.origin.y;            
		command.clipMaxY = viewport.origin.y + viewport.size.y - 1;
		
		// 렌더 큐에 명령 추가
		renderQueue.emplace_back(command);
	}

	void Renderer::Draw()
	{
		// 화면(이미지/프레임) 지우기
		Clear();

		// 프레임 비우고 바로 외곽선.
		if (bOutlineVisible)
		{
			for (int i = 1; i < viewportMap.size(); ++i)
			{
				DrawBorderOutline(viewportMap[static_cast<RenderSpace>(i)]);
			}
		}


		// 프레임 그리기
		DrawRenderQueue();

		// 화면(이미지/프레임) 표시
		Present();
	}

	Renderer& Renderer::Get()
	{
		assert(instance && "instance should not be nullptr !");
		return *instance;
	}

	void Renderer::Clear()
	{
		//프레임 값 초기화.
		frame->Clear(screenSize);

		//콘솔 버퍼 초기화.
		GetCurrentBuffer()->Clear();
	}

	void Renderer::DrawBorderOutline(const Viewport& viewport)
	{
		int left = viewport.origin.x - 1;
		int right = viewport.origin.x + viewport.size.x;
		int top = viewport.origin.y - 1;
		int bottom = viewport.origin.y + viewport.size.y;



		// 가로선 (위/아래)
		for (int x = left + 1; x < right; ++x)
		{
			WriteCharDirect(Vector2(x, top), L'─');
			WriteCharDirect(Vector2(x, bottom), L'─');
		}

		// 세로선 (좌/우)
		for (int y = top + 1; y < bottom; ++y)
		{
			WriteCharDirect(Vector2(left, y), L'│');
			WriteCharDirect(Vector2(right, y), L'│');
		}

		WriteCharDirect(Vector2(left, top), L'┌'); 
		WriteCharDirect(Vector2(right, top), L'┐');
		WriteCharDirect(Vector2(left, bottom), L'└');
		WriteCharDirect(Vector2(right, bottom), L'┘');
	}

	void Renderer::WriteCharDirect(const Vector2& position, wchar_t ch, Color color)
	{
		// 버퍼 범위 밖이면 무시
		if (position.x < 0 || position.x >= screenSize.x ||
			position.y < 0 || position.y >= screenSize.y)
		{
			return;
		}

		int index = (position.y * screenSize.x) + position.x;

		frame->charInfoArray[index].Char.UnicodeChar = ch;      // ← Unicode 필드
		frame->charInfoArray[index].Attributes = static_cast<DWORD>(color);
		frame->sortingOrderArray[index] = INT_MAX;               // 액터가 절대 못 덮어쓰게 최우선순위 부여
	}

	void Renderer::DrawRenderQueue()
	{
		// 렌더 큐를 순회하면서 그리기 명령 실행
		for (const RenderCommand& command : renderQueue)
		{     
			// 그릴 문자값이 없으면 건너뛰기
			if (command.image.empty())
			{
				continue;
			}

			// y 위치가 화면을 벗어났으면 건너뛰기.
			if (command.position.y < command.clipMinY || command.position.y > command.clipMaxY)
			{
				continue;
			}

			// --- wide char를 고려한 총 컬럼 폭 계산 ---
			int totalColumns = 0;
			for (size_t i = 0; i < command.image.size(); ++i)
			{
				totalColumns += IsWideChar(command.image[i]) ? 2 : 1;
			}

			// 글자의 시작/끝 위치 (컬럼 기준)
			const int StartX = command.position.x;
			const int EndX = StartX + totalColumns - 1;

			// x 위치가 화면을 벗어났는지 확인
			if (EndX < command.clipMinX || StartX > command.clipMaxX)
			{
				continue; 
			}

			// 문자열을 순회하며 컬럼 위치를 직접 추적
			int col = StartX;
			for (size_t i = 0; i < command.image.size(); ++i)
			{
				const wchar_t ch = command.image[i];
				const bool bWide = IsWideChar(ch);
				const int charWidth = bWide ? 2 : 1;

				// 이 글자가 차지하는 컬럼 범위: [col, col + charWidth - 1]
				const int charEndCol = col + charWidth - 1;

				// 클리핑: 완전히 밖이면 건너뛰기
				if (charEndCol < command.clipMinX || col > command.clipMaxX)
				{
					col += charWidth;
					continue;
				}

				// --- leading cell (첫 번째 컬럼) ---
				if (col >= 0 && col < screenSize.x)
				{
					const int index = (command.position.y * screenSize.x) + col;

					if (frame->sortingOrderArray[index] <= command.sortingOrder)
					{
						frame->charInfoArray[index].Char.UnicodeChar = ch;
						frame->charInfoArray[index].Attributes = static_cast<DWORD>(command.color);
						if (bWide)
						{
							frame->charInfoArray[index].Attributes |= COMMON_LVB_LEADING_BYTE;
						}
						frame->sortingOrderArray[index] = command.sortingOrder;
					}
				}

				// --- trailing cell (전각 문자의 두 번째 컬럼) ---
				if (bWide)
				{
					const int trailCol = col + 1;
					if (trailCol >= 0 && trailCol < screenSize.x)
					{
						const int trailIndex = (command.position.y * screenSize.x) + trailCol;

						if (frame->sortingOrderArray[trailIndex] <= command.sortingOrder)
						{
							frame->charInfoArray[trailIndex].Char.UnicodeChar = L' ';
							frame->charInfoArray[trailIndex].Attributes = static_cast<DWORD>(command.color)
								| COMMON_LVB_TRAILING_BYTE;
							frame->sortingOrderArray[trailIndex] = command.sortingOrder;
						}
					}
				}

				col += charWidth;
			}
		}

		// 앞에서 설정한 2차원 배열을 콘솔에 그리기
		GetCurrentBuffer()->Draw(frame->charInfoArray.get());

		// 렌더큐 비우기
		renderQueue.clear();

		SetConsoleTextAttribute(
			GetCurrentBuffer()->GetBuffer(),
			static_cast<DWORD>(Color::White)
		);
	}

	void Renderer::Present()
	{
		// 현재 순번의 콘솔 버퍼를 활성화.
		SetConsoleActiveScreenBuffer(GetCurrentBuffer()->GetBuffer());

		// 인덱스 업데이트(갱신)
		// 0 -> 1 -> 0 -> 0 (One Minus 공식)
		currentBufferIndex = 1 - currentBufferIndex;
	}

	const ScreenBuffer* const Renderer::GetCurrentBuffer() const
	{
		return screenBufferArray[currentBufferIndex].get();
	}

	std::wstring Renderer::UTF8ToWide(const std::string& utf8)
	{
		if (utf8.empty()) return std::wstring();

		int wideLength = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
		std::wstring wide(wideLength, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), wide.data(), wideLength);
		return wide;
	}


	bool Renderer::IsWideChar(wchar_t ch)
	{
		// 한글 자모
		if (ch >= 0x1100 && ch <= 0x11FF) return true;
		// 한글 호환 자모
		if (ch >= 0x3130 && ch <= 0x318F) return true;
		// 한글 음절
		if (ch >= 0xAC00 && ch <= 0xD7AF) return true;
		// CJK 통합 한자
		if (ch >= 0x4E00 && ch <= 0x9FFF) return true;
		// CJK 호환 한자
		if (ch >= 0xF900 && ch <= 0xFAFF) return true;
		// 전각 기호/가타카나/히라가나 등
		if (ch >= 0x2E80 && ch <= 0x303E) return true;
		if (ch >= 0x3040 && ch <= 0x30FF) return true;
		if (ch >= 0x3400 && ch <= 0x4DBF) return true;
		// 전각 영숫자/기호
		if (ch >= 0xFF01 && ch <= 0xFF60) return true;
		if (ch >= 0xFFE0 && ch <= 0xFFE6) return true;

		return false;
	}

/*--------------- 프레임 --------------------------*/
	Renderer::Frame::Frame(int bufferCount)
	{
		// 2차원 배열 생성
		charInfoArray = std::make_unique<CHAR_INFO[]>(bufferCount);
		sortingOrderArray = std::make_unique<int[]>(bufferCount);
	}

	Renderer::Frame::~Frame()
	{
	}

	//프레임 초기화 함수
	void Renderer::Frame::Clear(const Vector2& screenSize)
	{
		//이중 루프를 순회하면서 값 초기화
		const int width = screenSize.x;
		const int height = screenSize.y;

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				// 1차원 배열을 2차원 배열로 사용할 때
				// 필요한 인덱스 좌표 변환.

				const int index = (y * width) + x;

				// 글자 항목 초기화
				CHAR_INFO& info = charInfoArray[index];
				
				// 빈 문자 설정
				info.Char.UnicodeChar = L' ';

				// 색상 표기 안함
				info.Attributes = 0;

				// 그리기 순서 배열 항목 초기화
				sortingOrderArray[index] = -1;
			}
		}
	}

/*--------------- 프레임 --------------------------*/
}