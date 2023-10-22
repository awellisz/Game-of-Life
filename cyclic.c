#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#define SIZE 10
#define NUM_TYPES 7

#define HEIGHT 40
#define WIDTH 60

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


    // Current game state
    int state[HEIGHT][WIDTH] = {0};
    // Next game state
    int next[HEIGHT][WIDTH] = {0};

    printf("ENTER A FIVE DIGIT INTEGER TO CHANGE SEED\n");
    scanf("%d", &nseed);
    srand(nseed);

    float rn1;
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            state[i][j] = rand() % 7;
        }
    }
           
    display(state);

    // Main game loop
    while (true) {

        // Keep track of when this frame began
        gettimeofday(&start, NULL);
        start_ms = (((long long)start.tv_sec)*1000)+(start.tv_usec/1000);

        // Reset next state
        memset(next, 0, sizeof(int) * HEIGHT * WIDTH);

        int n = HEIGHT;
        int m = WIDTH;



        for (i = 0; i < HEIGHT; i++) {
            for (j = 0; j < WIDTH; j++) {
                // Von Neumann neighborhood
                int neumann_neighbors[4] = {
                    state[(i-1+n)%n][(j+m)%m], // left
                    state[(i+n)%n][(j-1+m)%m], // top
                    state[(i+1)%n][(j+m)%m], // right
                    state[(i+n)%n][(j+m)%m] // bottom
                };

                for (int k = 0; k < 4; k++) {
                    if (neumann_neighbors[k] == (state[i][j] + 1)) {
                        next[i][j] = (state[i][j] + 1);
                        break;
                    } else if ((state[i][j] == 6) && (neumann_neighbors[k] == 0)) {
                        next[i][j] = 0;
                        break;
                    } else {
                        next[i][j] = state[i][j];
                    }
                }

                // Moore neighborhood
                // int moore_neighbors[8] = {
                //     state[(i-1+n)%n][(j-1+m)%m], 
                //     state[(i+n)%n][(j-1+m)%m], 
                //     state[(i+1)%n][(j-1+m)%m], 
                //     state[(i-1+n)%n][(j+m)%m],
                //     state[(i+1)%n][(j+m)%m],
                //     state[(i-1+n)%n][(j+1)%m],
                //     state[(i+n)%n][(j+1)%m],
                //     state[(i+1)%n][(j+1)%m]
                // };
                // for (int k = 0; k < 8; k++) {
                //     if (moore_neighbors[k] == (state[i][j] + 1)) {
                //         next[i][j] = (state[i][j] + 1);
                //     } else if ((state[i][j] == 6) && (moore_neighbors[k] == 0)) {
                //         next[i][j] = 0;
                //     } else {
                //         next[i][j] = state[i][j];
                //     }
                // }
            }
        }


        // Set current game state to next game state
        for (i = 0; i < HEIGHT; i++) {
            for (j = 0; j < WIDTH; j++) {
                state[i][j] = next[i][j];
            }
        }  
        display(state);
        

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
    int i, j;

    // char *colors[6] = {"\x1B[31m", "\x1B[32m", "\x1B[33m", 
    //                    "\x1B[34m", "\x1B[35m", "\x1B[36m"};
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
            //printf("%d ", state[i][j]);
            //printf("%s\u2588", colors[state[i][j]]); 
        }
        printf(RESET "\n");
    }
}