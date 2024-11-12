# Tic-Tac-Toe Bot

This project is a simple implementation of the classic game Tic-Tac-Toe, where players can compete against a computer opponent. The game is written in C++ using OpenGL and GLUT libraries for graphics rendering and user interaction.

## Dependencies

Before compiling the program, ensure that the following dependencies are installed:

- libglu1-mesa-dev
- freeglut3-dev
- mesa-common-dev

You can install these dependencies using your package manager.

## Compilation

To compile the program, you can use the g++ compiler to compile the `main.cpp` file. Here's a sample compilation command:
```bash
g++ main.cpp -o tic_tac_toe -lGL -lGLU -lglut
```

## Game Features

- **Player vs. Computer:** Play against a computer opponent that utilizes the minimax algorithm with alpha-beta pruning for efficient move searching.
- **Position Evaluation:** The computer evaluates positions using the Aho–Corasick algorithm for faster position evaluation.
- **Graphics:** Simple graphics interface provided by OpenGL and GLUT for an interactive gaming experience.

## Usage

After compiling the program, run the executable `tic_tac_toe` to start the game. Follow the on-screen instructions to play against the computer.

## Contributions

Contributions to this project are welcome. If you find any issues or have suggestions for improvements, feel free to open an issue or create a pull request on the GitHub repository.

---

**Note:** This project was created for educational purposes and as a demonstration of advanced algorithms on C++. Enjoy playing Tic-Tac-Toe!
