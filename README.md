# snake-game

[![CI](https://github.com/Tonyc310/snake-game/actions/workflows/ci.yml/badge.svg)](https://github.com/Tonyc310/snake-game/actions/workflows/ci.yml)

Snake, built as one unit-tested game core with interchangeable front ends. The rules live in `core/` as plain C with no I/O; each front end only reads input and draws the game, so every version plays by exactly the same tested rules.

The first front end runs in the terminal. More will reuse the same core.

## Terminal version

```
┌────────────────────────────────────────┐
│                                        │
│                        @               │
│                        o               │
│                  *     o               │
│                        o               │
│                        o               │
│                        o               │
│                        o               │
│                        o               │
│                        o               │
│                        o               │
│                        o o             │
└────────────────────────────────────────┘
score 9   best 9
arrows/WASD steer, p pause, q quit
```

- Arrow keys or WASD to steer, `p` to pause, `r` to restart, `q` to quit
- Speeds up as the snake eats, from 150 ms per step down to 60 ms
- Score and best score for the session
- Pauses itself if the terminal becomes too small to show the board
- Fixed-size board and snake buffer; no allocation during play

## Requirements

- GCC or Clang, CMake 3.20+
- ncurses development headers (`libncurses-dev` on Debian/Ubuntu)
- A terminal of at least 42×16

## Build and play

```bash
cmake -B build && cmake --build build
./build/terminal/terminal-snake
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

CI builds with GCC and Clang under AddressSanitizer and UndefinedBehaviorSanitizer and runs the tests.

## Layout

```
core/       game rules: movement, food, collisions, score, speed (no I/O)
terminal/   ncurses front end: drawing, input, timing
tests/      Unity tests for core/
```

## License

MIT
