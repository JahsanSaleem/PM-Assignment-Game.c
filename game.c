
#include <stdio.h>
#include <stdlib.h>
#include <time.h>         // time() is used to seed rand() so the map changes each run
#include <ctype.h>        // toupper() is used to normalize move input
#include <string.h>     // kept (even if unused) for possible extensions


#define MIN_N 5         // minimum allowed grid size
#define MAX_N 15        // maximum allowed grid size

#define EMPTY '.'       // empty cell
#define EXTRACT 'X'     // extraction point
#define WALL '#'        // wall(blocks movement)
#define INTEL 'I'       // intel item (must collect 3 to win)
#define LIFE 'L'        // life pack (adds +1 life)

#define PLAYER '@'      // player 1 symbol
#define PLAYER2 '&'     // player 2 symbol
#define PLAYER3 '$'     // player 3 symbol

// Player struct stores each player's current state.
typedef struct {
    char symbol;         
    int r, c;
    int lives;        
    int intel;
    int active;
    int isHuman;
} Player;
 
// Logs the current game state to a text file after each important event.
void logState(FILE *fp, char **grid, int n,
              Player *p1, Player *p2, Player *p3,
              int mode, char move, const char *note) {

    fprintf(fp, "\n=============================\n");
    fprintf(fp, "Move: %c | %s\n", move, note);

    fprintf(fp, "P1(%c) | Lives=%d | Intel=%d | Pos=(%d,%d) | Active=%d\n",
            p1->symbol, p1->lives, p1->intel, p1->r, p1->c, p1->active);
    if (mode >= 2) {
        fprintf(fp, "P2(%c) | Lives=%d | Intel=%d | Pos=(%d,%d) | Active=%d\n",
                p2->symbol, p2->lives, p2->intel, p2->r, p2->c, p2->active);
    }
    if (mode == 3) {
        fprintf(fp, "P3(%c) | Lives=%d | Intel=%d | Pos=(%d,%d) | Active=%d\n",
                p3->symbol, p3->lives, p3->intel, p3->r, p3->c, p3->active);
    }

    fprintf(fp, "Grid:\n");
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++) {
            // Overlay player symbols on top of the map characters for logging
            if (p1->active && r == p1->r && c == p1->c) {
                fputc(p1->symbol, fp);
            } else if (mode >= 2 && p2->active && r == p2->r && c == p2->c) {
                fputc(p2->symbol, fp);
            } else if (mode == 3 && p3->active && r == p3->r && c == p3->c) {
                fputc(p3->symbol, fp);
            } else {
                fputc(grid[r][c], fp);
            }
        }
        fputc('\n', fp);
    }

    fflush(fp);
}


char **allocGrid(int n) {
                                                       // Allocate an N x N grid dynamically (rows allocated separately)
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


                                     //----------- Random map generation --------------


void placeExtraction(char **g, int n) {
    int r, c;
                               // Randomly place one extraction point (X) on an empty cell
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
                               // Choose a random empty starting position.
                               // We return it via *pr and *pc (player symbol is drawn as an overlay, not stored in grid[][]).
    } while (g[r][c] != EMPTY);

    *pr = r;
    *pc = c;
}

void placeWalls(char **g, int n) {
    int wallCount = (n * n) / 7;                           // ~15% of the grid
    if (wallCount < 5) wallCount = 5;
    // Randomly place walls (#) as obstacles; count is ~15% of grid (minimum 5)

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
        // Place exactly 3 intel items (I) on empty cells
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
        // Place exactly 2 life packs (L) on empty cells
        int r = rand() % n;
        int c = rand() % n;

        if (g[r][c] == EMPTY) {
            g[r][c] = LIFE;
            placed++;
        }
    }
}

// Converts a move key (W/A/S/D) into row/col deltas (dr, dc). 
int getMoveDelta(char move, int *dr, int *dc) {
    *dr = 0;
    *dc = 0;

    if (move == 'W') { *dr = -1; return 1; }
    if (move == 'S') { *dr =  1; return 1; }
    if (move == 'A') { *dc = -1; return 1; }
    if (move == 'D') { *dc =  1; return 1; }

    return 0;
}

// Decrease life by 1 for an invalid move; if lives reach 0 the player becomes inactive.
void loseLife(Player *p) {
    p->lives--;
    printf("Invalid move! Lives -1 (Lives now: %d)\n", p->lives);
    if (p->lives <= 0) {
        p->active = 0;
        printf("Player %c became inactive (lives reached 0).\n", p->symbol);
    }
}

// Simple computer AI: tries random moves until it finds one that is valid (not wall/outside/onto another player).
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


// Main: input -> setup -> game loop -> cleanup-------------------------------------------------------------------------------

int main(){
    srand(time(NULL));                                               // seed RNG for random placements

   printf("===Welcome to SpyNet-The Codebreaker Protocol====\n");

    int n;
    printf("\nEnter the grid size (5 <= N <= 15):  ");
    scanf("%d", &n);

    if (n < MIN_N || n > MAX_N) {                                     // validate N (assignment limits + avoid crash)
        printf("Invalid N. Must be between %d and %d.\n", MIN_N, MAX_N);
        return 1;
    }

    int mode;
    printf("\nSelect mode: 1) Single  2) Two-player  3) Three-player : ");   //asking user for player mode 
    scanf("%d", &mode);                                                      //single or dual or three 
    if (mode != 1 && mode != 2 && mode != 3) mode = 1;

    int p2IsHuman = 1;
    if (mode == 2 || mode == 3) {
        printf("Player 2 type: 1) Human  2) Computer : ");         //asking user for Human or Computer for mode 2
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
   p.symbol = PLAYER;              //declaring player 1 status through struct 
   p.lives = 3;
   p.intel = 0;
   p.active = 1;
   p.isHuman = 1;

    Player p2;
    p2.symbol = PLAYER2;
    p2.lives = 3;
    p2.intel = 0;                               //declaring player 2 status through struct 

    if (mode >= 2) p2.active = 1;
    else p2.active = 0;

    if (p2IsHuman == 1) p2.isHuman = 1;
    else p2.isHuman = 0;

    p2.r = -1;
    p2.c = -1;

    Player p3;
    p3.symbol = PLAYER3;                          //declaring player three status through struct 
    p3.lives = 3;
    p3.intel = 0;

    if (mode == 3) p3.active = 1;
    else p3.active = 0;

    if (p3IsHuman == 1) p3.isHuman = 1;  // 1=human
    else p3.isHuman = 0;                 // 0=computer

    p3.r = -1;
    p3.c = -1;

    char **grid = allocGrid(n);                          

    if (grid == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    FILE *logfp = fopen("spynet_log1.txt", "w");              // log game state after each move
    if (logfp == NULL) {
        printf("Could not open log file.\n");
        freeGrid(grid, n);
        return 1;
    }
    // Setup order matters: start empty, then add items, then choose player start positions.
    fillGrid(grid, n, EMPTY);
    placeWalls(grid, n);
    placeIntel(grid, n);
    placeLives(grid, n);
    placeExtraction(grid, n);
    placePlayer(grid, n, &p.r, &p.c);

    if (mode >= 2) {
        char saved1 = grid[p.r][p.c];                /* Place Player 2 on an EMPTY cell without overlapping Player 1.
                                              We temporarily mark P1's position as a WALL so placePlayer() won't choose it,
                                                     then restore the original cell value */
        grid[p.r][p.c] = WALL;
        placePlayer(grid, n, &p2.r, &p2.c);
        grid[p.r][p.c] = saved1;
    }

    if (mode == 3) {
        char saved1 = grid[p.r][p.c];               // Place Player 3 on an EMPTY cell without overlapping Player 1 or Player 2.
                                                 // We temporarily block both existing player positions, choose a free cell for P3,
                                                  // then restore the original grid values.
        char saved2 = grid[p2.r][p2.c];
        grid[p.r][p.c] = WALL;
        grid[p2.r][p2.c] = WALL;
        placePlayer(grid, n, &p3.r, &p3.c);
        grid[p.r][p.c] = saved1;
        grid[p2.r][p2.c] = saved2;
    }

    logState(logfp, grid, n, &p, &p2, &p3, mode, '-', "Initial state");
    
    int currentIndex = 0;
    Player *players[3] = { &p, &p2, &p3 };                                 // turn order list

    while (1) {
        Player *current = players[currentIndex];

        // Select the player whose turn it is using currentIndex.
        // If that player is inactive (dead or quit), skip them and move to the next one.
         // 'guard' prevents an infinite loop in case all players are inactive.

        int maxPlayers;
            if (mode == 3) maxPlayers = 3;
                else if (mode == 2) maxPlayers = 2;
                else maxPlayers = 1;
        int guard = 0;
        // Skip inactive players so they do not get turns
        while (!current->active && guard < maxPlayers) {
            currentIndex = (currentIndex + 1) % maxPlayers;
            current = players[currentIndex];
            guard++;
        }

        printf("\nTurn: Player %c\n", current->symbol);
        printf("P1(%c) Lives: %d | Intel: %d | Active: %d\n", p.symbol, p.lives, p.intel, p.active);
        if (mode >= 2) printf("P2(%c) Lives: %d | Intel: %d | Active: %d\n", p2.symbol, p2.lives, p2.intel, p2.active);
        if (mode == 3) printf("P3(%c) Lives: %d | Intel: %d | Active: %d\n", p3.symbol, p3.lives, p3.intel, p3.active);

        // Draw the board with player symbols overlaid on top of grid[][]
        for (int j = 0; j < n; j++) printf(" __");
        printf("\n");

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (p.active && i == p.r && j == p.c) printf("|%c ", p.symbol);
                else if (mode >= 2 && p2.active && i == p2.r && j == p2.c) printf("|%c ", p2.symbol);
                else if (mode == 3 && p3.active && i == p3.r && j == p3.c) printf("|%c ", p3.symbol);
                else printf("|%c ", grid[i][j]);
            }
            printf("|\n");

            for (int j = 0; j < n; j++) printf("|__");
            printf("|\n");
        }

        char move;

if (current->isHuman) {
    // Human player: read a move from the keyboard and normalize to uppercase
    printf("\nMove (W/A/S/D) or Q to quit: ");
    scanf(" %c", &move);                       // leading space skips newline/whitespace
    move = toupper((unsigned char)move);

} else {
    // Computer player: choose a move automatically.
    // We first collect pointers to the OTHER players (opponents) so the AI can avoid collisions.
    Player *o1 = NULL;                         // first opponent (if exists)
    Player *o2 = NULL;                         // second opponent (only in 3-player mode)
    int idx = 0;

    for (int k = 0; k < maxPlayers; k++) {
        if (k == currentIndex) continue;       // skip the current player (don't compare with self)

        if (idx == 0) o1 = players[k];         // store first other player
        else o2 = players[k];                  // store second other player (if any)
        idx++;
    }

    // AI picks a random valid move that does not hit walls, go outside, or move onto an active opponent.
    move = getComputerMove(grid, n, current, o1, o2);
    printf("\nComputer (Player %c) chose move: %c\n", current->symbol, move);
}

        int endTurn = 0;   // set to 1 when this player's turn should stop
        int endGame = 0;   // set to 1 when the whole game should stop

        // Process one turn. Using do/while(0) lets us "break" 
        do {
            // Handle quitting: human can quit (becomes inactive). Computer is penalized if it "quits".
            if (move == 'Q') {
                if (!current->isHuman) {
                    loseLife(current);
                    logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Computer attempted quit (penalized)");
                    endTurn = 1;
                    break;
                }

                printf("Player %c quit and became inactive.\n", current->symbol);
                current->active = 0;
                logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Quit (current player inactive)");

                if (mode == 1) {
                    endGame = 1;
                }
                endTurn = 1;
                break;
            }

            // If only one active player remains in multiplayer, they win immediately.
            if (mode >= 2) {
                int activeCount = p.active;
                if (mode >= 2) activeCount += p2.active;
                if (mode == 3) activeCount += p3.active;

                if (activeCount == 1) {
                    Player *winner = NULL;
                    if (p.active) winner = &p;
                    else if (mode >= 2 && p2.active) winner = &p2;
                    else if (mode == 3 && p3.active) winner = &p3;

                    if (winner) {
                        printf("Player %c wins (only active player remaining).\n", winner->symbol);
                        logState(logfp, grid, n, &p, &p2, &p3, mode, '-', "Auto-win: only one active player");
                    }
                    endGame = 1;
                    break;
                }
            }

            // If current player became inactive earlier, skip their turn (single-player ends).
            if (!current->active) {
                if (mode == 1) endGame = 1;
                endTurn = 1;
                break;
            }

            // Convert the input move into direction changes (dr, dc).
            // If the key is not one of W/A/S/D, it's an invalid move -> lose 1 life, log it, and end this turn.

              int dr, dc;
             if (!getMoveDelta(move, &dr, &dc)) {
               loseLife(current);
             logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Invalid key");

                // In single-player, if the player loses their last life, the game ends.
             if (mode == 1 && !current->active) endGame = 1;

                    endTurn = 1;   // stop processing this turn after the penalty
                    break;         // exit the do{...}while(0) turn-processing block
            }

            int nr = current->r + dr;  // Compute the new row and column the player wants to move to
            int nc = current->c + dc;   // (current position + direction offsets)

            if (nr < 0 || nr >= n || nc < 0 || nc >= n) {
                loseLife(current);
                logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Outside grid");
                if (mode == 1 && !current->active) endGame = 1;
                endTurn = 1;
                break;
            }

            // Wall collision: cannot move onto a wall cell
            if (grid[nr][nc] == WALL) {
                loseLife(current);
                logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Hit Wall");
                if (mode == 1 && !current->active) endGame = 1;
                endTurn = 1;
                break;
            }

            int maxPlayers2;
                if (mode == 3) maxPlayers2 = 3;
                    else if (mode == 2) maxPlayers2 = 2;
                else maxPlayers2 = 1;
                
            // Prevent moving onto another active player's cell (collision)
            for (int k = 0; k < maxPlayers2; k++) {
                if (k == currentIndex) continue;
                if (players[k]->active && nr == players[k]->r && nc == players[k]->c) {
                    loseLife(current);
                    logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Tried to move onto another player");
                    if (mode == 1 && !current->active) endGame = 1;
                    endTurn = 1;
                    break;
                }
            }
            if (endTurn || endGame) break;

            // Item collection: stepping onto I/L changes stats and removes the item from the grid
            if (grid[nr][nc] == INTEL) {
                current->intel++;
                grid[nr][nc] = EMPTY;
                printf("Player %c collected Intel. Intel now: %d\n", current->symbol, current->intel);
            } else if (grid[nr][nc] == LIFE) {
                current->lives++;
                grid[nr][nc] = EMPTY;
                printf("Player %c collected Life. Lives now: %d\n", current->symbol, current->lives);
            }

            // Extraction rule: win only if intel==3, otherwise player becomes inactive
            if (grid[nr][nc] == EXTRACT) {
                if (current->intel == 3) {
                    printf("Player %c wins by extracting with all intel.\n", current->symbol);
                    logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Reached extraction (win)");
                    endGame = 1;
                    break;
                } else {
                    printf("Player %c became inactive (reached extraction without all intel).\n", current->symbol);
                    current->active = 0;
                    logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Reached extraction (inactive)");
                    if (mode == 1) endGame = 1;
                    endTurn = 1;
                    break;
                }
            }

            // Apply valid move
            current->r = nr;
            current->c = nc;
            logState(logfp, grid, n, &p, &p2, &p3, mode, move, "Valid move");
            endTurn = 1;

        } while (0);

        if (endGame) {
            break;
        }

        // End-of-turn: check auto-win (multiplayer) and rotate to the next active player.
        if (mode >= 2) {
            int activeCount = p.active;
            if (mode >= 2) activeCount += p2.active;
            if (mode == 3) activeCount += p3.active;

            if (activeCount == 1) {
                Player *winner = NULL;
                if (p.active) winner = &p;
                else if (mode >= 2 && p2.active) winner = &p2;
                else if (mode == 3 && p3.active) winner = &p3;

                if (winner) {
                    printf("Player %c wins (only active player remaining).\n", winner->symbol);
                    logState(logfp, grid, n, &p, &p2, &p3, mode, '-', "Auto-win: only one active player");
                }
                break;
            }
        }

       // Rotate to the next player's turn after finishing the current turn.
      // In mode 2: toggle between player index 0 and 1, skipping an inactive player.
       // In mode 3: move to the next index (0->1->2->0) and skip inactive players.
    // 'tries < 2' is enough because in 3-player mode there are only two other players to check.
      if (mode == 2) {
    currentIndex = (currentIndex + 1) % 2;                 // switch to the other player (0<->1)
    if (!players[currentIndex]->active)                    // if they are inactive, skip back to the active one
        currentIndex = (currentIndex + 1) % 2;

}      else if (mode == 3) {
    currentIndex = (currentIndex + 1) % 3;                 // move to next player (cycle 0->1->2->0)

    // Skip inactive players: check at most two times (only two other players exist)
    for (int tries = 0; tries < 2; tries++) {
        if (players[currentIndex]->active) break;          // found an active player
        currentIndex = (currentIndex + 1) % 3;             // otherwise keep rotating
    }
}
    }

    fclose(logfp);
    freeGrid(grid, n);
    return 0;
}
