// A procedurally generated sea exploration game.
// The player is a seal.
// The sea map is generated procedurally. The map scrolls from bottom to top indefinitely.
// The goal is to reach a prize at the end of the map.

#include "wasm4.h"
#include "heart.h"

#define WIDTH 40
#define HEIGHT 40

int speed = 2; // frames per move

int paused = 1;
int gameover = 0;
int frames = 0;

// Define the map
unsigned short map[WIDTH][HEIGHT];
unsigned short map2[WIDTH][HEIGHT];

// Map scrolling position
int scroll = 0;

// Define the position of the player
int playerX = 0;
int playerY = 0;
int lives = 3;
int blink = 0;

void start () {
  // Setup the colors
  PALETTE[0] = 0x73aac1; // water
  PALETTE[1] = 0x5c6c6e; // rocks
  PALETTE[2] = 0xd5d8ce; // ice
  PALETTE[3] = 0x413e44; // seal
}

const uint8_t seal[] = {
  0b1001,
  0b1001,
  0b1111,
  0b1111,
  0b0110,
};

// Generate a Perlin noise for a given x and y, use a seed
float perlinNoise(int x, int y, int seed) {
  int n = x + y * 57 + seed * 131;
  n = (n << 13) ^ n;
  return (float)(1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

// Pass the map pointer
void generateMap (unsigned short map[WIDTH][HEIGHT], int seed) {
  // Generate the map using a cellular automata
  // Generate the map using Perlin noise
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      float noise = perlinNoise(i, j, seed);
      if (noise > 0.4) {
        map[i][j] = 2;
      } else if (noise > 0.3) {
        map[i][j] = 3;
      } else {
        map[i][j] = 1;
      }
    }
  }

  // Run the cellular automata 5 times, prioritizing the water
  for (int k = 0; k < 5; k++) {
    for (int i = 0; i < WIDTH; i++) {
      for (int j = 0; j < HEIGHT; j++) {
        int count = 0;
        for (int x = -1; x <= 1; x++) {
          for (int y = -1; y <= 1; y++) {
            if (i + x >= 0 && i + x < WIDTH && j + y >= 0 && j + y < HEIGHT) {
              if (map[i + x][j + y] == 1) {
                count++;
              }
            }
          }
        }
        if (count > 5) {
          map[i][j] = 1;
        }
      }
    }
  }

  // Connect the waters with rivers
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      if (map[i][j] == 1) {
        int count = 0;
        for (int x = -1; x <= 1; x++) {
          for (int y = -1; y <= 1; y++) {
            if (i + x >= 0 && i + x < WIDTH && j + y >= 0 && j + y < HEIGHT) {
              if (map[i + x][j + y] == 1) {
                count++;
              }
            }
          }
        }
        if (count > 4) {
          map[i][j] = 1;
        }
      }
    }
  }
}

void init () {
  paused = 0;
  gameover = 0;
  scroll = 0;
  lives = 3;
  blink = 0;

  // Generate the map
  generateMap(map, frames * 2);
  generateMap(map2, frames * 2);

  // Make half of the initial map just water
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT / 2; j++) {
      map[i][j] = 1;
    }
  }


  // Place the player in the middle of the screen
  playerX = SCREEN_SIZE / 2;
  playerY = 10;
}

int read_pixel (int x, int y) {
  // The byte index into the framebuffer that contains (x, y)
  int idx = (y*160 + x) >> 2;

  // Calculate the bits within the byte that corresponds to our position
  int shift = (x & 0b11) << 1;
  int mask = 0b11 << shift;
  // Read the pixel color
  int color = (FRAMEBUFFER[idx] & mask) >> shift;
  return color;
}

void update () {
  frames = (frames + 1) % (HEIGHT * 1000);

  uint8_t gamepad = *GAMEPAD1;
  if (gamepad & BUTTON_1) {
    init();
  }

  if (paused) {
    *DRAW_COLORS = 3;
    text("Press X to start", 16, 70);
    *DRAW_COLORS = 4;
    text("Press X to start", 17, 71);
    return;
  }

  if (!gameover && (frames % speed == 0)) {
    // Scroll the map
    scroll++;

    // Once the second map is scrolled, generate a new one
    if (scroll >= HEIGHT * 4) {
      scroll = 0;

      for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
          map[i][j] = map2[i][j];
        }
      }

      generateMap(map2, frames);
    }

    // Player movement
    if (gamepad & BUTTON_LEFT) {
      playerX--;
    } else if (gamepad & BUTTON_RIGHT) {
      playerX++;
    } else if (gamepad & BUTTON_UP) {
      playerY -= 2;
    } else if (gamepad & BUTTON_DOWN) {
      playerY++;
    }

    if (playerX < 0) {
      playerX = 0;
    } else if (playerX >= SCREEN_SIZE) {
      playerX = SCREEN_SIZE - 1;
    }
    if (playerY < 0) {
      playerY = 0;
    } else if (playerY >= SCREEN_SIZE) {
      playerY = SCREEN_SIZE - 1;
    }
  }

  // void rect (int32_t x, int32_t y, uint32_t width, uint32_t height);

  // Draw the map with an offset of scroll
  // If scroll is less than HEIGHT, draw the first map
  // If scroll is greater than HEIGHT, draw the second map
  for (int i = 0; i < SCREEN_SIZE; i++) {
    for (int j = 0; j < SCREEN_SIZE; j++) {
      int di = i / 4;
      int dj = j + scroll;
      int fdj = dj / 4;

      if (fdj < HEIGHT) {
        *DRAW_COLORS = map[di][fdj];
      } else {
        *DRAW_COLORS = map2[di][fdj % HEIGHT];
      }

      rect(i, j, 4, 4);

      if (!gameover && (blink == 0)) {
        // Collision detection
        if (
            (*DRAW_COLORS != 1) &&
            (i >= playerX) &&
            (i < (playerX + 4)) &&
            (j >= playerY) &&
            (j < (playerY + 4))
            ) {
          lives--;
          blink = 50;

          if (lives == 0) {
            gameover = 1;
          }
        }
      }
    }
  }

  // Blinking effect
  if (blink > 0) {
    blink--;
  }

  // Draw the player
  // void blit (const uint8_t* data, int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t flags);
  if (!gameover || (blink > 0)) {
    *DRAW_COLORS = blink && (blink % 2 == 0) ? 3 : 4;
    blit(seal, playerX, playerY, 4, 4, BLIT_1BPP);
  }

  if (gameover && (blink == 0)) {
    *DRAW_COLORS = 3;
    text("Game over!", 41, 59);
    text("Press X to restart", 9, 79);

    *DRAW_COLORS = 4;
    text("Game over!", 42, 60);
    text("Press X to restart", 10, 80);
  }

  // Draw the lives
  *DRAW_COLORS = 4;
  for (int i = 0; i < lives; i++) {
    blit(heart, 130 + i * 10, 2, heartWidth, heartHeight, heartFlags);
  }

  if (blink > 0) {
    *DRAW_COLORS = (blink % 2 == 0) ? 3 : 4;
    blit(heart, 130 + lives * 10, 2, heartWidth, heartHeight, heartFlags);
  }
}
