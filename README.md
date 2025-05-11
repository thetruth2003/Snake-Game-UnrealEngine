# 🐍 Snake Game with Unreal Engine 5
3D Snake Game developed in Unreal Engine 5 using C++.

- Futuregames Academy  
- (04 May 2025)

<p align="center">
  <img src="https://github.com/user-attachments/assets/63e3095c-8017-41f2-ba3f-aa245eaa8893" width="250" title="hover text"><br>
  <a href=""></a>
</p>


* * *

## Key Features

- **Multiple Game Modes**: Single Player, PvP (local split-screen), Coop, AI-supported modes (PvAI, CoopAI)  
- **Dynamic Tail Growth**: Tail segments grow and follow the head smoothly  
- **AI Pathfinding**: AI snake uses grid-based BFS to chase the closest apple  
- **Level Management**: Load levels from text files with walls (`#`), doors (`D`), floors (`.`), and empty tiles (`O`)  
- **Food Spawning**: Apples spawn on valid floor tiles surrounded by walkable neighbors  
- **Custom UI**: UMG widgets for Main Menu, In-Game HUD, Pause Menu, and Game Over screens  
- **Sound & Effects**: Ambient music, game over sound, eat particles, notice sound with question-mark widget  

* * *

## Controls

- **Movement**: Arrow keys or WASD to change direction  
- **Jump**: Spacebar  
- **Pause**: `P` key toggles pause menu  
- **Restart**: On Game Over screen or click Restart button  

* * *

## Screenshots

Screenshots           |  Screenshots 
:-------------------------:|:-------------------------:
![](ForReadme/1.png)  |  ![](ForReadme/3.png)
![](ForReadme/2.png)  |  ![](ForReadme/4.png)

* * *

## Implementation Details

### SnakePawn

- Handles player and AI snake movement, grid snapping, tail growth, collision detection, and jump logic  
- Uses `ESnakeDirection` enum and grid-based movement in `UpdateMovement` and `MoveSnake`  

### ASnakeAIController

- Implements `Tick` to recalc path only on new tile entry  
- Uses BFS (`FindPath`) on walkable tiles, excluding the snake’s tail  
- Snaps goals and path points to grid for precise navigation  

### SnakeWorld

- Loads level layouts from `Content/Levels/LevelN.txt` via `LoadLevelFromText`  
- Manages instanced static meshes for walls and floors  
- Spawns food actors (`SnakeFood`) on valid tiles in `SpawnFood`  

### SnakeGameMode

- Manages game state transitions (`MainMenu`, `Game`, `Pause`, `Outro`)  
- Handles player/AI spawning, score & apple counters, and UI widget creation  
- Responds to `NotifyAppleEaten` to update UI and progress levels  

### MyUserWidget (UMG)

- Binds `ScoreText`, `LevelText`, `ScoreP1Text`, `ScoreP2Text`  
- Provides `SetScore`, `SetLevel`, and `SetPlayerScores` blueprint-callable functions  

* * *

## Challenges Faced and Solutions Implemented

- **Enhanced Input Mapping**: Configured separate `P1Mapping` and `P2Mapping` contexts for multiple local players  
- **Grid Precision**: Ensured accurate snapping and BFS convergence by rounding positions in `SnapToGrid`  
- **Performance**: Optimized tail updates and pathfinding to avoid hitches by limiting recalculations  

* * *

## Reflection on the Learning Experience

- Gained proficiency in Unreal Engine C++ workflows and module dependencies (`AIModule`, `EnhancedInput`)  
- Explored advanced UMG widget creation and dynamic UI management  
- Deepened understanding of AI pathfinding in grid-based environments  
- Learned best practices for actor instancing and data-driven level design  

## About

Personal project by **Kenan EGE**  

- Computer Engineer, Futuregames Academy  
- 04 May 2025  

Enjoy playing and experimenting!
