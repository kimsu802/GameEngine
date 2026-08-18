#include "NPC.h"

using namespace Craft;

NPC::NPC(const Craft::Vector2& position)
	:Actor("N", position, Color::Cyan)
{
	sortingOrder = 4;
}

NPC::NPC(const Craft::Vector2& position, int npcId)
	:Actor("N", position, Color::Cyan), npcId(npcId)
{
	sortingOrder = 4;
}
