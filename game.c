#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>



#define MIN_N 5
#define MAX_N 15
#define EMPTY '.'
#define EXTRACT 'X'
#define PLAYER '@'
#define WALL '#'
#define INTEL 'I'
#define LIFE 'L'


char **allocGrid(int n) {
    char **g = (char **)malloc(n * sizeof(char *));
    if (g == NULL) return NULL;

    for (int i = 0; i < n; i++) {
        g[i] = (char *)malloc(n * sizeof(char));
        if (g[i] == NULL) {
            for (int k = 0; k < i; k++) free(g[k]);
            free(g);
            return NULL;
        }
    }
   return g;
}
void freeGrid(char **g, int n) {
    for (int i = 0; i < n; i++) free(g[i]);
    free(g);
}

void fillGrid(char **g, int n, char ch) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            g[i][j] = ch;
      
}





void placeExtraction(char **g, int n) {
    int r, c;
    do {
        r = rand() % n;
        c = rand() % n;
    } while (g[r][c] != EMPTY);

    g[r][c] = EXTRACT;
}

void placePlayer(char **g, int n, int *pr, int *pc) {
    int r, c;
    do {
        r = rand() % n;
        c = rand() % n;
    } while (g[r][c] != EMPTY);

    *pr = r;
    *pc = c;
}

void placeWalls(char **g, int n) {
    int wallCount = (n * n) / 7;   // ~15% of the grid
    if (wallCount < 5) wallCount = 5;

    int placed = 0;
    while (placed < wallCount) {
        int r = rand() % n;
        int c = rand() % n;

        if (g[r][c] == EMPTY) {
            g[r][c] = WALL;
            placed++;
        }
    }
}

void placeIntel(char **g, int n) {
    int placed = 0;

    while (placed < 3) {
        int r = rand() % n;
        int c = rand() % n;

        if (g[r][c] == EMPTY) {
            g[r][c] = INTEL;
            placed++;
        }
    }
}

void placeLives(char **g, int n) {
    int placed = 0;

    while (placed < 2) {
        int r = rand() % n;
        int c = rand() % n;

        if (g[r][c] == EMPTY) {
            g[r][c] = LIFE;
            placed++;
        }
    }
}

int getMoveDelta(char move, int *dr, int *dc) {
    *dr = 0;
    *dc = 0;

    if (move == 'W') { *dr = -1; return 1; }
    if (move == 'S') { *dr =  1; return 1; }
    if (move == 'A') { *dc = -1; return 1; }
    if (move == 'D') { *dc =  1; return 1; }

    return 0;
}

void loseLife(int *lives) {
    (*lives)--;
    printf("❗ Invalid move! Lives -1 (Lives now: %d)\n", *lives);
}

 

int main(){

    srand(time(NULL));

    int n;
    printf("Enter the grid size (5 <= N <= 15):  ");
    scanf("%d", &n);


    int pr, pc;

   

   char **grid = allocGrid(n);
   
   fillGrid(grid, n, EMPTY);
   placeWalls(grid, n);
   placeIntel(grid, n);
   placeLives(grid, n);
   placeExtraction(grid, n);
   placePlayer(grid, n, &pr, &pc);

   int lives = 3;
   int intel = 0;
    
    while (1) {

    printf("\nLives: %d | Intel: %d\n", lives, intel);

    for (int j = 0; j < n; j++) printf(" __");
    printf("\n");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == pr && j == pc) printf("|%c ", PLAYER);
            else printf("|%c ", grid[i][j]);
        }
        printf("|\n");

        for (int j = 0; j < n; j++) printf("|__");
        printf("|\n");
    }

    char move;
    printf("\nMove (W/A/S/D) or Q to quit: ");
    scanf(" %c", &move);
    move = toupper((unsigned char)move);

    if (move == 'Q') {
        printf("You quit.\n");
        break;
    }

    int dr, dc;
    if (!getMoveDelta(move, &dr, &dc)) {
        loseLife(&lives);
        if (lives <= 0) { printf("Game Over! Lives reached 0.\n"); break; }
        continue;
    }

    int nr = pr + dr;
    int nc = pc + dc;

    if (nr < 0 || nr >= n || nc < 0 || nc >= n) {
        loseLife(&lives);
        if (lives <= 0) { printf("Game Over! Lives reached 0.\n"); break; }
        continue;
    }

    if (grid[nr][nc] == WALL) {
        loseLife(&lives);
        if (lives <= 0) { printf("Game Over! Lives reached 0.\n"); break; }
        continue;
    }

    // valid move
    pr = nr;
    pc = nc;
}

freeGrid(grid, n);
return 0;
}