#include <stdio.h>

   int main() {
    printf("=== Welcome to SpyNet – The Codebreaker Protocol ===\n");

    int n;
    printf("Enter the grid size (5 <= N <= 15): ");
    scanf("%d", &n);

    
    for (int j = 0; j < n; j++) printf(" __");
    printf("\n");

    
    for (int i = 0; i < n; i++) {
        
        for (int j = 0; j < n; j++) printf("|  ");
        printf("|\n");

        
        for (int j = 0; j < n; j++) printf("|__");
        printf("|\n");
    }
    
    return 0;
}
