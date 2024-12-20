# 2D Infinite Runner Game

## Overview
The **2D Infinite Runner Game** is a side-scrolling game developed using OpenGL. The player controls a character who must dodge obstacles, collect items, and avoid losing all lives within the given time. The game progressively becomes faster as time goes on, requiring quick reflexes. The player can collect items and power-ups, with the goal of achieving the highest score possible before time runs out or all lives are lost.

## Game Mechanics

In the **Infinite Runner Game**, the player controls a character who is running from left to right. The player must avoid obstacles, collect items for score, and activate power-ups that provide temporary advantages. The main features of the game include:
- **Player**: Can jump or duck to avoid obstacles.
- **Obstacles**: Appear at varying heights and move towards the player. Colliding with an obstacle causes the player to lose a life.
- **Collectables**: Appear throughout the game, providing score points when collected.
- **Power-ups**: Provide temporary effects like slow-motion and boosted jump when collected.
- **Game End**: The game ends when the player loses all lives or when time runs out.

### Power-ups
- **Slow Motion**: Temporarily slows down the game, making it easier to avoid obstacles and collect items.
- **Boosted Jump**: Increases the height of the player's jump, making it easier to collect items placed above the player or avoid higher obstacles.

## Game Requirements

### Environment
- **Health, Score, and Time**: Displayed at the top of the screen. Health is represented visually (non-numerically), and it updates upon losing a life.
- **Player**: Starts on the left side of the screen and can jump or duck using keyboard controls. The player cannot move outside the game borders.
- **Obstacles**: Appear on the right side of the screen, moving toward the player. Obstacles are positioned at varying heights, allowing the player to dodge them by jumping or ducking.
- **Collectables**: Continuously spawn throughout the game. The player must jump or duck to collect them.
- **Power-ups**: Two types: Slow Motion and Boosted Jump. Power-ups disappear after being collected and activate for a limited time.
- **Upper and Lower Boundaries**: At least 4 primitives are used to draw the upper and lower boundaries.

### Player Controls
- **W**: Jump.
- **S**: Duck.
- **Esc**: Exit the game.

### Obstacles
- Obstacles continuously spawn from the right and move towards the player.
- Collisions with obstacles result in the loss of one life, and the health bar is updated.

### Collectables
- Collectables spawn continuously, and the player must be in the correct position (jump or duck) to collect them.
- Collecting an item increases the score.

### Power-ups
- **Slow Motion**: Slows down the game for a short duration.
- **Boosted Jump**: Temporarily increases the height of the player's jump.
- Both power-ups disappear when collected and affect the gameplay for a limited time.

### Game End
- The game ends when all lives are lost or when time runs out.
- When the game ends, a message such as "Game Over" or "You Win" will replace the game scene, showing the score.

## Animations
- **Player**: The player can jump and duck, with corresponding animations for each action.
- **Obstacles and Collectables**: Obstacles and collectables animate as they move toward the player.
- **Background**: The background animates to create a dynamic environment.
- **Power-ups**: Power-ups animate to indicate their availability for collection.

## Bonus Features

### Sound
- Background music plays when the game starts.
- A sound effect is played when the player collects an item.
- A different sound effect plays when the player collides with an obstacle.
- Additional sound effects for game win and game loss.

## Setup and Installation

1. **Clone or download the project** to your local machine.
2. **Install OpenGL** (OpenGL 32.x) and ensure you have the necessary libraries for C++.
3. **Compile the project** using your preferred C++ IDE or build system.
4. **Run the game** by executing the compiled file.

## Usage

- **Start Game**: Press **W** to make the player jump.
- **Duck**: Press **S** to make the player duck.
- **Power-ups**: Collect power-ups to activate slow motion or boosted jump effects.
- **Exit Game**: Press **ESC** to exit the game.

## Technical Details

- **Rendering**: The game utilizes OpenGL for rendering 2D primitives like points, lines, triangles, and polygons for the player, obstacles, collectables, and power-ups.
- **Game Logic**: The game speed increases as time progresses, and objects move faster toward the player as the game time elapses.
- **Animations**: Various objects, including the player, obstacles, collectables, and power-ups, have animations that respond to player input and game progression.

## License

This project is open-source and is released under the MIT License.
