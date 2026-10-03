# snake-game

Snake, built as one unit-tested game core with interchangeable front ends. The rules live in `core/` as plain C with no I/O; each front end only reads input and draws the game, so every version plays by exactly the same tested rules.

The first front end runs in the terminal. More will reuse the same core.

## Terminal version

- Arrow keys or WASD to steer, `p` to pause, `r` to restart, `q` to quit
- Speeds up as the snake grows
- Score and best score for the session
- Fixed-size board and snake buffer; no allocation during play

## Requirements

- GCC or Clang, CMake 3.20+
- ncurses development headers (`libncurses-dev` on Debian/Ubuntu)

## Build and play

```bash
cmake -B build && cmake --build build
./build/terminal/terminal-snake
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

## Layout

```
core/       game rules: movement, food, collisions, score (no I/O)
terminal/   ncurses front end: drawing, input, timing
tests/      Unity tests for core/
```

## License

MIT
