#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define SIZE 36

#define RESET  "\x1B[0m"
#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define YEL  "\x1B[33m"
#define BLUE  "\x1B[34m"
#define MAG  "\x1B[35m"
#define CYN  "\x1B[36m"
#define WHT  "\x1B[37m"

#define YANG 1
#define KOHM -1

int count_neighbors(int state[SIZE][SIZE], int i, int j);
void display(int state[SIZE][SIZE]);

int main(int argc, char *argv[]) {
    int state[SIZE][SIZE] = {0};
    int next[SIZE][SIZE] = {0};

    int nseed = 0;
    int n = SIZE;
    int num_neighbors, manual, i, j;


    printf("ENTER 1 FOR MANUAL SETUP, 0 FOR RANDOM:");
    scanf("%d", &manual);

    if (manual) {
        while (true) {
            printf("ENTER COORDINATES OF LIVE CELLS AS (I,J); WRITE 0,0 WHEN DONE\n");
            scanf("%d,%d", &i, &j);
            if (i == 0 && j == 0) break;
            state[i][j] = 1;
            display(state);
        }
    } else {
        
        printf("ENTER A FIVE DIGIT INTEGER TO CHANGE SEED\n");
        scanf("%d", &nseed);
        srand(nseed);

        float rn1;
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                rn1 = (double)rand() / RAND_MAX;
                if (rn1 < 0.75) {
                    state[i][j] = 0;
                } else {
                    state[i][j] = 1;
                }
            }
        }
    }

    display(state);
    printf("ADJUST SCREEN TO BE 36 LINES LONG.");

    // Main game loop
    while (true) {
        // Loop over all cells
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                num_neighbors = count_neighbors(state, i, j);
                // Living cells
                if (state[i][j] == 1) {
                    if (num_neighbors <= 1) {
                        next[i][j] = 0;
                    } else if (num_neighbors >= 4) {
                        next[i][j] = 0;
                    } else {
                        next[i][j] = 1;
                    }
                // Dead cells
                } else {
                    if (num_neighbors == 3) next[i][j] = 1;
                }
            }
        }

        // Set current game state to next game state
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                state[i][j] = next[i][j];
            }
        }  

        display(state);

    }

}

// Print current game state to screen
void display(int state[SIZE][SIZE]) {
    int i, j;
    int n = SIZE;
    printf("\n\n\n\n\n\n\n\n\n");
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < n; j++) {
            if (state[i][j] == 1) {
                //printf(" @ ")
                printf(CYN "\u2588\u2588");
            } else {
                printf("  ");
            }
        }
        printf("\n" RESET);
    }
    usleep(1000 * 100); // 100 milliseconds -> ~15 fps animation
}

int count_neighbors(int state[SIZE][SIZE], int i, int j) {

    int n = SIZE;
    // Modulo arithmetic for toroidal board
    int num_neighbors = state[(i-1+n)%n][(j-1+n)%n] + state[(i+n)%n][(j-1+n)%n] + state[(i+1)%n][(j-1+n)%n] +
                     state[(i-1+n)%n][(j+n)%n]                              + state[(i+1)%n][(j+n)%n] +
                     state[(i-1+n)%n][(j+1)%n] + state[(i+n)%n][(j+1)%n] + state[(i+1)%n][(j+1)%n];
    return num_neighbors;
}