#pragma once

#include <string>

// ==========================================
//  StringUtil — 콘솔 표시 폭 계산
// ==========================================
//
// Windows 콘솔에서 한 글자가 차지하는 "칸 수"는 문자 종류에 따라 다르다.
//   - ASCII (0x00~0x7F)               : 1칸 (Half-Width)
//   - 한글/한자/가나 등 CJK Full-Width : 2칸
//
// std::string::length()는 UTF-8 바이트 수를 돌려주므로,
// 한글이 섞인 문자열의 가운데 정렬에 사용하면 위치가 어긋난다.
//
// 이 유틸은 UTF-8 문자열을 순회하면서 "콘솔에서 몇 칸을 차지하는지"를 계산한다.
//
// 사용 예:
//   int width = StringUtil::GetDisplayWidth("시고르자브 이장님");
//   // → 8×2 + 1 = 17  (length()는 25를 반환하지만 실제 표시 폭은 17)
//
namespace StringUtil
{
	// UTF-8 문자열이 콘솔에서 차지하는 칸 수를 반환한다.
	inline int GetDisplayWidth(const std::string& utf8)
	{
		int width = 0;
		int i = 0;
		int len = static_cast<int>(utf8.size());

		while (i < len)
		{
			unsigned char c = static_cast<unsigned char>(utf8[i]);

			int charBytes = 1;
			int charWidth = 1; // ASCII 기본값.

			if (c < 0x80)
			{
				// ASCII (1바이트) → 1칸.
				charBytes = 1;
				charWidth = 1;
			}
			else if ((c & 0xE0) == 0xC0)
			{
				// 2바이트 UTF-8 (라틴 확장 등) → 대부분 1칸.
				charBytes = 2;
				charWidth = 1;
			}
			else if ((c & 0xF0) == 0xE0)
			{
				// 3바이트 UTF-8 → CJK 범위인지 확인.
				charBytes = 3;

				if (i + 2 < len)
				{
					// UTF-8 3바이트를 유니코드 코드포인트로 변환.
					unsigned int cp =
						((c & 0x0F) << 12) |
						((static_cast<unsigned char>(utf8[i + 1]) & 0x3F) << 6) |
						(static_cast<unsigned char>(utf8[i + 2]) & 0x3F);

					// CJK Full-Width 범위 판정.
					// 한글 자모    : U+1100 ~ U+11FF
					// CJK 통합 한자: U+2E80 ~ U+9FFF
					// 한글 음절    : U+AC00 ~ U+D7AF
					// CJK 호환     : U+F900 ~ U+FAFF
					// 전각 기호    : U+FF01 ~ U+FF60
					// 가나/가타카나: U+3000 ~ U+30FF
					if ((cp >= 0x1100 && cp <= 0x11FF) ||
						(cp >= 0x2E80 && cp <= 0x9FFF) ||
						(cp >= 0xAC00 && cp <= 0xD7AF) ||
						(cp >= 0xF900 && cp <= 0xFAFF) ||
						(cp >= 0xFF01 && cp <= 0xFF60) ||
						(cp >= 0x3000 && cp <= 0x30FF))
					{
						charWidth = 2;
					}
				}
			}
			else if ((c & 0xF8) == 0xF0)
			{
				// 4바이트 UTF-8 (이모지 등) → 2칸으로 취급.
				charBytes = 4;
				charWidth = 2;
			}

			width += charWidth;
			i += charBytes;
		}

		return width;
	}

	// 가운데 정렬 시 x 좌표를 계산한다.
	// viewportWidth : 뷰포트 너비 (칸 수).
	// text          : 표시할 문자열.
	inline int GetCenteredX(int viewportWidth, const std::string& text)
	{
		int displayWidth = GetDisplayWidth(text);
		int x = (viewportWidth - displayWidth) / 2;
		return (x > 0) ? x : 0;
	}
}
