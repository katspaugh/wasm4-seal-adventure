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

uint8_t frames = 0;
uint8_t paused = 1;

void write_pixel (int x, int y) {
  // The byte index into the framebuffer that contains (x, y)
  int idx = (y*160 + x) >> 2;

  // Calculate the bits within the byte that corresponds to our position
  int shift = (x & 0b11) << 1;
  int mask = 0b11 << shift;

  // Use the first DRAW_COLOR as the pixel color
  int palette_color = *DRAW_COLORS & 0b1111;
  int color = (palette_color - 1) & 0b11;

  // Write to the framebuffer
  FRAMEBUFFER[idx] = (uint8_t)(color << shift) | (FRAMEBUFFER[idx] & ~mask);
}

int read_pixel (int x, int y) {
  // The byte index into the framebuffer that contains (x, y)
  int idx = (y*160 + x) >> 2;

  // Calculate the bits within the byte that corresponds to our position
  int shift = (x & 0b11) << 1;
  int mask = 0b11 << shift;

  // Read the pixel color
  int color = (FRAMEBUFFER[idx] & mask) >> shift;

  // Return the color
  return color;
}

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
  // Use read_pixel to read the current state of the cell
  // Write the next state into the cells array

  // Iterate over the cells
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      // Count the number of live neighbors
      int neighbors = 0;

      // Iterate over the neighbors
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          // Skip the current cell
          if (dx == 0 && dy == 0) continue;

          // Wrap the coordinates
          int nx = (x + dx + WIDTH) % WIDTH;
          int ny = (y + dy + HEIGHT) % HEIGHT;

          // Read the pixel
          int color = read_pixel(nx, ny);

          // Count the live neighbors
          neighbors += color >= 1;
        }
      }

      // Compute the next state
      int next_state = 0;
      int current_state = read_pixel(x, y) > 0;

      // Rule 1
      if (current_state && neighbors < 2) {
        next_state = 0;
      }

      // Rule 2
      if (current_state && (neighbors == 2 || neighbors == 3)) {
        next_state = 1;
      }

      // Rule 3
      if (current_state && neighbors > 3) {
        next_state = 0;
      }

      // Rule 4
      if (!current_state && neighbors == 3) {
        next_state = 1;
      }

      // Write the next state into the cells array
      int idx = (y*WIDTH + x) >> 3;
      int shift = (x & 0b111);
      int mask = 0b1 << shift;
      cells[idx] = (uint8_t)(next_state << shift) | (cells[idx] & ~mask);
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

  // Render the cells
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if ((cells[y * WIDTH / 8 + x / 8] >> (x % 8)) & 1) {
        write_pixel(x, y);
      }
    }
  }

  // Compute the next generation
  next_gen();
}
