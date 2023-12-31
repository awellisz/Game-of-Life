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

int count_neighbors(int state[SIZE][SIZE], int i, int j);
void display(int state[SIZE][SIZE]);
void parse_fen(char *fen, int (*state)[SIZE]);
uint64_t rand_U64();
void announce_winner(int state[SIZE][SIZE]);

int main(int argc, char *argv[]) {
    // Used for keeping track of how long it's taken to render a frame
    //  to keep the framerate consistent. Probably not ideal
    struct timeval stop, start;
    long long start_ms, stop_ms, timediff;

    int nseed = 0;
    int n = SIZE;
    int num_neighbors, setup_mode, is_tribal, 
        cycle_detect_on, i, j, preset_selection, speed_selection;

    // For FEN string parsing
    int fen_buffer_size = SIZE * SIZE + SIZE;
    char fen_input[fen_buffer_size];

    // Current game state
    int state[SIZE][SIZE] = {0};
    // Next game state
    int next[SIZE][SIZE] = {0};
    // Used for storing hash keys for each cell
    uint64_t hash_keys[SIZE][SIZE];
    // Initialize hash_keys to random values
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            hash_keys[i][j] = rand_U64(); 
        }
    }

    // Hash table
    // Literally just an array lmao 
    uint64_t hash_table[HT_SIZE];

    printf("ENTER A NUMBER TO CHOOSE LIFE MODE:\n");
    printf("    0: SINGLE-ORGANISM\n");
    printf("    1: TWO-ORGANISM (COMPETITIVE TRIBES)\n");
    scanf("%d", &is_tribal);

    printf("DO YOU WANT CYCLE DETECTION ENABLED?\n");
    printf("(Answer '0' e.g. if you explicitly want to watch cyclic pattern repeat)\n");
    printf("    0: NO\n");
    printf("    1: YES\n");
    scanf("%d", &cycle_detect_on);

    printf("CHOOSE ANIMATION SPEED:\n");
    printf("    0: 1 FPS (VERY SLOW)\n");
    printf("    1: 5 FPS (SLOW)\n");
    printf("    2: 10 FPS (NORMAL)\n");
    printf("    3: 20 FPS (FAST)\n");
    printf("    4: 50 FPS (VERY FAST)\n");
    scanf("%d", &speed_selection);

    printf("ENTER A NUMBER TO CHOOSE SETUP MODE:\n");
    printf("    0: RANDOM\n");
    printf("    1: MANUAL (SET INDIVIDUAL CELLS)\n");
    printf("    2: BATCH MANUAL (INPUT FEN STRING)\n");
    printf("    3: VIEW PRESET OPTIONS\n");
    scanf("%d", &setup_mode);

    int framerate = 10; // Frames per second
    switch(speed_selection) {
        case 0: framerate = 1; break;
        case 1: framerate = 5; break;
        case 2: framerate = 10; break;
        case 3: framerate = 20; break;
        case 4: framerate = 50; break;
        default: 
            printf("Invalid input.\n"); 
            exit(0);
    }
    int ms_per_frame = 1000 / framerate; 

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
                        if (is_tribal && (j > n/2)) {
                            state[i][j] = KOHM;
                        } else {
                            state[i][j] = YANG;
                        }
                        
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
                display(state);
            }
            break;
        case BATCH:

            // Clear input buffer
            while ((i = getchar()) != '\n' && i != EOF) {}

            printf("Enter a valid FEN string: ");
            fgets(fen_input, sizeof(fen_input), stdin);
            // Strip trailing newline
            if (fen_input[strlen(fen_input) - 1] == '\n') {
                fen_input[strlen(fen_input) - 1] = '\0';
            }
            parse_fen(fen_input, state);
            break;
        case PRESET:
            printf("ENTER A NUMBER TO CHOOSE A PRESET:\n");
            printf("    0: GLIDER\n");
            printf("    1: R PENTOMINO\n");
            printf("    2: IMPOSTOR (sus)\n");
            printf("    3: 2 SQUARES (cool symmetric patterns)\n");
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

    display(state);

    // For keeping track of how many generations have been stored
    int hash_index = 0;

    int total_generations = 0;

    // Main game loop
    while (true) {
        printf("\x1b[?25l"); // hide the cursor
        total_generations++;
        // Keep track of when this frame began
        gettimeofday(&start, NULL);
        start_ms = (((long long)start.tv_sec)*1000)+(start.tv_usec/1000);
        // Reset next state
        memset(next, 0, sizeof(int) * SIZE * SIZE);

        uint64_t hashkey = 0;

        // Loop over all cells
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {

                num_neighbors = count_neighbors(state, i, j);

                // Living Yangs
                switch (state[i][j]) {
                    case YANG:
                        hashkey ^= hash_keys[i][j];

                        if (num_neighbors == 2 || num_neighbors == 3) {
                            next[i][j] = YANG;
                        } else if (num_neighbors == -3) {
                            next[i][j] = KOHM;
                        }
                        break;
                    case KOHM:
                        hashkey ^= ~(hash_keys[i][j]);

                        if (num_neighbors == -2 || num_neighbors == -3) {
                            next[i][j] = KOHM;
                        } else if (num_neighbors == 3) {
                            next[i][j] = YANG;
                        }
                        break;
                    default: 
                        if (num_neighbors == 3) {
                            next[i][j] = YANG;
                        } else if (num_neighbors == -3) {
                            next[i][j] = KOHM;
                        }
                }
            }
        }

        // Detect cycles
        if (cycle_detect_on) {
            for (i = 0; i < HT_SIZE; i++) {
                if (hashkey == hash_table[i]) {
                    printf("\x1b[?25h"); // show the cursor
                    printf("Cycle detected with a period of %d generations.\n", hash_index - i + 1);
                    printf("Total game length: %d\n", total_generations);
                    if (is_tribal) announce_winner(state);
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
        display(state);
    }
}

// Print current game state to screen
void display(int state[SIZE][SIZE]) {
    system("clear"); // prevents flickering
    int i, j;
    int n = SIZE;
    printf("\n");
    
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < n; j++) {
            switch (state[i][j]) {
                // 2 Unicode rectangles make up one square cell
                case YANG: printf(BLUE "\u2588\u2588"); break;
                case KOHM: printf(RED "\u2588\u2588"); break;
                default: printf("  "); break;
            }
        }
        printf(RESET "\n");
    }
}

// Count number of neighbors around a cell
int count_neighbors(int state[SIZE][SIZE], int i, int j) {
    int num_neighbors = 0;

    for(int di = -1; di <= 1; di++) {
        for(int dj = -1; dj <= 1; dj++) {
            // Skip the cell itself
            if(di == 0 && dj == 0) continue; 
            // Handling toric boundary
            int ni = (i + di + SIZE) % SIZE;
            int nj = (j + dj + SIZE) % SIZE;
            num_neighbors += state[ni][nj];
        }
    }
    return num_neighbors;
}

// Generate a random 64-bit number, used for hashing
uint64_t rand_U64() {
    uint64_t int1 = (uint64_t) rand();
    uint64_t int2 = (uint64_t) rand();
    return int1 | (int2 << 32);
}

// Find which tribe has more living cells at the end of the game
void announce_winner(int state[SIZE][SIZE]) {
    int tribal_balance = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            tribal_balance += state[i][j];
        }
    }
    if (tribal_balance == 0) {
        printf("It was a draw!\n");
    } else if (tribal_balance > 0) {
        printf("Yangs win with %d more cells!\n", tribal_balance);
    } else {
        printf("Kohms win with %d more cells!\n", tribal_balance * -1);
    }
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