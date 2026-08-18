#include "Actor.h"
#include "Engine/Engine.h"
#include "Render/Renderer.h"

namespace Craft
{
	Actor::Actor(const std::string& image,
		const Vector2& position,
		Color color,RenderSpace renderspace) : image(image), position(position), color(color),width(static_cast<int>(image.length())), renderSpace(renderspace)
	{
		
	}

	Actor::~Actor()
	{
	}

	void Actor::BeginPlay()
	{
		// 이벤트 처리했다고 설정.
		hasBeganPlay = true;


	}

	void Actor::Tick(float deltaTime)
	{
		
	}

	void Actor::Draw()
	{
		// 비활성화 상태이면 종료.
		if (!isActive)
			return;

		// 렌더러에 필요한 데이터 제출
		Renderer::Get().Submit(image, position, color,sortingOrder, renderSpace);
	}

	void Actor::OnCollision(const std::shared_ptr<Actor>& other)
	{

	}

	void Actor::Destroy()
	{
		// 삭제 예약 설정.
		hasExpired = true;
	}

	void Actor::QuitGame()
	{
		Engine::Get().Quit();
	}

	void Actor::SetPosition(const Vector2& newPosition)
	{
		// 변경하려는 위치와 현재 위치가 동일하다면,
		if (position == newPosition)
			return;

		position = newPosition;
	}
}


