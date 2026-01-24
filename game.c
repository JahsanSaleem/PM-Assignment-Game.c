#include <stdio.h>
#include <stdlib.h>
#include <time.h>



#define MIN_N 5
#define MAX_N 15
#define EMPTY '.'
#define EXTRACT 'X'
#define PLAYER '@'
#define WALL '#'


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

 

int main(){

    srand(time(NULL));

    int n;
    printf("Enter the grid size (5 <= N <= 15):  ");
    scanf("%d", &n);


    int pr, pc;

   

   char **grid = allocGrid(n);

   fillGrid(grid, n, EMPTY);
   placeWalls(grid, n);
   placeExtraction(grid, n);
   placePlayer(grid, n, &pr, &pc);
    
    for (int j = 0; j < n; j++) printf(" __");
    printf("\n");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
    if (i == pr && j == pc) 
    
         printf("|%c ", PLAYER);
    
    else printf("|%c ", grid[i][j]);
}
       
       printf("|\n");

        for (int j = 0; j < n; j++) printf("|__");
        printf("|\n");
    }

    
             
       freeGrid(grid,n);
 return 0;
}