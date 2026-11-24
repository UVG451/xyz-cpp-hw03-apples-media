#include "Apple.h"
#include "GameSettings.h"

#include <cstdlib>

namespace ApplesGame
{
	void InitApple(Apple& apple, const sf::Texture& texture)
	{
		// Keep the whole apple inside the visible area on both axes.
		const auto margin = static_cast<unsigned int>(APPLE_SIZE / 2.f + 1.f);
		apple.position.x = static_cast<float>(margin + rand() % (SCREEN_WIDTH - 2 * margin + 1));
		apple.position.y = static_cast<float>(margin + rand() % (SCREEN_HEGHT - 2 * margin + 1));

		// The sprite refers to the texture owned by the game.
		apple.sprite.setTexture(texture);
		// The origin is in texture coordinates. Scaling happens separately.
		apple.sprite.setOrigin(GetSpriteOrigin(apple.sprite, { 0.5f, 0.5f }));
		// Use the same diameter for rendering and overlap checks.
		apple.sprite.setScale(GetSpriteScale(apple.sprite, { APPLE_SIZE, APPLE_SIZE }));
	}

	void DrawApple(Apple& apple, sf::RenderWindow& window)
	{
		// An apple may have respawned since the last frame.
		apple.sprite.setPosition(OurVectorToSf(apple.position));
		window.draw(apple.sprite);
	}

}
