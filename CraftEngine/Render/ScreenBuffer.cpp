#include "ScreenBuffer.h"
#include <cassert>
#include <Camera/Camera.h>

namespace Craft
{
	ScreenBuffer::ScreenBuffer(const Vector2& screenSize)
		: size(screenSize)
	{
		buffer = CreateConsoleScreenBuffer(
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);

		assert(buffer != INVALID_HANDLE_VALUE && buffer != nullptr);

		// 1) 창을 최소로 먼저 줄인다 (이게 핵심)
		SMALL_RECT minRect = { 0, 0, 1, 1 };
		BOOL result = SetConsoleWindowInfo(buffer, TRUE, &minRect);
		assert(result && "SetConsoleScreenBufferSize failed");

		
		// 2) 그 다음 버퍼 크기 설정
		result = SetConsoleScreenBufferSize(buffer, size);
		assert(result && "SetConsoleScreenBufferSize failed");

		// 3) 마지막에 창을 원하는 크기로 확대
		SMALL_RECT rect = {
			0, 0,
			static_cast<SHORT>(size.x - 1),
			static_cast<SHORT>(size.y - 1)
		};
		result = SetConsoleWindowInfo(buffer, TRUE, &rect);
		assert(result && "SetConsoleWindowInfo failed");

		// 커서 끄기
		CONSOLE_CURSOR_INFO info{};
		result = GetConsoleCursorInfo(buffer, &info);
		assert(result);

		info.bVisible = FALSE;
		result = SetConsoleCursorInfo(buffer, &info);   // ← 반환값 받기
		assert(result);
	}
	ScreenBuffer::~ScreenBuffer()
	{
		// 콘솔 닫기
		if (buffer)
		{
			CloseHandle(buffer);
		}
	}
	void ScreenBuffer::Clear() const
	{
		// 콘솔 전체를 지우는 함수.

		//화면에 설정된 글자 갯수
		DWORD writtenCount = 0;

		// 공백 문자를 화면 전체에 한 번에 설정.
		BOOL result = FillConsoleOutputCharacterA(buffer, ' ', (size.x * size.y), Vector2::Zero, &writtenCount);

		assert(result);
	}
	void ScreenBuffer::Draw(const CHAR_INFO* const charInfo) const
	{
		// charInfo는 2차원 배열 (1차원 배열에 2차원 배열 정보를 기록).

		// 설정할 글자 영역
		SMALL_RECT rect = {
			0,							 // Left
			0,							 // Top
			static_cast<short>(size.x -1),	 // Right
			static_cast<short>(size.y -1)	 // Bottom		
		};

		// 콘솔에 CHAR_INFO 타입으로 글자 쓰는 함수.
		BOOL result = WriteConsoleOutputW( 
				buffer,
				charInfo,
				size,
				Vector2::Zero,
				&rect);

		assert(result);
	}
}