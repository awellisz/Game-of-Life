#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#define NUM_TYPES 7
#define HEIGHT 32
#define WIDTH 64

#define RESET  "\x1B[0m"

void display(int state[HEIGHT][WIDTH]);

int main(int argc, char *argv[]) {
    // Used for keeping track of how long it's taken to render a frame
    //  to keep the framerate consistent. Probably not ideal
    struct timeval stop, start;
    long long start_ms, stop_ms, timediff;
    int framerate = 5; // Frames per second
    int ms_per_frame = 1000 / framerate; 

    int nseed = 12345;
    int i, j;

    // How far to search for neighbors
    int d = 1;
    // Threshold number of neighbors to get 'eaten'
    int threshold = 1;
    // moore or von neumann neighborhood
    int neighborhood = 1;

    // Current game state
    int state[HEIGHT][WIDTH] = {0};
    // Next game state
    int next[HEIGHT][WIDTH] = {0};

    printf("ENTER A FIVE DIGIT INTEGER TO CHANGE SEED\n");
    scanf("%d", &nseed);
    srand(nseed);

    printf("\nCHOOSE CELL NEIGHBORHOOD TYPE\n");
    printf("    0: Moore Neighborhood (square around cell)\n");
    printf("    1: von Neumann Neighborhood (excludes corners)\n");
    scanf("%d", &neighborhood);

    printf("\nENTER NEIGHBORHOOD SIZE\n");
    printf("(i.e. how far away from each cell to search for neighbors that can eat it)\n");
    scanf("%d", &d);

    printf("\nENTER CELL EATING THRESHOLD\n");
    printf("(i.e. how many neighbors that can eat the current cell are required before the cell is actually eaten)\n");
    scanf("%d", &threshold);

    // Generate random board
    float rn1;
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            state[i][j] = rand() % 7;
        }
    }
    printf("\x1b[?25l"); // hide the cursor   

    // Main game loop
    while (true) {
        // Keep track of when this frame began
        gettimeofday(&start, NULL);
        start_ms = (((long long)start.tv_sec)*1000)+(start.tv_usec/1000);

        display(state);

        // Reset next state
        memset(next, 0, sizeof(int) * HEIGHT * WIDTH);

        int num_neighbors;

        for (i = 0; i < HEIGHT; i++) {
            for (j = 0; j < WIDTH; j++) {
                num_neighbors = 0;

                for(int dx = -d; dx <= d; dx++) {
                    for(int dy = -d; dy <= d; dy++) {
                        // Skip the cell itself
                        if(dx == 0 && dy == 0) continue; 
                        
                        // in von Neumann neighborhood case
                        if (neighborhood==1) {
                            // Manhattan distance <= d
                            if(abs(dx) + abs(dy) > d) continue;
                        }

                        // Handling cyclic boundary
                        int ni = (i + dy + HEIGHT) % HEIGHT;
                        int nj = (j + dx + WIDTH) % WIDTH;

                        if (state[ni][nj] == (state[i][j] + 1)) {
                            num_neighbors++;
                        } else if ((state[i][j] == 6) && (state[ni][nj] == 0)) {
                            num_neighbors++;
                        }
                    }
                }

                if (num_neighbors >= threshold) {
                    next[i][j] = (state[i][j] + 1) % 7;
                } else {
                    next[i][j] = state[i][j];
                }
                

            }
        }

        // Set current game state to next game state
        for (i = 0; i < HEIGHT; i++) {
            for (j = 0; j < WIDTH; j++) {
                state[i][j] = next[i][j];
            }
        }  

        gettimeofday(&stop, NULL);
        // Get render time in milliseconds
        stop_ms = (((long long)stop.tv_sec)*1000)+(stop.tv_usec/1000);
        timediff = start_ms - stop_ms;
        // Sleep for the rest of the frame (keeps a consistent frame rate)
        if (timediff < ms_per_frame) {
            usleep(1000 * (ms_per_frame - timediff));
        }
    }
}

// Print current game state to screen
void display(int state[HEIGHT][WIDTH]) {
    system("clear");
    int i, j;

    char *colors[NUM_TYPES] = {
        "\x1b[38;5;163m", // violet
        "\x1b[38;5;17m", // indigo
        "\x1b[38;5;27m", // blue
        "\x1b[38;5;46m", // green
        "\x1b[38;5;226m", // yellow
        "\x1b[38;5;202m", // orange
        "\x1b[38;5;196m", // red
        };

    printf("\n");
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            printf("%s\u2588\u2588", colors[state[i][j]]); 
        }
        printf(RESET "\n");
    }
}