#include "wasm4.h"
#include <stdlib.h>

// The Game of Life

// Rules:
// 1. Any live cell with fewer than two live neighbours dies, as if caused by underpopulation.
// 2. Any live cell with two or three live neighbours lives on to the next generation.
// 3. Any live cell with more than three live neighbours dies, as if by overpopulation.
// 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
//
// The rendering should wrap to the other side of the screen

#define WIDTH 160
#define HEIGHT 160

// The game state is stored in a 160x160 array of bits
// Each bit represents a cell
// 0 = dead, 1 = alive
uint8_t cells[WIDTH * HEIGHT / 8];
uint8_t copy[WIDTH * HEIGHT / 8];

uint8_t frames = 0;
uint8_t paused = 1;

void init () {
  srand(frames);

  // Populate the cells with values from the seed
  for (int i = 0; i < WIDTH * HEIGHT / 8; i++) {
    // Convert rand to a cell state
    cells[i] = rand() & 0xFF;
  }
}

// Compute the next generation of the game
void next_gen () {
  // Create a copy of the current state
  for (int i = 0; i < WIDTH * HEIGHT / 8; i++) {
    copy[i] = cells[i];
  }

  // Iterate over each cell
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      // Count the number of live neighbours
      int neighbours = 0;
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          // Wrap the edges of the grid
          int nx = (x + dx + WIDTH) % WIDTH;
          int ny = (y + dy + HEIGHT) % HEIGHT;

          // Count the number of live neighbours
          neighbours += (copy[ny * WIDTH / 8 + nx / 8] >> (nx % 8)) & 1;
        }
      }

      // Subtract the current cell
      neighbours -= (copy[y * WIDTH / 8 + x / 8] >> (x % 8)) & 1;

      // Apply the rules of the game
      if (neighbours < 2 || neighbours > 3) {
        // Rule 1 and 3
        cells[y * WIDTH / 8 + x / 8] &= ~(1 << (x % 8));
      } else if (neighbours == 3) {
        // Rule 4
        cells[y * WIDTH / 8 + x / 8] |= 1 << (x % 8);
      }
    }
  }
}

void start () {
  // Setup the colors
  PALETTE[0] = 0xfff7ab;
  PALETTE[1] = 0xffe528;
  PALETTE[2] = 0xffc71b;
  PALETTE[3] = 0x1e0900;
}

void update () {
  uint8_t gamepad = *GAMEPAD1;
  if (gamepad & BUTTON_1) {
    paused = 0;
    frames++;
    init();
  }

  // Set the color to black
  *DRAW_COLORS = 4;

  // If paused, show the start screen
  if (paused) {
    frames++;
    text("Press X to start", 16, 70);
    return;
  }

  next_gen();

  // Render the cells
  // void rect (int32_t x, int32_t y, uint32_t width, uint32_t height);
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if ((cells[y * WIDTH / 8 + x / 8] >> (x % 8)) & 1) {
        rect(x, y, 1, 1);
      }
    }
  }
}
