#pragma once
#ifndef COMPONENT_PLAYER_H
#define COMPONENT_PLAYER_H

#include "Component.h"
#include "../../math/Vector.h"

struct Movement;


//NOTE sushi: probably rename this to something more general, like an actor or something, but I don't like the name actor, so think of a better one :)
struct Player : public Component {
	int health;

	Movement* movement;

	Player(Movement* movement);

	void Update() override;

	std::vector<char> Save() override;
	static void Load(std::vector<Entity>& entityArray, const char* fileData, u32& cursor, u32 countToLoad);
};

#endif