#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#define SIZE 36
#define HT_SIZE 16384

#define YANG 1
#define KOHM -1


uint64_t rand_U64();
int count_neighbors(int state[SIZE][SIZE], int i, int j);
int play_life(int state[SIZE][SIZE], uint64_t hash_keys[SIZE][SIZE], int *live_cells);

int main(int argc, char *argv[]) {

    bool cycle_detected = false;
    int n = SIZE;
    int i, j, num_generations;
    int nseed = 65537; 
    srand(nseed);
    int state[SIZE][SIZE] = {0};

    // output CSV file
    FILE *foutput = fopen("stats10.csv", "w");

    // Used for storing hash keys for each cell
    uint64_t hash_keys[SIZE][SIZE];
    // Initialize hash_keys to random values
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            hash_keys[i][j] = rand_U64(); 
        }
    }

    printf("Running longevity test\n");

    fprintf(foutput, "fill_fraction,generations,living_cells");

    int iters_per_frac = 10;
    int total_generations = 0;
    double average_generations = 0.0;
    double average_live_cells = 0.0;

    // Test longevity from 1% to 99%
    for (float fill_fraction = 0.01; fill_fraction < 1; fill_fraction += 0.01) {
        printf("Longevity test for %d percent filling\n", (int) (fill_fraction * 100));

        int gens = 0;
        int total_generations = 0;
        int live_cells = 0;
        int total_live_cells = 0;

        // Do many iterations per fill fraction 
        for (int k = 0; k < iters_per_frac; k++) {
            printf("\rIteration %d of %d", k+1, iters_per_frac);
            fflush(stdout);
            
            float rn1;
            // Create a new random game board
            for (i = 0; i < n; i++) {
                for (j = 0; j < n; j++) {
                    rn1 = (double)rand() / RAND_MAX;
                    if (rn1 < fill_fraction) {
                        state[i][j] = YANG;
                    } else {
                        state[i][j] = 0;
                    }
                }
            }
            gens = play_life(state, hash_keys, &live_cells);
            if (gens == -1) {
                // This should never happen
                printf("Over 1,000,000 generations!?\n");
            } else {
                total_generations += gens;
                total_live_cells += live_cells;
            }
        }
        average_generations = (float)total_generations / iters_per_frac;
        average_live_cells = (float)total_live_cells / iters_per_frac;
        printf("\nAverage lifespan of %f generations.\n\n", average_generations);
        // Write to file in csv format
        fprintf(foutput, "%f,%f,%f\n", fill_fraction, average_generations, average_live_cells);
    }

    fclose(foutput);
}

// Count number of neighbors around a cell
int count_neighbors(int state[SIZE][SIZE], int i, int j) {
    int n = SIZE;
    int num_neighbors = 0;

    num_neighbors = state[(i-1+n)%n][(j-1+n)%n] + state[(i+n)%n][(j-1+n)%n] + state[(i+1)%n][(j-1+n)%n] +
                    state[(i-1+n)%n][(j+n)%n]                              + state[(i+1)%n][(j+n)%n] +
                    state[(i-1+n)%n][(j+1)%n] + state[(i+n)%n][(j+1)%n] + state[(i+1)%n][(j+1)%n];

    return num_neighbors;
}

// Generate a random 64-bit number, used for hashing
uint64_t rand_U64() {
    uint64_t int1 = (uint64_t) rand();
    uint64_t int2 = (uint64_t) rand();
    return int1 | (int2 << 32);
}

int play_life(int state[SIZE][SIZE], uint64_t hash_keys[SIZE][SIZE], int *live_cells) {
    // Next game state
    int next[SIZE][SIZE] = {0};
    // Hash "table"
    uint64_t hash_table[HT_SIZE];
    // For keeping track of how many generations have been stored
    int hash_index = 0;

    int num_neighbors = 0;
    int n = SIZE;
    int i,j;

    int num_generations = 0;

    // Breaks and returns at cycle detection condition
    while(true) {
        // Reset next game state
        memset(next, 0, sizeof(int) * SIZE * SIZE);

        uint64_t hashkey = 0;

        // Loop over all cells
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                num_neighbors = count_neighbors(state, i, j);

                if (state[i][j]) {
                    hashkey ^= hash_keys[i][j];
                    if (num_neighbors == 2 || num_neighbors == 3) next[i][j] = YANG;
                } else {
                    if (num_neighbors == 3) next[i][j] = YANG;
                }
            }
        }
        // Detect if cycle occurs and return if so
        // critical bug: will not detect cycles that occur at the limits of 
        //  the hash table size. Not an issue if the total number of generations
        //  is less than HT_SIZE (i.e. 16384) but this points to the fact that
        //  a proper hash table would be much better than a simple array.
        for (i = 0; i < hash_index; i++) {
            if (hashkey == hash_table[i]) {

                // Count number of living cells and set out parameter
                for (i = 0; i < SIZE; i++) {
                    for (j = 0; j < SIZE; j++) {
                        *live_cells += state[i][j];
                    }
                }
                return num_generations;
            }
        }

        // Set current game state to next game state
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
    
                state[i][j] = next[i][j];
            }
        }
        // If the code gets to this point, the next generation is 'successful'
        num_generations += 1;

        // Add hash to hash "table", start flushing out old hashes if too many
        hash_table[hash_index] = hashkey;
        hash_index++;
        if (hash_index >= HT_SIZE) {
            hash_index = 0;
        }

        if (num_generations >= 1000000) {
            // Should never happen but you never know
            return -1;
        }

    }    
}