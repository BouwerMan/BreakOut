#include <SDL2/SDL.h>
#include <stdbool.h>

/** Keybindings */
#define QUIT_KEY SDLK_ESCAPE
/** Keybindings */

/** Log type defines */
#define LOG_INFO 0
#define LOG_ERROR 1
/** Log type defines */

/** Window defines */
#define WIDTH 800
#define HEIGHT 500
/** Window defines */

/** Blocks defines */
#define BLOCKS_TALL 8
#define BLOCKS_WIDE 14
/** Blocks defines */

#define TICK_INTERVAL    30

static uint32_t next_time;

typedef struct Ball {
    float x;
    float y;
    int32_t radius;
    float speedX;
    float speedY;
    SDL_Color color;
} Ball;

typedef struct Brick {
    SDL_Rect rect;
    SDL_Color color;
    int destroyed;
} Brick;

SDL_Window* screen = NULL;
SDL_Renderer* renderer = NULL;
SDL_Event event;
SDL_Color bgc = {0};

int errorCount = 0;
int keyPressed, mousePressed;
int gameRunning = 1;

void Log(int type, char* msg) {
    switch (type)
    {
    case LOG_INFO:
        printf("[INFO]: %s\n", msg);
        break;

    case LOG_ERROR:
        printf("[ERROR]: %s\n", msg);
        errorCount++;
        break;
    
    default:
        msg = "Unknown log type";
        printf("[ERROR]: %s\n", msg);
        errorCount++;
        break;
    }
    
}

void drawRectangle(int h, int w, int x, int y, SDL_Color color) {
    SDL_Rect rect;
    rect.h = h;
    rect.w = w;
    rect.x = x;
    rect.y = y;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void drawCircle(SDL_Renderer* render, Ball circle) {
    SDL_Color color = circle.color;
    SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
    int32_t x = circle.x;
    int32_t y = circle.y;
    uint32_t radius = circle.radius;
    for (size_t w = 0; w < radius * 2; w++)
    {
        for (size_t h = 0; h < radius * 2; h++)
        {
            uint32_t dx = radius - w; // horizontal offset
            uint32_t dy = radius - h; // vertical offset
            if ((dx*dx + dy*dy) <= (radius * radius))
            {
                SDL_RenderDrawPoint(render, x + dx, y + dy);
            }
        }
    }
}

uint32_t time_left(void)
{
    uint32_t now;

    now = SDL_GetTicks();
    if(next_time <= now)
        return 0;
    else
        return next_time - now;
}

// Object-to-object bounding-box collision detector:
int Sprite_Collide(Brick brick, Ball ball) {
    SDL_Rect rect = brick.rect;

    int left1, left2;
    int right1, right2;
    int top1, top2;
    int bottom1, bottom2;

    left1 = rect.x;
    left2 = ball.x;
    right1 = rect.x + rect.w;
    right2 = ball.x + ball.radius;
    top1 = rect.y;
    top2 = ball.y;
    bottom1 = rect.y + rect.h;
    bottom2 = ball.y + ball.radius;

    if (bottom1 < top2) return(0);
    if (top1 > bottom2) return(0);

    if (right1 < left2) return(0);
    if (left1 > right2) return(0);

    return(1);

};

int collide(Brick brick, Ball* ball) {
    SDL_Rect rect = brick.rect;

    // brick is object1 and ball is object2 for the following vars
    int left1, left2;
    int right1, right2;
    int top1, top2;
    int bottom1, bottom2;

    left1 = rect.x;
    left2 = ball->x - ball->radius;
    right1 = rect.x + rect.w;
    right2 = ball->x + ball->radius;
    top1 = rect.y;
    top2 = ball->y - ball->radius;
    bottom1 = rect.y + rect.h;
    bottom2 = ball->y + ball->radius;

    if (bottom1 < top2) return(0);
    if (top1 > bottom2) return(0);

    if (right1 < left2) return(0);
    if (left1 > right2) return(0);

    Log(LOG_INFO, "Found collision!");
    return(1);
}

// TODO: Color properly
void initBlocks(Brick bricks[BLOCKS_TALL][BLOCKS_WIDE]) {
    Log(LOG_INFO, "Initializing bricks");
    Brick brick = {0};
    for (size_t i = 0; i < BLOCKS_TALL; i++) {
        for (size_t j = 0; j < BLOCKS_WIDE; j++) {
            brick.rect.h = 10;
            brick.rect.w = WIDTH / BLOCKS_WIDE - 10; //? - 10 is padding?
            brick.rect.x = 5 + j * (brick.rect.w+10) % WIDTH;
            brick.rect.y = 10 + i * (brick.rect.h+10) % HEIGHT;
            brick.color = (SDL_Color){
                .r = 255,
                .g = 255,
                .b = 255,
                .a = 255,
            };
            brick.destroyed = 0;

            bricks[i][j] = brick;
        }
    }
}

void initWindow() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, SDL_WINDOW_SHOWN | SDL_RENDERER_PRESENTVSYNC, &screen, &renderer);
    if (!screen) {
        Log(LOG_ERROR, "initWindow failed ot create window");
    }
    SDL_SetWindowTitle(screen, "SDL2Test");
}

void drawBlocks(Brick bricks[BLOCKS_TALL][BLOCKS_WIDE]) {
    for (size_t i = 0; i < BLOCKS_TALL; i++) {
        for (size_t j = 0; j < BLOCKS_WIDE; j++) {
            if (bricks[i][j].destroyed) { continue; }
            Brick brick = bricks[i][j];
            drawRectangle(brick.rect.h, brick.rect.w, brick.rect.x, brick.rect.y, brick.color);
        }
    }
}

/* Cleans up */
void done() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(screen);
	//Quit SDL
	SDL_Quit();
	exit(0);
}

int main(void) {
    initWindow();
    
    Brick bricks[BLOCKS_TALL][BLOCKS_WIDE] = {0};
    initBlocks(bricks);

    next_time = SDL_GetTicks() + TICK_INTERVAL;

    SDL_Color ballColor = {
        .r = 255,
        .g = 255,
        .b = 255,
        .a = 255
    };

    Ball ball = {
        .x = WIDTH/2 + 20,
        .y = 100,
        .radius = 10,
        .speedX = 1,
        .speedY = -1,
        .color = ballColor
    };
    
    while (gameRunning) {
        SDL_RenderClear(renderer);

        ball.x += ball.speedX;
        ball.y += ball.speedY;

        while(SDL_PollEvent(&event)) {
            switch (event.type)
            {
            case SDL_KEYDOWN:
                // keyPressed = event.key.keysym.sym;
                switch (event.key.keysym.sym)
                {
                case QUIT_KEY:
                    gameRunning = 0;
                    break;
                default:
                    break;
                }
                break;
            case SDL_QUIT: /* if mouse click to close window */
                gameRunning = 0;
                break;
            case SDL_KEYUP:
                break;
            default:
                break;
            }
        }

        // Ball bounds detection
        if (ball.x == (WIDTH - ball.radius) || ball.x == ball.radius) {
            ball.speedX *= -1;
        }
        if (ball.y == (HEIGHT - ball.radius) || ball.y == ball.radius) {
            ball.speedY *= -1;
        }
        
        //! Unsure if this works properly lol
        // Ball rectangle collision
        for (size_t i = 0; i < BLOCKS_TALL; i++) {
            for (size_t j = 0; j < BLOCKS_WIDE; j++) {
                bool collision = false;
                // Put this above for so we only deal with vars if brick isnt destroyed.
                // Probably not worth any potential (virtually none) performance improvements.
                if (bricks[i][j].destroyed) { continue; }
                Brick brick = bricks[i][j];
                if(collide(brick, &ball)) {
                    
                }
                //if (Sprite_Collide(brick, ball)) { break; }
                // SDL_Rect rect = brick.rect;

                // // WTF is this mess
                // bool top = (ball.y + ball.radius == rect.y) && ((ball.x <= (rect.x + rect.w)) && (ball.x >= rect.x));
                // bool bottom = (ball.y - ball.radius == rect.y + rect.h) && ((ball.x <= (rect.x + rect.w)) && (ball.x >= rect.x));
                // bool right = (ball.x - ball.radius == rect.x + rect.w) && ((ball.y <= (rect.y + rect.h)) && (ball.y >= rect.y));
                // bool left = (ball.x + ball.radius == rect.x) && ((ball.y <= (rect.y + rect.h)) && (ball.y >= rect.y));
                
                // if (top || bottom) {
                //     Log(LOG_INFO, "Top or Bottom collision.");
                //     ball.speedY *= -1;
                //     // "Destroys" brick
                //     brick.color = (SDL_Color){0};
                //     bricks[i][j].destroyed = 1;
                //     collision = true;
                // }
                // if (right || left) {
                //     Log(LOG_INFO, "Right or Left collision.");
                //     ball.speedX *= -1;
                //     // "Destroys" brick
                //     brick.color = (SDL_Color){0};
                //     bricks[i][j].destroyed = 1;
                //     collision = true;
                // }
                // if (collision) { break; }
            }
        }

        // Rendering everything
        drawCircle(renderer, ball);
        drawBlocks(bricks);
        SDL_SetRenderDrawColor(renderer, bgc.r, bgc.g, bgc.b, bgc.a);
        SDL_RenderPresent(renderer);
        
        // Constant tick rate stuff
        SDL_Delay(time_left());
        next_time += TICK_INTERVAL;
    }

    done();
    return 0;
}