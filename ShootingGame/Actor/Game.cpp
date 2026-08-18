#include "Game.h"
#include <Level/RestLevel.h>
#include <Level/GameLevel.h>


Game::Game()
{
	//두 레벨 생성 및 배열에 추가.
	levelList.emplace_back(std::make_shared<RestLevel>());
	levelList.emplace_back(std::make_shared<RestLevel>());

	// 시작 상태 설정
	state = State::Gameplay;

	// 게임 시작 시 활성화할 레벨 설정.
	mainLevel = levelList[(int)state];
}


void Game::ToggleMenu()
{
	int stateIndex = static_cast<int>(state);
	int nextState = 1 - stateIndex;

	// 레벨 설정 및 상태 값 업데이트
	mainLevel = levelList[nextState];
	state = static_cast<State>(nextState);
}