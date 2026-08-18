#include <Engine/Engine.h>
#include <Level/GameLevel.h>
#include <Level/RestLevel.h>
#include <Level/MenuLevel.h>
#include <Actor/Game.h>


int main()
{
	// 엔진 객체 생성 및 실행.
	//Craft::Engine engine;
	//engine.AddNewLevel<RestLevel>();
	//engine.Run();

	Game game;
	game.AddNewLevel<MenuLevel>();
	game.Run();
}