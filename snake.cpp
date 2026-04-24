//SNAKE GAME
// Made by Tamzid - 25-APRIL-2026
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <ctime>
#include <stdio.h>
#include <string>

const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 2400;
const int GRID_SIZE = 60; 

enum Direction { UP, DOWN, LEFT, RIGHT, STOP };
struct Point { int x, y; };

// --- PIXEL FONT MODULE ---
void drawDigit(SDL_Renderer* renderer, int digit, int x, int y, int size) {
    static const unsigned char font[10][5] = {
        {0x7C, 0x82, 0x82, 0x82, 0x7C}, {0x00, 0x42, 0xFE, 0x02, 0x00},
        {0x46, 0x8A, 0x92, 0x92, 0x62}, {0x44, 0x92, 0x92, 0x92, 0x6C},
        {0x18, 0x28, 0x48, 0xFE, 0x08}, {0xE4, 0xA2, 0xA2, 0xA2, 0x1C},
        {0x7C, 0x92, 0x92, 0x92, 0x4C}, {0x80, 0x8E, 0x90, 0xA0, 0xC0},
        {0x6C, 0x92, 0x92, 0x92, 0x6C}, {0x64, 0x92, 0x92, 0x92, 0x7C}  
    };
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) {
            if (font[digit][i] & (1 << (7 - j))) {
                SDL_Rect r = { x + i * size, y + j * size, size, size };
                SDL_RenderFillRect(renderer, &r);
            }
        }
    }
}

/**
 * drawArrow: Draws a pixel-art arrow inside the controller buttons.
 * Uses a corrected bitmask for perfect alignment.
 */
void drawArrow(SDL_Renderer* renderer, Direction dir, int bx, int by, int bw, int bh) {
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); // Dark Gray Arrow
    int pixelSize = 12; 
    
    // Each row of the 5x7 grid represented as a bitmask
    // Mask: 5 bits used (0-31 value)
    static const unsigned char arrowMasks[4][7] = {
        {0x04, 0x0E, 0x1F, 0x04, 0x04, 0x04, 0x04}, // UP (Stem at bottom)
        {0x04, 0x04, 0x04, 0x04, 0x1F, 0x0E, 0x04}, // DOWN (Stem at top)
        {0x04, 0x0C, 0x1C, 0x3F, 0x1C, 0x0C, 0x04}, // LEFT
        {0x04, 0x06, 0x07, 0x3F, 0x07, 0x06, 0x04}  // RIGHT
    };

    int type = 0;
    if (dir == UP) type = 0;
    else if (dir == DOWN) type = 1;
    else if (dir == LEFT) type = 2;
    else if (dir == RIGHT) type = 3;

    // Center the 5x7 arrow inside the button
    int startX = bx + (bw - (5 * pixelSize)) / 2;
    int startY = by + (bh - (7 * pixelSize)) / 2;

    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            // Check the specific bit for this pixel
            // Using (4 - col) because we want the 0th bit to be on the right
            if (arrowMasks[type][row] & (1 << (4 - col))) {
                SDL_Rect r = { startX + col * pixelSize, startY + row * pixelSize, pixelSize, pixelSize };
                SDL_RenderFillRect(renderer, &r);
            }
        }
    }
}


void drawGameOver(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_Rect panel = {140, 800, 800, 400};
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &panel);
}

void drawScore(SDL_Renderer* renderer, int score, int x, int y, int size) {
    std::string s = std::to_string(score);
    for (size_t i = 0; i < s.length(); i++) {
        drawDigit(renderer, s[i] - '0', x + (i * 6 * size), y, size);
    }
}

static Mix_Chunk *loadSound(const char *path) {
    Mix_Chunk *sound = Mix_LoadWAV(path);
    return sound;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    srand(time(NULL));

    Mix_Chunk *biteSound = loadSound("bite.mp3");
    Mix_Chunk *diedSound = loadSound("died.mp3");

    SDL_Window* window = SDL_CreateWindow("Snake Pro", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool running = true;
    Direction dir = STOP;
    std::vector<Point> snake = {{9, 11}, {9, 10}}; 
    int score = 0; 
    bool isGameOver = false;
    
    const int GAME_START_Y = 200; 
    const int GAME_HEIGHT_LIMIT = 1500; 
    const int GRID_HEIGHT_MAX = GAME_HEIGHT_LIMIT / GRID_SIZE; 

    Point fruit = {rand() % (SCREEN_WIDTH / GRID_SIZE), rand() % GRID_HEIGHT_MAX};
    Uint32 lastMoveTime = 0;
    Uint32 startTime = SDL_GetTicks();
    bool gameStarted = false;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            
            if (!isGameOver && gameStarted && (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_FINGERDOWN)) {
                int tx, ty;
                if (event.type == SDL_MOUSEBUTTONDOWN) { tx = event.button.x; ty = event.button.y; }
                else { tx = event.tfinger.x * SCREEN_WIDTH; ty = event.tfinger.y * SCREEN_HEIGHT; }

                if (ty > 1650) { 
                    if (tx > 400 && tx < 680 && ty < 1900) { if(dir != DOWN) dir = UP; } 
                    else if (tx > 400 && tx < 680 && ty > 2000) { if(dir != UP) dir = DOWN; } 
                    else if (tx < 380 && ty > 1850 && ty < 2100) { if(dir != RIGHT) dir = LEFT; } 
                    else if (tx > 700 && ty > 1850 && ty < 2100) { if(dir != LEFT) dir = RIGHT; }
                }
            }
        }

        if (!isGameOver && !gameStarted && (SDL_GetTicks() - startTime > 2000)) {
            dir = DOWN;
            gameStarted = true;
        }

        if (!isGameOver && gameStarted && (SDL_GetTicks() - lastMoveTime > 115)) {
            Point newHead = snake.front();
            if (dir == UP) newHead.y--; if (dir == DOWN) newHead.y++;
            if (dir == LEFT) newHead.x--; if (dir == RIGHT) newHead.x++;

            if (newHead.x < 0) newHead.x = (SCREEN_WIDTH / GRID_SIZE) - 1;
            else if (newHead.x >= (SCREEN_WIDTH / GRID_SIZE)) newHead.x = 0;
            if (newHead.y < 0) newHead.y = GRID_HEIGHT_MAX - 1;
            else if (newHead.y >= GRID_HEIGHT_MAX) newHead.y = 0;

            for (size_t i = 1; i < snake.size(); i++) {
                if (snake[i].x == newHead.x && snake[i].y == newHead.y) {
                    if (diedSound) Mix_PlayChannel(-1, diedSound, 0);
                    isGameOver = true;
                    startTime = SDL_GetTicks();
                    goto skip;
                }
            }

            snake.insert(snake.begin(), newHead);
            if (newHead.x == fruit.x && newHead.y == fruit.y) {
                if (biteSound) Mix_PlayChannel(-1, biteSound, 0);
                score += 10;
                fruit = {rand() % (SCREEN_WIDTH / GRID_SIZE), rand() % GRID_HEIGHT_MAX};
            } else { snake.pop_back(); }
            skip: lastMoveTime = SDL_GetTicks();
        }

        if (isGameOver && (SDL_GetTicks() - startTime > 3000)) {
            isGameOver = false;
            gameStarted = false;
            snake = {{9, 11}, {9, 10}};
            score = 0;
            dir = STOP;
            startTime = SDL_GetTicks();
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255); 
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); 
        drawScore(renderer, score, 480, 60, 10); 

        SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255); 
        for (int i = 0; i < 6; i++) {
            SDL_Rect b = { i, GAME_START_Y + i, SCREEN_WIDTH - 2 * i, GAME_HEIGHT_LIMIT - 2 * i };
            SDL_RenderDrawRect(renderer, &b);
        }

        SDL_SetRenderDrawColor(renderer, 46, 204, 113, 255);
        for (auto p : snake) {
            SDL_Rect r = {p.x * GRID_SIZE + 4, (p.y * GRID_SIZE) + GAME_START_Y + 4, GRID_SIZE - 8, GRID_SIZE - 8};
            SDL_RenderFillRect(renderer, &r);
        }

        SDL_SetRenderDrawColor(renderer, 231, 76, 60, 255);
        SDL_Rect fRect = {fruit.x * GRID_SIZE + 4, (fruit.y * GRID_SIZE) + GAME_START_Y + 4, GRID_SIZE - 8, GRID_SIZE - 8};
        SDL_RenderFillRect(renderer, &fRect);

        // Controller Buttons & Arrows
        SDL_Rect btns[4] = {
            {440, 1700, 200, 150}, // UP
            {440, 2050, 200, 150}, // DOWN
            {120, 1875, 200, 150}, // LEFT
            {760, 1875, 200, 150}  // RIGHT
        };
        Direction btnDirs[4] = {UP, DOWN, LEFT, RIGHT};

        for(int i=0; i<4; i++) {
            SDL_SetRenderDrawColor(renderer, 255, 224, 67, 150); 
            SDL_RenderFillRect(renderer, &btns[i]);
            drawArrow(renderer, btnDirs[i], btns[i].x, btns[i].y, btns[i].w, btns[i].h);
        }

        if (isGameOver) {
            drawGameOver(renderer);
            drawScore(renderer, score, 450, 950, 15);
        }

        SDL_RenderPresent(renderer);
    }

    Mix_CloseAudio(); SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window); SDL_Quit();
    return 0;
}