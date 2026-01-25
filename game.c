#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>



#define MIN_N 5
#define MAX_N 15
#define EMPTY '.'
#define EXTRACT 'X'
#define PLAYER '@'
#define PLAYER2 '&'
#define WALL '#'
#define INTEL 'I'
#define LIFE 'L'



typedef struct {
    char symbol;     // '@'
    int r, c;        // position
    int lives;       // lives
    int intel;       // intel collected
    int active;      // 1 active, 0 inactive
    int isHuman;     // 1 human, 0 computer (for later)
} Player;
 

void logState(FILE *fp, char **grid, int n, Player *p1, Player *p2, char move, const char *note) {
    fprintf(fp, "Move: %c | %s\n", move, note);
    fprintf(fp, "P1(%c) Lives: %d | Intel: %d | Pos: (%d,%d)\n", p1->symbol, p1->lives, p1->intel, p1->r, p1->c);
    fprintf(fp, "P2(%c) Lives: %d | Intel: %d | Pos: (%d,%d)\n", p2->symbol, p2->lives, p2->intel, p2->r, p2->c);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == p1->r && j == p1->c) fputc(p1->symbol, fp);
            else if (i == p2->r && j == p2->c) fputc(p2->symbol, fp);
            else fputc(grid[i][j], fp);
            fputc(' ', fp);
        }
        fputc('\n', fp);
    }
    fprintf(fp, "----------------------------\n");
}



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
    printf("Invalid move! Lives -1 (Lives now: %d)\n", *lives);
}





int main(){

    srand(time(NULL));

   printf("===Welcome to SpyNet-The Codebreaker Protocol====\n");

    int n;
    printf("\nEnter the grid size (5 <= N <= 15):  ");
    scanf("%d", &n);


    Player p;
   p.symbol = PLAYER;
   p.lives = 3;
   p.intel = 0;
   p.active = 1;
   p.isHuman = 1;

    Player p2;
    p2.symbol = PLAYER2;
    p2.lives = 3;
    p2.intel = 0;
    p2.active = 1;
    p2.isHuman = 1;

   

   char **grid = allocGrid(n);

   FILE *logfp = fopen("spynet_log1.txt", "w");
  if (logfp == NULL) {
    printf("Could not open log file.\n");
    freeGrid(grid, n);
    return 1;
}
   
   fillGrid(grid, n, EMPTY);
   placeWalls(grid, n);
   placeIntel(grid, n);
   placeLives(grid, n);
   placeExtraction(grid, n);
   placePlayer(grid, n, &p.r, &p.c);
    char saved = grid[p.r][p.c];
    grid[p.r][p.c] = WALL;      // temporarily block P1 cell
    placePlayer(grid, n, &p2.r, &p2.c);
    grid[p.r][p.c] = saved;     // restore cell

   int lives = 3;
   int intel = 0;

   logState(logfp, grid, n, &p, &p2, '-', "Initial state");
    
    while (1) {

    printf("\nLives: %d | Intel: %d\n", lives, intel);

    for (int j = 0; j < n; j++) printf(" __");
    printf("\n");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == p.r && j == p.c) printf("|%c ", p.symbol);
            else if (i == p2.r && j == p2.c) printf("|%c ", p2.symbol);
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
       
        logState(logfp, grid, n, &p, &p2, move, "Quit");
        break;
    }

    int dr, dc;
    if (!getMoveDelta(move, &dr, &dc)) {
        loseLife(&lives);
        if (lives <= 0) { printf("Game Over! Lives reached 0.\n");
            
            
        logState(logfp, grid, n, &p, &p2, move, "Invalid key");

            break; }
        continue;
    }

   int nr = p.r + dr;
   int nc = p.c + dc;

    if (nr < 0 || nr >= n || nc < 0 || nc >= n) {
        loseLife(&lives);
        if (lives <= 0) { printf("Game Over! Lives reached 0.\n"); 
            
            logState(logfp, grid, n, &p, &p2, move, "Outside grid");
            break; }
        continue;
    }

    if (grid[nr][nc] == WALL) {
        loseLife(&lives);
        if (lives <= 0) { printf("Game Over! Lives reached 0.\n");
            
            logState(logfp, grid, n, &p, &p2, move, "Hit Wall");
            break; }
        continue;
    }

    // valid move: check what is on the target cell
if (grid[nr][nc] == INTEL) {
    intel++;
    grid[nr][nc] = EMPTY;   // remove collected intel
    printf("Collected Intel! Intel now: %d\n", intel);
}
else if (grid[nr][nc] == LIFE) {
    lives++;
    grid[nr][nc] = EMPTY;   // remove collected life pack
    printf("Collected Life! Lives now: %d\n", lives);
}

  // If next cell is extraction point
if (grid[nr][nc] == EXTRACT) {
    if (intel == 3) {
        printf("YOU WIN! Extracted with all intel.\n");

        logState(logfp, grid, n, &p, &p2, move, "Reached extraction (win)");
    } else {
        printf("YOU LOST! Reached extraction without all intel.\n");

        logState(logfp, grid, n, &p, &p2, move, "Reached extraction (loss)");
    }
    break; // end game
}

   // move player
    p.r = nr;
    p.c = nc;
    logState(logfp, grid, n, &p, &p2, move, "Valid move");
}

 fclose(logfp);
freeGrid(grid, n);
return 0;
}
