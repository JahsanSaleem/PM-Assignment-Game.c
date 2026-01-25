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

void loseLife(Player *p) {
    p->lives--;
    printf("Invalid move! Lives -1 (Lives now: %d)\n", p->lives);
    if (p->lives <= 0) {
        p->active = 0;
        printf("Player %c became inactive (lives reached 0).\n", p->symbol);
    }
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

   // Turn system helpers

   logState(logfp, grid, n, &p, &p2, '-', "Initial state");
    
    int currentIndex = 0; // 0 => p (@), 1 => p2 (&)
    Player *current = &p;
    Player *other = &p2;

    while (1) {

    printf("\nTurn: Player %c\n", current->symbol);
    printf("P1(%c) Lives: %d | Intel: %d | Active: %d\n", p.symbol, p.lives, p.intel, p.active);
    printf("P2(%c) Lives: %d | Intel: %d | Active: %d\n", p2.symbol, p2.lives, p2.intel, p2.active);

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
        printf("Player %c quit and became inactive.\n", current->symbol);
        current->active = 0;
        logState(logfp, grid, n, &p, &p2, move, "Quit (current player inactive)");
    }

    // Auto-win if only one active player remains
    if (p.active + p2.active == 1) {
        Player *winner = p.active ? &p : &p2;
        printf("Player %c wins (only active player remaining).\n", winner->symbol);
        logState(logfp, grid, n, &p, &p2, '-', "Auto-win: only one active player");
        break;
    }

    // If current player became inactive (e.g., quit), advance turn
    if (!current->active) {
        currentIndex = 1 - currentIndex;
        current = (currentIndex == 0) ? &p : &p2;
        other   = (currentIndex == 0) ? &p2 : &p;
        continue;
    }

    int dr, dc;
    if (!getMoveDelta(move, &dr, &dc)) {
        loseLife(current);
        logState(logfp, grid, n, &p, &p2, move, "Invalid key");
        if (!current->active) {
            // inactive due to lives reaching 0
        }
        // advance turn at end of loop
        goto ADVANCE_TURN;
    }

   int nr = current->r + dr;
   int nc = current->c + dc;

    if (nr < 0 || nr >= n || nc < 0 || nc >= n) {
        loseLife(current);
        logState(logfp, grid, n, &p, &p2, move, "Outside grid");
        goto ADVANCE_TURN;
    }

    if (grid[nr][nc] == WALL) {
        loseLife(current);
        logState(logfp, grid, n, &p, &p2, move, "Hit Wall");
        goto ADVANCE_TURN;
    }

    if (nr == other->r && nc == other->c && other->active) {
        loseLife(current);
        logState(logfp, grid, n, &p, &p2, move, "Tried to move onto other player");
        goto ADVANCE_TURN;
    }

    // valid move: check what is on the target cell
if (grid[nr][nc] == INTEL) {
    current->intel++;
    grid[nr][nc] = EMPTY;
    printf("Player %c collected Intel. Intel now: %d\n", current->symbol, current->intel);
}
else if (grid[nr][nc] == LIFE) {
    current->lives++;
    grid[nr][nc] = EMPTY;
    printf("Player %c collected Life. Lives now: %d\n", current->symbol, current->lives);
}

  // If next cell is extraction point
if (grid[nr][nc] == EXTRACT) {
    if (current->intel == 3) {
        printf("Player %c wins by extracting with all intel.\n", current->symbol);
        logState(logfp, grid, n, &p, &p2, move, "Reached extraction (win)");
        break;
    } else {
        printf("Player %c became inactive (reached extraction without all intel).\n", current->symbol);
        current->active = 0;
        logState(logfp, grid, n, &p, &p2, move, "Reached extraction (inactive)");
        // do not move onto X; just end this turn
        goto ADVANCE_TURN;
    }
}

    // move current player
    current->r = nr;
    current->c = nc;
    logState(logfp, grid, n, &p, &p2, move, "Valid move");

ADVANCE_TURN:
    // Auto-win check after turn resolution
    if (p.active + p2.active == 1) {
        Player *winner = p.active ? &p : &p2;
        printf("Player %c wins (only active player remaining).\n", winner->symbol);
        logState(logfp, grid, n, &p, &p2, '-', "Auto-win: only one active player");
        break;
    }

    // Advance to next active player
    currentIndex = 1 - currentIndex;
    current = (currentIndex == 0) ? &p : &p2;
    other   = (currentIndex == 0) ? &p2 : &p;
    if (!current->active) {
        // skip inactive player
        currentIndex = 1 - currentIndex;
        current = (currentIndex == 0) ? &p : &p2;
        other   = (currentIndex == 0) ? &p2 : &p;
    }
}

 fclose(logfp);
freeGrid(grid, n);
return 0;
}
