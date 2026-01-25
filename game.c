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
#define PLAYER3 '$'
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
 

void logState(FILE *fp, char **grid, int n, Player *p1, Player *p2, Player *p3, int mode, char move, const char *note) {
    fprintf(fp, "Move: %c | %s\n", move, note);
    fprintf(fp, "P1(%c) Lives: %d | Intel: %d | Pos: (%d,%d) | Active: %d\n", p1->symbol, p1->lives, p1->intel, p1->r, p1->c, p1->active);
    fprintf(fp, "P2(%c) Lives: %d | Intel: %d | Pos: (%d,%d) | Active: %d\n", p2->symbol, p2->lives, p2->intel, p2->r, p2->c, p2->active);
    if (mode == 3) {
        fprintf(fp, "P3(%c) Lives: %d | Intel: %d | Pos: (%d,%d) | Active: %d\n", p3->symbol, p3->lives, p3->intel, p3->r, p3->c, p3->active);
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == p1->r && j == p1->c && p1->active) fputc(p1->symbol, fp);
            else if (p2->active && i == p2->r && j == p2->c) fputc(p2->symbol, fp);
            else if (mode == 3 && p3->active && i == p3->r && j == p3->c) fputc(p3->symbol, fp);
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

char getComputerMove(char **grid, int n, Player *current, Player *pA, Player *pB) {
    const char moves[4] = {'W','A','S','D'};

    for (int tries = 0; tries < 30; tries++) {
        char mv = moves[rand() % 4];
        int dr = 0, dc = 0;
        if (mv == 'W') dr = -1;
        else if (mv == 'S') dr = 1;
        else if (mv == 'A') dc = -1;
        else if (mv == 'D') dc = 1;

        int nr = current->r + dr;
        int nc = current->c + dc;

        if (nr < 0 || nr >= n || nc < 0 || nc >= n) continue;
        if (grid[nr][nc] == WALL) continue;
        if (pA && pA->active && nr == pA->r && nc == pA->c) continue;
        if (pB && pB->active && nr == pB->r && nc == pB->c) continue;

        return mv;
    }

    return 'W';
}





int main(){

    srand(time(NULL));

   printf("===Welcome to SpyNet-The Codebreaker Protocol====\n");

    int n;
    printf("\nEnter the grid size (5 <= N <= 15):  ");
    scanf("%d", &n);

    int mode;
    printf("\nSelect mode: 1) Single  2) Two-player  3) Three-player : ");
    scanf("%d", &mode);
    if (mode != 1 && mode != 2 && mode != 3) mode = 1;

    int p2IsHuman = 1;
    if (mode == 2) {
        printf("Player 2 type: 1) Human  2) Computer : ");
        scanf("%d", &p2IsHuman);
        if (p2IsHuman != 1 && p2IsHuman != 2) p2IsHuman = 1;
    }

    int p3IsHuman = 1;
    if (mode == 3) {
        printf("Player 3 type: 1) Human  2) Computer : ");
        scanf("%d", &p3IsHuman);
        if (p3IsHuman != 1 && p3IsHuman != 2) p3IsHuman = 1;
    }


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
    p2.active = (mode >= 2) ? 1 : 0;
    p2.isHuman = (p2IsHuman == 1) ? 1 : 0;
    p2.r = -1;
    p2.c = -1;

    Player p3;
    p3.symbol = PLAYER3;
    p3.lives = 3;
    p3.intel = 0;
    p3.active = (mode == 3) ? 1 : 0;
    p3.isHuman = (p3IsHuman == 1) ? 1 : 0;
    p3.r = -1;
    p3.c = -1;

   

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

    if (mode >= 2) {
        char saved1 = grid[p.r][p.c];
        grid[p.r][p.c] = WALL;      // temporarily block P1 cell
        placePlayer(grid, n, &p2.r, &p2.c);
        grid[p.r][p.c] = saved1;    // restore cell
    }

    if (mode == 3) {
        // Temporarily block P1 and P2 cells so P3 can't spawn on them
        char saved1 = grid[p.r][p.c];
        char saved2 = grid[p2.r][p2.c];
        grid[p.r][p.c] = WALL;
        grid[p2.r][p2.c] = WALL;
        placePlayer(grid, n, &p3.r, &p3.c);
        grid[p.r][p.c] = saved1;
        grid[p2.r][p2.c] = saved2;
    }

   // Turn system helpers

   logState(logfp, grid, n, &p, &p2, &p3, mode, '-', "Initial state");
    
    int currentIndex = 0; // 0 => p (@), 1 => p2 (&), 2 => p3 ($)
    Player *players[3] = { &p, &p2, &p3 };

    while (1) {
    Player *current = players[currentIndex];

    // Skip inactive players automatically (mode 2: 0..1, mode 3: 0..2)
    int maxPlayers = (mode == 3) ? 3 : 2;
    int guard = 0;
    while (!current->active && guard < maxPlayers) {
        currentIndex = (currentIndex + 1) % maxPlayers;
        current = players[currentIndex];
        guard++;
    }

    printf("\nTurn: Player %c\n", current->symbol);
    printf("P1(%c) Lives: %d | Intel: %d | Active: %d\n", p.symbol, p.lives, p.intel, p.active);
    printf("P2(%c) Lives: %d | Intel: %d | Active: %d\n", p2.symbol, p2.lives, p2.intel, p2.active);
    if (mode == 3) {
        printf("P3(%c) Lives: %d | Intel: %d | Active: %d\n", p3.symbol, p3.lives, p3.intel, p3.active);
    }

    for (int j = 0; j < n; j++) printf(" __");
    printf("\n");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (p.active && i == p.r && j == p.c) printf("|%c ", p.symbol);
            else if (p2.active && i == p2.r && j == p2.c) printf("|%c ", p2.symbol);
            else if (mode == 3 && p3.active && i == p3.r && j == p3.c) printf("|%c ", p3.symbol);
            else printf("|%c ", grid[i][j]);
        }
        printf("|\n");

        for (int j = 0; j < n; j++) printf("|__");
        printf("|\n");
    }

    char move;
    if (current->isHuman) {
        printf("\nMove (W/A/S/D) or Q to quit: ");
        scanf(" %c", &move);
        move = toupper((unsigned char)move);
    } else {
        // Pass other active players (up to two) to avoid collisions
        Player *o1 = NULL;
        Player *o2 = NULL;
        int idx = 0;
        for (int k = 0; k < ((mode == 3) ? 3 : 2); k++) {
            if (k == currentIndex) continue;
            if (idx == 0) o1 = players[k];
            else o2 = players[k];
            idx++;
        }
        move = getComputerMove(grid, n, current, o1, o2);
        printf("\nComputer (Player %c) chose move: %c\n", current->symbol, move);
    }

    if (move == 'Q') {
        if (!current->isHuman) {
            // Computer should not quit; treat as invalid key
            loseLife(current);
            logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Computer attempted quit (penalized)");
            goto ADVANCE_TURN;
        }
        printf("Player %c quit and became inactive.\n", current->symbol);
        current->active = 0;
        logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Quit (current player inactive)");
        if (mode == 1) {
            break;
        }
    }

    if (mode == 2 || mode == 3) {
        int activeCount = p.active + p2.active + (mode == 3 ? p3.active : 0);
        if (activeCount == 1) {
            Player *winner = p.active ? &p : (p2.active ? &p2 : &p3);
            printf("Player %c wins (only active player remaining).\n", winner->symbol);
            logState(logfp, grid, n, &p, &p2, &p3, mode, '-', "Auto-win: only one active player");
            break;
        }
    }

    // If current player became inactive (e.g., quit), advance turn
    if (!current->active) {
        if (mode == 1) {
            break;
        }
        continue;
    }

    int dr, dc;
    if (!getMoveDelta(move, &dr, &dc)) {
        loseLife(current);
        logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Invalid key");
        if (mode == 1 && !current->active) {
            break;
        }
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
        logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Outside grid");
        if (mode == 1 && !current->active) {
            break;
        }
        goto ADVANCE_TURN;
    }

    if (grid[nr][nc] == WALL) {
        loseLife(current);
        logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Hit Wall");
        if (mode == 1 && !current->active) {
            break;
        }
        goto ADVANCE_TURN;
    }

    // cannot move onto another active player
    int maxPlayers2 = (mode == 3) ? 3 : 2;
    for (int k = 0; k < maxPlayers2; k++) {
        if (k == currentIndex) continue;
        if (players[k]->active && nr == players[k]->r && nc == players[k]->c) {
            loseLife(current);
            logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Tried to move onto another player");
            if (mode == 1 && !current->active) {
                break;
            }
            goto ADVANCE_TURN;
        }
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
        logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Reached extraction (win)");
        break;
    } else {
        printf("Player %c became inactive (reached extraction without all intel).\n", current->symbol);
        current->active = 0;
        logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Reached extraction (inactive)");
        // do not move onto X; just end this turn
        if (mode == 1) {
            break;
        }
        goto ADVANCE_TURN;
    }
}

    // move current player
    current->r = nr;
    current->c = nc;
    logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Valid move");

ADVANCE_TURN:
    if (mode == 2 || mode == 3) {
        int activeCount = p.active + p2.active + (mode == 3 ? p3.active : 0);
        if (activeCount == 1) {
            Player *winner = p.active ? &p : (p2.active ? &p2 : &p3);
            printf("Player %c wins (only active player remaining).\n", winner->symbol);
            logState(logfp, grid, n, &p, &p2, &p3, mode, '-', "Auto-win: only one active player");
            break;
        }
    }

    // Advance to next active player
    if (mode == 2) {
        currentIndex = (currentIndex + 1) % 2;
        if (!players[currentIndex]->active) currentIndex = (currentIndex + 1) % 2;
    } else if (mode == 3) {
        currentIndex = (currentIndex + 1) % 3;
        // skip inactive players (at most 2 skips needed)
        for (int tries = 0; tries < 2; tries++) {
            if (players[currentIndex]->active) break;
            currentIndex = (currentIndex + 1) % 3;
        }
    }
}

 fclose(logfp);
freeGrid(grid, n);
return 0;
}
