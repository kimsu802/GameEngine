#pragma once
#include <Windows.h>
#include <Math/Vector2.h>

namespace Craft
{
	class Vector2;

	class ScreenBuffer
	{
	public:
		ScreenBuffer(const Vector2& screenSize);
		~ScreenBuffer();

		void Clear() const;

		void Draw(const CHAR_INFO* const charInfo) const;

		inline HANDLE GetBuffer() const { return buffer; }


	private:
		// 화면 버퍼 핸들
		HANDLE buffer = nullptr;

		// 화면 크기
		Vector2 size;
	};
}
