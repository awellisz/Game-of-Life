#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#define SIZE 36
#define HT_SIZE 2048

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

// Some presets
#define GLIDER "1y/2y/yyy"
#define R_PENTOMINO "////////////////991yy/99yy/991y"
#define IMPOSTOR "///////////////99yyy/98yy/98yyyy/98yyyy/99y1y"
#define SQUARES "///////////////95yy/95yy1y1yy/991yy"

enum modes { RANDOM, MANUAL, BATCH, PRESET };

int count_neighbors(int state[SIZE][SIZE], int color[SIZE][SIZE], int i, int j, int *average_color);
void display(int state[SIZE][SIZE], int color[SIZE][SIZE]);
void parse_fen(char *fen, int (*state)[SIZE]);
uint64_t rand_U64();

int main(int argc, char *argv[]) {
    // Used for keeping track of how long it's taken to render a frame
    //  to keep the framerate consistent. Probably not ideal
    struct timeval stop, start;
    long long start_ms, stop_ms, timediff;
    int framerate = 10; // Frames per second
    int ms_per_frame = 1000 / framerate; 

    int nseed = 0;
    int n = SIZE;
    int num_neighbors, setup_mode, cycle_detect_on, i, j, preset_selection;

    // For FEN string parsing
    int fen_buffer_size = SIZE * SIZE + SIZE;
    char fen_input[fen_buffer_size];

    // Current game state
    int state[SIZE][SIZE] = {0};
    // Next game state
    int next[SIZE][SIZE] = {0};
    // Color of each cell
    int color[SIZE][SIZE] = {0};
    // Used for storing hash keys for each cell
    uint64_t hash_keys[SIZE][SIZE];
    // Initialize hash_keys to random values
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            hash_keys[i][j] = rand_U64(); 
        }
    }

    // Hash table
    uint64_t hash_table[HT_SIZE];

    printf("DO YOU WANT CYCLE DETECTION ENABLED?\n");
    printf("(Answer '0' e.g. if you explicitly want to watch cyclic pattern repeat)\n");
    printf("    0: NO\n");
    printf("    1: YES\n");
    scanf("%d", &cycle_detect_on);

    printf("ENTER A NUMBER TO CHOOSE SETUP MODE:\n");
    printf("    0: RANDOM\n");
    printf("    1: MANUAL (SET INDIVIDUAL CELLS)\n");
    printf("    2: BATCH MANUAL (INPUT FEN STRING)\n");
    printf("    3: VIEW PRESET OPTIONS\n");
    scanf("%d", &setup_mode);

    switch(setup_mode) {
        case RANDOM:
            printf("ENTER A FIVE DIGIT INTEGER TO CHANGE SEED\n");
            scanf("%d", &nseed);
            srand(nseed);

            float rn1;
            for (i = 0; i < n; i++) {
                for (j = 0; j < n; j++) {
                    rn1 = (double)rand() / RAND_MAX;
                    if (rn1 < 0.6) {
                        state[i][j] = 0;
                    } else { 
                        state[i][j] = 1;
                        color[i][j] = (double)rand() / RAND_MAX * 255;
                    }
                }
            }
            break;
        case MANUAL:
            while (true) {
                printf("ENTER COORDINATES OF LIVE CELLS AS I,J; WRITE 0,0 WHEN DONE\n");
                scanf("%d,%d", &i, &j);
                if (i == 0 && j == 0) break;
                state[i][j] = 1;
                display(state, color);
            }
            break;
        case BATCH:

            // Clear input buffer
            while ((i = getchar()) != '\n' && i != EOF) {}

            printf("Enter a valid FEN string: ");
            fgets(fen_input, sizeof(fen_input), stdin);
            if (fen_input[strlen(fen_input) - 1] == '\n') {
                fen_input[strlen(fen_input) - 1] = '\0';
            }
            parse_fen(fen_input, state);
            break;
        case PRESET:
            printf("ENTER A NUMBER TO CHOOSE A PRESET:\n");
            printf("    0: GLIDER\n");
            printf("    1: R PENTOMINO\n");
            printf("    2: IMPOSTOR\n");
            printf("    3: 2 SQUARES\n");
            scanf("%d", &preset_selection);

            switch(preset_selection) {
                case 0: parse_fen(GLIDER, state); break;
                case 1: parse_fen(R_PENTOMINO, state); break;
                case 2: parse_fen(IMPOSTOR, state); break;
                case 3: parse_fen(SQUARES, state); break;
                default: break;
            }

            break;
        default: printf("An error occured.\n"); exit(0);
    }

    display(state, color);

    // For keeping track of how many generations have been stored
    int hash_index = 0;

    int total_generations = 0;

    // Main game loop
    while (true) {
        total_generations++;
        // Keep track of when this frame began
        gettimeofday(&start, NULL);
        start_ms = (((long long)start.tv_sec)*1000)+(start.tv_usec/1000);
        // Reset next state
        memset(next, 0, sizeof(int) * SIZE * SIZE);

        uint64_t hashkey = 0;
        int average_color = 0;

        // Loop over all cells
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                average_color = 0;
                num_neighbors = count_neighbors(state, color, i, j, &average_color);
                if (state[i][j]) {
                    hashkey ^= hash_keys[i][j];
                    if (num_neighbors == 2 || num_neighbors == 3) {
                        next[i][j] = 1;
                    }
                } else {
                    if (num_neighbors == 3) { 
                        next[i][j] = 1;
                        color[i][j] = average_color;
                    }
                }
            }
        }

        // Detect cycles
        if (cycle_detect_on) {
            for (i = 0; i < HT_SIZE; i++) {
                if (hashkey == hash_table[i]) {
                    printf("Cycle detected with a period of %d generations.\n", hash_index - i + 1);
                    printf("Total game length: %d\n", total_generations);
                    exit(0);
                }
            }
        }


        // Set current game state to next game state
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                state[i][j] = next[i][j];
            }
        }  

        // Only store HT_SIZE number of previous generations
        hash_table[hash_index] = hashkey;
        hash_index++;
        if (hash_index >= HT_SIZE) {
            hash_index = 0;
        }

        gettimeofday(&stop, NULL);
        // Get render time in milliseconds
        stop_ms = (((long long)stop.tv_sec)*1000)+(stop.tv_usec/1000);
        timediff = start_ms - stop_ms;
        // Sleep for the rest of the frame (keeps a consistent frame rate)
        if (timediff < ms_per_frame) {
            usleep(1000 * (ms_per_frame - timediff));
        }
        display(state, color);
    }
}

// Print current game state to screen
void display(int state[SIZE][SIZE], int color[SIZE][SIZE]) {
    int i, j;
    int n = SIZE;
    char color_code[16]; // should never be longer than 14(?) chars
    printf("\n\n\n\n\n\n\n\n");
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < n; j++) {
            if (state[i][j]) {
                snprintf(color_code, sizeof(color_code), "\x1b[38;5;%dm", color[i][j]);
                printf("%s\u2588\u2588", color_code); 
            } else {
                printf("  ");
            }
        }
        printf(RESET "\n");
    }
}

// Count number of neighbors around a cell
int count_neighbors(int state[SIZE][SIZE], int color[SIZE][SIZE], int i, int j, int *average_color) {
    int n = SIZE;
    int num_neighbors = 0;
    int total_color = 0;

    for(int di = -1; di <= 1; di++) {
        for(int dj = -1; dj <= 1; dj++) {
            // Skip the cell itself
            if(di == 0 && dj == 0) continue; 
            // Handling toric boundary
            int ni = (i + di + SIZE) % SIZE;
            int nj = (j + dj + SIZE) % SIZE;

            num_neighbors += state[ni][nj];
            total_color += color[ni][nj];
        }
    }

    *average_color = total_color / 8;

    return num_neighbors;

}

// Generate a random 64-bit number, used for hashing
uint64_t rand_U64() {
    uint64_t int1 = (uint64_t) rand();
    uint64_t int2 = (uint64_t) rand();
    return int1 | (int2 << 32);
}


// Takes a FEN (Forsyth-Edwards Notation, adapted to Life) string and 
//  places it on the current game board.
void parse_fen(char *fen, int (*state)[SIZE]) {
    assert(fen != NULL);
    int count =  0;
    int i = 0;
    int j = 0;
    int cell = 0;
    while(*fen) {
        cell = 0;
        count = 1;
        // Switch on current character
        switch (*fen) {
            case 'y': cell = YANG; break;
            case 'k': cell = KOHM; break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                // subtract ASCII values from characters to get integer val
                count = *fen - '0';
                break;
            case '/': // at a new row
                j = 0; // go to start of row (1st column)
                i++; // increase row
                fen++;
                continue;
            default: // something wrong
                fprintf(stderr, "FEN parse error!\n");
                exit(0);
        }

        for (int k = 0; k < count; k++) {
            if (cell != 0) state[i][j] = cell;
            j++; // increase column
        }

        // Read next character
        fen++;
    }
}