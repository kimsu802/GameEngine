#pragma once
#include <Level/Level.h>

// 메뉴 아이템 구조체.
struct MenuItem
{
	// 메뉴 선택 시 실행할 로직 저장을 위한 함수 포인터.
	using OnSelected = void(*)();

	MenuItem(const std::string& text, OnSelected onSelected)
		:text(text), onSelected(onSelected)
	{

	}

	// 메뉴 텍스트
	std::string text;

	// 메뉴를 선택했을 때 실행할 로직.
	OnSelected onSelected = nullptr;
};

class MenuLevel : public Craft::Level
{
	TYPE_DECLARATIONS(MenuLevel, Level);

	MenuLevel();

public:
	void LoadTitle(const std::string& filename);

private:
	virtual void OnInitialized() override;
	virtual void Draw() override;
	virtual void Tick(float deltaTime) override;


private:
	// 현재 활성화된 메뉴 아이템 인덱스
	int currentIndex = 0;

	// 선택된 메뉴 아이템의 색상;
	Craft::Color selectedColor = Craft::Color::Green;

	// 미선택된 메뉴 아이템의 색상.
	Craft::Color unselectedColor = Craft::Color::White;

	// 메뉴 아이템 배열
	std::vector<std::unique_ptr<MenuItem>> itemList;

};

