#pragma once
#include <Engine/Engine.h>
#include <vector>

enum class State
{
	Gameplay = 0,
	Menu = 1,
	Length
};

class Game : public Craft::Engine
{
public:
	Game();
	~Game() = default;

	void ToggleMenu();

	// Engine의 mainLevel은 protected이므로 Game을 통해 접근한다.
	inline std::shared_ptr<Craft::Level> GetMainLevel() const { return mainLevel; }

private:
	std::vector<std::shared_ptr<Craft::Level>> levelList;

	State state = State::Gameplay;
};
