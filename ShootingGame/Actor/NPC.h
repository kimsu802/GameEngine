#pragma once
#include <Actor/Actor.h>

class NPC : public Craft::Actor
{
	TYPE_DECLARATIONS(NPC, Actor)

public:
	NPC(const Craft::Vector2& position);
	NPC(const Craft::Vector2& position, int npcId);

	inline int GetNpcId() const { return npcId; }

private:
	// NPCManager에 등록된 NPCData를 찾기 위한 식별자.
	int npcId = -1;
};
