#include <stdio.h>

#define MIN_N 5
#define MAX_N 15
#define EMPTY '.'


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
 }
    return g;

    void freeGrid(char **g, int n) {
    for (int i = 0; i < n; i++) free(g[i]);
    free(g);
    }

    void fillGrid(char **g, int n, char ch) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            g[i][j] = ch;
    }

  int main(){

    int n;
    printf("Enter the grid size (5 <= N <= 15):  ");
    scanf("%d", &n);

    char **grid = allocGrid(n);


     fillGrid(grid, n, EMPTY);

    
    for (int j = 0; j < n; j++) printf(" __");
    printf("\n");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) printf("|%c ", grid[i][j]);
        printf("|\n");

        for (int j = 0; j < n; j++) printf("|__");
        printf("|\n");
    }

    
             
       freeGrid(grid,n);
 return 0;
}