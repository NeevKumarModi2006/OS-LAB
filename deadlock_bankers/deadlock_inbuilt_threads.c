//with threads
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define P 3 // Number of processes
#define R 3 // Number of resource types

int Available[R] = {3, 3, 2};
int Max[P][R]    = { {7, 5, 3}, {3, 2, 2}, {9, 0, 2} };
int Alloc[P][R]  = { {0, 1, 0}, {2, 0, 0}, {3, 0, 2} };
int Need[P][R];

pthread_mutex_t lock;
// pthread_cond_t wait_cond;

// Function to check if the current state is safe
int isSafe() {
    int Work[R], Finish[P] = {0};
    for (int i = 0; i < R; i++) Work[i] = Available[i];

    int count = 0;
    while (count < P) {
        int found = 0;
        for (int p = 0; p < P; p++) {
            if (Finish[p] == 0) {
                int j;
                for (j = 0; j < R; j++) {
                    if (Need[p][j] > Work[j]) break;
                }
                if (j == R) { // Can satisfy this process
                    for (int k = 0; k < R; k++) Work[k] += Alloc[p][k];
                    Finish[p] = 1;
                    found = 1;
                    count++;
                }
            }
        }
        if (found == 0) return 0; // Unsafe state
    }
    return 1; // Safe state
}

void* process_thread(void* arg) {
    int id = *(int*)arg;
    int request[R] = {1, 0, 0}; // Example request

    pthread_mutex_lock(&lock);
    printf("Process %d requesting resources...\n", id);

    // Pretend to allocate
    for (int i = 0; i < R; i++) {
        Available[i] -= request[i];
        Alloc[id][i] += request[i];
        Need[id][i] -= request[i];
    }

    if (isSafe()) {
        printf("  -> Request granted for Process %d (System is SAFE)\n", id);
          for (int i = 0; i < R; i++) {
            Available[i] += request[i];
            Alloc[id][i] -= request[i];
            Need[id][i] += request[i];
        }
    } else {
        printf("  -> Request denied for Process %d (System would be UNSAFE). Reverting...\n", id);
        // Revert allocation
        for (int i = 0; i < R; i++) {
            Available[i] += request[i];
            Alloc[id][i] -= request[i];
            Need[id][i] += request[i];
        }
    }
    
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    pthread_t threads[P];
    int pids[P] = {0, 1, 2};

    pthread_mutex_init(&lock, NULL);
    // pthread_cond_init(&wait_cond, NULL);

    // Calculate initial Need matrix
    for (int i = 0; i < P; i++)
        for (int j = 0; j < R; j++)
            Need[i][j] = Max[i][j] - Alloc[i][j];

    // Create process threads
    for (int i = 0; i < P; i++) {
        pthread_create(&threads[i], NULL, process_thread, &pids[i]);
        sleep(1); // Slight delay to stagger output
    }

    for (int i = 0; i < P; i++) pthread_join(threads[i], NULL);

    return 0;
}


// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include <unistd.h>

// #define P 3 // Processes
// #define R 3 // Resource types

// int Available[R] = {0, 0, 0};
// int Alloc[P][R]  = { {0, 1, 0}, {2, 0, 0}, {3, 0, 3} };
// int Request[P][R]= { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} }; // dynamically updated

// pthread_mutex_t lock;

// // The detection algorithm
// void detect_deadlock() {
//     int Work[R], Finish[P] = {0};
//     for (int i = 0; i < R; i++) Work[i] = Available[i];

//     for (int i = 0; i < P; i++) {
//         // If allocation is 0, it's not involved in deadlock
//         int sum = 0;
//         for(int j=0; j<R; j++) sum += Alloc[i][j];
//         if (sum == 0) Finish[i] = 1; 
//     }

//     int count = 0;
//     while (1) {
//         int found = 0;
//         for (int i = 0; i < P; i++) {
//             if (Finish[i] == 0) {
//                 int j;
//                 for (j = 0; j < R; j++) {
//                     if (Request[i][j] > Work[j]) break;
//                 }
//                 if (j == R) { // Can finish
//                     for (int k = 0; k < R; k++) Work[k] += Alloc[i][k];
//                     Finish[i] = 1;
//                     found = 1;
//                 }
//             }
//         }
//         if (found == 0) break;
//     }

//     // Check who didn't finish
//     int deadlocked = 0;
//     for (int i = 0; i < P; i++) {
//         if (Finish[i] == 0) {
//             printf("DETECTOR: Process %d is DEADLOCKED!\n", i);
//             deadlocked = 1;
//         }
//     }
//     if (!deadlocked) printf("DETECTOR: System is currently deadlock-free.\n");
// }

// void* process_thread(void* arg) {
//     int id = *(int*)arg;
    
//     // Simulate a process making a request
//     pthread_mutex_lock(&lock);
//     printf("Process %d is making a request...\n", id);
//     Request[id][0] = 1; // It requests 1 instance of Resource 0
//     Request[id][1] = 0;
//     Request[id][2] = 2; // It requests 2 instances of Resource 2
//     pthread_mutex_unlock(&lock);

//     // Simulate process waiting for resources
//     sleep(5); 
//     return NULL;
// }

// void* detector_thread(void* arg) {
//     sleep(2); // Give processes time to make their requests
//     pthread_mutex_lock(&lock);
//     printf("\n--- Running Deadlock Detection ---\n");
//     detect_deadlock();
//     pthread_mutex_unlock(&lock);
//     return NULL;
// }

// int main() {
//     pthread_t p_threads[P], d_thread;
//     int pids[P] = {0, 1, 2};

//     pthread_mutex_init(&lock, NULL);

//     // Create process threads
//     for (int i = 0; i < P; i++) {
//         pthread_create(&p_threads[i], NULL, process_thread, &pids[i]);
//     }
//     // Create the detection thread
//     pthread_create(&d_thread, NULL, detector_thread, NULL);

//     for (int i = 0; i < P; i++) pthread_join(p_threads[i], NULL);
//     pthread_join(d_thread, NULL);

//     return 0;
// }
