#pragma once
#include <string>
#include <vector>
#include <fstream>

// NPCManager / QuestManager 에서 공통으로 사용하는 아주 단순한 "키: 값" 텍스트 데이터 로더.
// 기존에 이미 있던 Assets/Q1.txt 포맷("NPC: ...", "QUEST: ...")을 그대로 재사용하기 위한
// 용도라서, 별도 JSON 라이브러리 없이 std::ifstream + std::getline 정도로만 구현한다.
namespace DataFileUtil
{
	// 문자열 양 끝의 공백/개행(\r 포함)을 제거한다.
	// (Windows에서 만든 txt 파일이라 줄바꿈이 \r\n 인 경우가 많음 -> \r 제거가 특히 중요)
	inline std::string Trim(const std::string& str)
	{
		size_t start = str.find_first_not_of(" \t\r\n");
		if (start == std::string::npos)
		{
			return "";
		}

		size_t end = str.find_last_not_of(" \t\r\n");
		return str.substr(start, end - start + 1);
	}

	// 파일을 한 줄씩 읽어서 반환한다 (양 끝 공백은 이미 제거된 상태).
	// 파일을 못 열면 빈 벡터를 반환한다.
	inline std::vector<std::string> ReadAllLines(const std::string& path)
	{
		std::vector<std::string> lines;

		std::ifstream file(path);
		if (!file.is_open())
		{
			return lines;
		}

		std::string line;
		while (std::getline(file, line))
		{
			lines.push_back(line);
		}

		return lines;
	}

	// "키: 값" 형태의 한 줄을 key / value로 분리한다.
	// 빈 줄이거나 ':' 이 없으면 false를 반환한다 (그런 줄은 호출부에서 건너뛰면 됨).
	inline bool SplitKeyValue(const std::string& line, std::string& outKey, std::string& outValue)
	{
		if (line.empty())
		{
			return false;
		}

		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
		{
			return false;
		}

		outKey = Trim(line.substr(0, colonPos));
		outValue = Trim(line.substr(colonPos + 1));
		return true;
	}
}
