#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <emscripten.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define WIDTH 128
#define HEIGHT 64
#define VELOSITY 80
#define FLIP 7.0
#define SCALE 3

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *spritesTexture;
SDL_Texture *screenTexture;

struct SpriteImage {
    SDL_Rect rect;
    int offsetx;
    int offsety;
};
struct SpriteImage spriteRun1;
struct SpriteImage spriteRun2;
struct SpriteImage spriteSit;
struct SpritePose {
    struct SpriteImage run1;
    struct SpriteImage run2;
    struct SpriteImage sit;
};
struct SpritePose spritePose;

enum Movement {
  SIT,
  RUN_RIGHT,
  RUN_LEFT
};

struct CharState {
    float x;
    float y;
    enum Movement moveState;
};

struct CharState charState;

struct timespec frameClock, prevClock;
double total_t;

void load_textures() {
    SDL_Surface *surface = IMG_Load("assets/texture.png");
    if (!surface) {
        printf("%s\n", IMG_GetError());
    }
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));
    spritesTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void updateChar() {
    if (charState.moveState == RUN_LEFT | charState.moveState == RUN_RIGHT){
        float velosity = (charState.moveState == RUN_RIGHT) ? VELOSITY : -VELOSITY;
        float delta = velosity * total_t;
        charState.x += delta;
        if (charState.x < 0) {
            charState.x = 0;
        }
        if (charState.x > WIDTH) {
            charState.x = WIDTH;
        }
    }
}

void drawChar() {
    struct SpriteImage spr;
    if (charState.moveState == SIT) {
        spr = spritePose.sit;
    } else {
        if ((int)fmod(charState.x / FLIP, 2) == 0) {
            spr = spritePose.run1;
        }
        else {
            spr = spritePose.run2;
        }
    }
    SDL_Rect dst_rect;
    dst_rect.y = (int)charState.y - spr.offsety;
    dst_rect.h = spr.rect.h;
    dst_rect.w = spr.rect.w;
    if (charState.moveState == RUN_LEFT) {
        dst_rect.x = (int)charState.x - spr.rect.w + spr.offsetx;
        SDL_RenderCopyEx(renderer, spritesTexture, &spr.rect, &dst_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
    } else {
        dst_rect.x = (int)charState.x - spr.offsetx;
        SDL_RenderCopy(renderer, spritesTexture, &spr.rect, &dst_rect);
    }
}

void process_event(SDL_Event *event) {
    SDL_Keycode key = event->key.keysym.sym;
    
    if (event->key.type == SDL_KEYDOWN) {
        if (key == SDLK_LEFT) {
            charState.moveState = RUN_LEFT;
        }
        if (key == SDLK_RIGHT) {
            charState.moveState = RUN_RIGHT;
        }
    }
    if (event->key.type == SDL_KEYUP) {
        charState.moveState = SIT;
    }
}

void process_input() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        process_event(&event);
    }
}

double timedelta(struct timespec prevCl, struct timespec frameCl){
    int nsec_dif = frameCl.tv_nsec - prevCl.tv_nsec;
    int sec_dif = frameCl.tv_sec - prevCl.tv_sec;
    return (double)sec_dif + (double)nsec_dif/1000000000;
}

void mainloop() {
    process_input();
    SDL_SetRenderTarget(renderer, screenTexture);
    SDL_RenderClear(renderer);
    SDL_RenderFillRect(renderer, NULL);
    updateChar();
    drawChar();
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
    prevClock.tv_nsec = frameClock.tv_nsec;
    prevClock.tv_sec = frameClock.tv_sec;
    clock_gettime(CLOCK_MONOTONIC, &frameClock);
    total_t = timedelta(prevClock, frameClock);
    if (total_t > 0.1) {
        total_t = 0.1;
    }
}

void destroy() {
    SDL_DestroyTexture(screenTexture);
    SDL_DestroyTexture(spritesTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    spriteRun1.rect.x = 0;
    spriteRun1.rect.y = 0;
    spriteRun1.rect.w = 16;
    spriteRun1.rect.h = 9;
    spriteRun1.offsetx = 9;
    spriteRun1.offsety = 9;
    spriteRun2.rect.x = 16;
    spriteRun2.rect.y = 0;
    spriteRun2.rect.w = 13;
    spriteRun2.rect.h = 10;
    spriteRun2.offsetx = 7;
    spriteRun2.offsety = 10;
    spriteSit.rect.x = 29;
    spriteSit.rect.y = 0;
    spriteSit.rect.w = 6;
    spriteSit.rect.h = 9;
    spriteSit.offsetx = 3;
    spriteSit.offsety = 9;
    spritePose.run1 = spriteRun1;
    spritePose.run2 = spriteRun2;
    spritePose.sit = spriteSit;
    charState.x = 10;
    charState.y = HEIGHT;
    charState.moveState = SIT;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WIDTH * SCALE, HEIGHT * SCALE, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 175, 38, 38, 255);
    screenTexture = SDL_CreateTexture(renderer,
                        SDL_PIXELFORMAT_RGB888,
                        SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);
    load_textures();
    clock_gettime(CLOCK_MONOTONIC, &frameClock);
    emscripten_set_main_loop(mainloop, 0, 1);
    destroy();
}
