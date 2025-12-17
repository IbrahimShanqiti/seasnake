#include<stdio.h>
#include<stdlib.h>
#include <SDL.h>
#define WIDTH 600
#define HEIGHT 600
#define MOVE_EVERY 40 
#define MOVE_QUEUE_MAX 10
#define BLOCK_SQUARE 20

typedef struct {
    SDL_Rect *rects;
    int *moves;
    int num_s;
} Snake;

typedef struct foodNode {
    struct foodNode *next;
    SDL_Rect curr;
} foodNode;

int MOVE_QUEUE[MOVE_QUEUE_MAX];
int queue_head = 0;
int queue_end = 0;

foodNode* addFood(foodNode *first, SDL_Rect *curr) {
    foodNode *temp = (foodNode*) malloc(sizeof *temp);
    temp->curr = *curr;
    temp->next = NULL;

    if (!first) {
        first = temp;
    } else {
        foodNode *nxt = first;
        while (nxt->next) {
            nxt = nxt->next;
        }
        nxt->next = temp;
    }
    return first;
}

foodNode* removeFood(foodNode *first, int i) {
    if (!first) {
        return NULL;
    }
    foodNode *temp;
    if (i == 0) {
        temp = first;
        first = first->next;
        free(temp);
        return first;
    }
    int l = 0;
    foodNode *prev = NULL;
    temp = first;
    while (temp && l != i) {
        prev = temp;
        temp = temp->next;
        l++;
    }
    if (!temp) {
        return first;
    }
    prev->next = temp->next;
    free(temp);
    return first;
}

void enqueue(int i) {
    if (queue_head - queue_end == 10) {
        fprintf(stderr, "UHOH! MOVE QUEUE FULL!");
    }
    MOVE_QUEUE[queue_end] = i;
    queue_end = (queue_end + 1) % MOVE_QUEUE_MAX;
}

int dequeue(int current_dir) {
    if (queue_head - queue_end == 0) {
        return current_dir;
    }
    int to_ret = MOVE_QUEUE[queue_head];
    queue_head = (queue_head + 1) % MOVE_QUEUE_MAX;
    return to_ret;
    
}

int snake_collision_check(Snake *snake, SDL_Rect curr, int i) {
    // int i = 0 if checking for food eaten, i = 1 if for self-collision
    for (int j = i; j < snake->num_s; j++) {
        if (curr.x == snake->rects[j].x && curr.y == snake->rects[j].y) {
            return 1;
        }
    }
    return 0;
}

void move_snake(SDL_Surface *psurface, Snake *snake, int direction) {

    for (int i = 0; i < snake->num_s; i++) {
        SDL_FillRect(psurface, &snake->rects[i], 0x000000);
    }

    SDL_Rect prev = snake->rects[0];
    int prev_dir = snake->moves[0];
    snake->moves[0] = direction;
    
    SDL_Rect *head = &snake->rects[0];
    switch (direction) {
        case 0: head->y = (head->y - BLOCK_SQUARE + HEIGHT) % HEIGHT; break;
        case 1: head->x = (head->x + BLOCK_SQUARE) % WIDTH; break;
        case 2: head->y = (head->y + BLOCK_SQUARE) % HEIGHT; break;
        case 3: head->x = (head->x - BLOCK_SQUARE + WIDTH) % WIDTH; break;
    }

    for (int i = 1; i < snake->num_s; i++) {
        SDL_Rect temp = snake->rects[i];
        snake->rects[i] = prev;
        prev = temp;
        
        int temp_dir = snake->moves[i];
        snake->moves[i] = prev_dir;
        prev_dir = temp_dir;
    }
    for (int i = 0; i < snake->num_s; i++) {
        SDL_FillRect(psurface, &snake->rects[i], 0xFFFFFF);
    }
}

int check_if_food_eaten(foodNode **first, SDL_Rect check) {
    if (!*first) {
        return 0;
    }

    if ((*first)->curr.x == check.x && (*first)->curr.y == check.y) {
        foodNode *temp = *first;
        *first = (*first)->next;
        free(temp);
        return 1;
    }

    foodNode *prev = *first;
    foodNode *curr = (*first)->next;
    while (curr) {
        if (curr->curr.x == check.x && curr->curr.y == check.y) {
            prev->next = curr->next;
            free(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;
    }

void create_food(SDL_Surface *psurface, Snake *snake, foodNode **first) {
    int food_rand = (int)((double)rand()/(double)RAND_MAX * 30000);
    if (food_rand > 29980) {
        SDL_Rect new_food;
        do {
            int x_coord = ((rand() % (WIDTH/BLOCK_SQUARE))) * BLOCK_SQUARE;
            int y_coord = ((rand() % (HEIGHT/BLOCK_SQUARE))) * BLOCK_SQUARE;
            new_food = (SDL_Rect){x_coord, y_coord, BLOCK_SQUARE, BLOCK_SQUARE};
        } while (snake_collision_check(snake, new_food, 0));
        
        SDL_FillRect(psurface, &new_food, 0xFF0000);
        *first = addFood(*first, &new_food);
    }
}

void grow_snake(Snake *snake, foodNode **first, int i, int *score) { 
    int old = *score;
    *score += check_if_food_eaten(first, snake->rects[0]);
    if (old != *score) {
        for (int l = 0; l < i; l++) {
            SDL_Rect new_rec;
            new_rec.h = BLOCK_SQUARE;
            new_rec.w = BLOCK_SQUARE;
            int last_dir = snake->moves[snake->num_s - 1];
            SDL_Rect last_rec = snake->rects[snake->num_s - 1];
            if (last_dir == 0) {
                new_rec.x = last_rec.x;
                new_rec.y = last_rec.y + BLOCK_SQUARE;
            } else if (last_dir == 1) {
                new_rec.x = last_rec.x + BLOCK_SQUARE;
                new_rec.y = last_rec.y;
            } else if (last_dir == 2) {
                new_rec.x = last_rec.x;
                new_rec.y = last_rec.y - BLOCK_SQUARE;
            } else {
                new_rec.x = last_rec.x - BLOCK_SQUARE;
                new_rec.y = last_rec.y;
            }
            snake->rects[snake->num_s] = new_rec;
            snake->moves[snake->num_s] = last_dir;
            snake->num_s++;
        }
    }
    
}

void snake_init(Snake *snake) {
    snake->rects = malloc(900 * sizeof(SDL_Rect));
    snake->moves = malloc(900 * sizeof(int));
    snake->num_s = 1;
    snake->rects[0] = (SDL_Rect){0, 0, BLOCK_SQUARE, BLOCK_SQUARE};
    snake->moves[0] = 1; 
}


int main(int argv, char *argc[]) {
    SDL_Init(SDL_INIT_VIDEO);
    int SNAKE_GROW_RATE;
    if (argv == 1) {
        SNAKE_GROW_RATE = 2;
    } else if (argv == 2) {
        int attempt = atoi(argc[1]);
        if (attempt <= 0) {
            printf("Usage: ./snake [growth rate as integer]\n");
            return -1;
        }
        SNAKE_GROW_RATE = attempt;
    } else {
        printf("Usage: snake [growth rate as integer]");
    }

    // INITS : ) (no safety checks bc lowk it is what it is)
    SDL_Window *pwindow = SDL_CreateWindow("Practice", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Surface *psurface = SDL_GetWindowSurface(pwindow);
    SDL_Event e;
    Snake snake;
    snake_init(&snake);
    foodNode *first = NULL;
    int quit = 0;
    int direct = 1;
    int score = 0;
    int frame_count = 0;

    // SDL Loop
    while (quit == 0) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.scancode) {
                    case SDL_SCANCODE_W:
                        enqueue(0);
                        break;
                    case SDL_SCANCODE_D:
                        enqueue(1);

                        break;
                    case SDL_SCANCODE_S:
                        enqueue(2);

                        break;
                    case SDL_SCANCODE_A:
                        enqueue(3);
                        break;
                    default:
                        break;
                }
            }
        }
        if (frame_count % MOVE_EVERY == 0) {
            int new = dequeue(direct);
            if (new == -1) {
                continue;
            } else if ((new == 0 && direct == 2) || (new == 2 && direct == 0)) {
                continue;
            } else if ((new == 1 && direct == 3) || (new == 3 && direct == 1)) {
                continue;
            } else {
                direct = new;
            }
            move_snake(psurface, &snake, direct);
            if (snake_collision_check(&snake, snake.rects[0], 1)) {
                quit = 1;
            }
        }

        create_food(psurface, &snake, &first);
        grow_snake(&snake, &first, SNAKE_GROW_RATE, &score);


        frame_count++;
        SDL_Delay(1);
        SDL_UpdateWindowSurface(pwindow);
    }


    // Frees n Quits
    free(snake.rects);
    free(snake.moves);
    foodNode *temp = first;
    while (temp) {
        foodNode *next = temp->next;
        free(temp);
        temp = next;
    }
    SDL_DestroyWindow(pwindow);
    SDL_Quit();
    return 0;
}
