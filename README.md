🐍 NeoSnake - Unreal Engine 5

A modern 3D Snake Game developed in Unreal Engine 5 using C++. This version is adapted and maintained as part of a personal portfolio project.

Developed originally at Futuregames Academy (May 2025)

Adapted and maintained by [Your Name]

🎮 Key Features

Game Modes: Single Player, PvP (Split-Screen), Coop, PvAI, CoopAI

Dynamic Tail Growth: Snake segments follow smoothly and expand naturally

AI Pathfinding: Grid-based BFS navigation chasing nearest apple

Text-Based Levels: Levels loaded from .txt maps using simple characters

Valid Food Spawning: Apples spawn on available floor tiles

Full UI System: UMG widgets for all menus and HUD screens

Visual & Audio Feedback: Game over sounds, particle effects, ambient music

⌨️ Controls

Action

Key

Move

Arrow keys / WASD

Jump

Spacebar

Pause

P

Restart

R or on-screen

🖼️ Screenshots

Gameplay

UI & Feedback









🔧 Implementation Overview

SnakePawn

Player and AI control

Tail growth and snake head rotation

Snap-to-grid logic

ASnakeAIController

Uses BFS to calculate path toward apples

Path recalculates only when snake enters a new tile

SnakeWorld

Loads levels from text file

Spawns floor, wall, and food actors

Validates spawn logic to avoid unwalkable areas

SnakeGameMode

Manages transitions: MainMenu, Game, Pause, Outro

Tracks score, handles apple collection logic

Instantiates player and AI snakes

MyUserWidget (UMG)

Handles UI for score, level, player status

Blueprint-callable functions like SetScore(), SetLevel() etc.

🛠️ Custom Improvements

Enhanced tail-following system for visual clarity

Modularized UI layout with reusable widgets

BFS optimized for performance and grid precision

Dynamic level loading using character-coded text files

🎓 Learnings

Integrated AIModule and EnhancedInput effectively

Created polished game loop with responsive UI

Explored grid-based logic in 3D Unreal environments

Balanced AI logic and game feel for smooth player experience

📚 Acknowledgments

Originally built at Futuregames AcademyCustomized and documented by [Your Name] for showcase purposes