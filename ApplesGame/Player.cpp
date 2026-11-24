#include "Player.h"
#include <cmath>
#include "GameSettings.h"
#include "Apple.h"
#include "Obstacle.h"

namespace ApplesGame
{
	void InitPlayer(Player& player, const sf::Texture& texture)
	{
		// The position stores the center of the sprite.
		player.position.x = (float)SCREEN_WIDTH / 2.f;
		player.position.y = (float)SCREEN_HEGHT / 2.f;
		// Use the default velocity before any apples are eaten.
		player.speed = INITIAL_SPEED;
		// A new round begins moving towards the top of the window.
		player.direction = PlayerDirection::Up;

		// Share the texture that GameState keeps alive for the whole session.
		player.sprite.setTexture(texture);
		// Rotate and mirror around the center rather than the texture corner.
		player.sprite.setOrigin(GetSpriteOrigin(player.sprite, {0.5f, 0.5f}));
		// Match the visible size to the size used by the collision code.
		player.sprite.setScale(GetSpriteScale(player.sprite, {PLAYER_SIZE, PLAYER_SIZE}));
	}

	void UpdatePlayer(Player& player, float timeDelta)
	{
		// Each frame advances one axis by speed multiplied by elapsed seconds.
		switch (player.direction)
		{
		case PlayerDirection::Up:
			// Screen y increases downwards.
			player.position.y -= player.speed * timeDelta;
			break;

		case PlayerDirection::Right:
			player.position.x += player.speed * timeDelta;
			break;

		case PlayerDirection::Down:
			player.position.y += player.speed * timeDelta;
			break;

		case PlayerDirection::Left:
			player.position.x -= player.speed * timeDelta;
			break;
		}
	}

	bool HasPlayerCollisionWithScreenBorder(const Player& player)
	{
		// The center can be inside the screen while the edge already touches a wall.
		return (player.position.x - PLAYER_SIZE / 2.f < 0) ||
			(player.position.x + PLAYER_SIZE / 2.f > SCREEN_WIDTH) ||
			(player.position.y - PLAYER_SIZE / 2.f < 0) ||
			(player.position.y + PLAYER_SIZE / 2.f > SCREEN_HEGHT);
	}

	bool HasPlayerCollisionWithApple(const Player& player, const Apple& apple)
	{
		// Compare the distance between centers against the sum of the radii.
		float dx = player.position.x - apple.position.x;
		float dy = player.position.y - apple.position.y;
		float distance = std::sqrt(dx * dx + dy * dy);
		return distance < (PLAYER_SIZE + APPLE_SIZE) / 2.f;
	}

	bool HasPlayerCollisionWithObstacle(const Player& player, const Obstacle& obstacle)
	{
		// Rocks use their own radius while sharing the player's circular hitbox.
		float dx = player.position.x - obstacle.position.x;
		float dy = player.position.y - obstacle.position.y;
		float distance = std::sqrt(dx * dx + dy * dy);
		return distance < (PLAYER_SIZE + OBSTACLE_SIZE) / 2.f;
	}

	void DrawPlayer(Player& player, sf::RenderWindow& window)
	{
		// Rendering converts the game position to SFML's two-component vector.
		player.sprite.setPosition(OurVectorToSf(player.position));
		// Restore the positive scale before applying any reflection.
		const sf::Vector2f spriteScale = (GetSpriteScale(player.sprite, { PLAYER_SIZE, PLAYER_SIZE }));

		// We need to rotate and flip sprite to match player direction
		switch (player.direction)
		{
		case PlayerDirection::Up:
			player.sprite.setScale(spriteScale.x, spriteScale.y);
			player.sprite.setRotation(-90.f);
			break;

		case PlayerDirection::Right:
			// The source artwork already faces right.
			player.sprite.setScale(spriteScale.x, spriteScale.y);
			player.sprite.setRotation(0.f);
			break;

		case PlayerDirection::Down:
			player.sprite.setScale(spriteScale.x, spriteScale.y);
			player.sprite.setRotation(90.f);
			break;

		case PlayerDirection::Left:
			// Horizontal reflection keeps the top of the artwork upright.
			player.sprite.setScale(-spriteScale.x, spriteScale.y);
			player.sprite.setRotation(0.f);
			break;
		}

		// Submit the sprite after updating its transform for this frame.
		window.draw(player.sprite);
	}
}
