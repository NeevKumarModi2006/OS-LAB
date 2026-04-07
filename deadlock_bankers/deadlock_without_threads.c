#include <stdio.h>

#define MAX 10

int n = 5; // processes
int m = 3; // resources

int Allocation[5][3] = { {0,1,0}, {2,0,0}, {3,0,2}, {2,1,1}, {0,0,2} };
int Max[5][3]        = { {7,5,3}, {3,2,2}, {9,0,2}, {2,2,2}, {4,3,3} };
int Need[5][3];

int Available[3]     = {3,3,2};
int Finish[5];
int SafeSeq[5];

void findSafeSequences(int work[], int finish[], int seq[], int count) {

    if (count == n) {
        // Found one safe sequence
        printf("Safe Sequence: ");
        for (int i = 0; i < n; i++) {
            printf("P%d", seq[i]);
            if (i != n - 1) printf(" -> ");
        }
        printf("\n");
        return;
    }

    for (int i = 0; i < n; i++) {

        if (finish[i] == 0) {
            int canExecute = 1;

            for (int j = 0; j < m; j++) {
                if (Need[i][j] > work[j]) {
                    canExecute = 0;
                    break;
                }
            }

            if (canExecute) {
                // Choose process i
                for (int j = 0; j < m; j++)
                    work[j] += Allocation[i][j];

                seq[count] = i;
                finish[i] = 1;

                // Recurse
                findSafeSequences(work, finish, seq, count + 1);

                // Backtrack
                finish[i] = 0;
                for (int j = 0; j < m; j++)
                    work[j] -= Allocation[i][j];
            }
        }
    }
}

int main() {

    // Step 1: Calculate Need
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            Need[i][j] = Max[i][j] - Allocation[i][j];

    // Step 2: Initialize
    int work[3];
    for (int i = 0; i < m; i++)
        work[i] = Available[i];

    for (int i = 0; i < n; i++)
        Finish[i] = 0;

    printf("All Safe Sequences are:\n");

    // Step 3: Find all sequences
    findSafeSequences(work, Finish, SafeSeq, 0);

    return 0;
}

// #include <stdio.h>

// int main() {
//     int n = 5; // Number of processes
//     int m = 3; // Number of resource types

//     // Hardcoded system state for detection
//     int Allocation[5][3] = { {0, 1, 0}, {2, 0, 0}, {3, 0, 3}, {2, 1, 1}, {0, 0, 2} };
//     int Request[5][3]    = { {0, 0, 0}, {2, 0, 2}, {0, 0, 0}, {1, 0, 0}, {0, 0, 2} };
//     int Available[3]     = {0, 0, 0}; 

//     int Finish[5] = {0};
//     int Work[3];

//     // Initialize Work array with Available resources
//     for(int i = 0; i < m; i++) {
//         Work[i] = Available[i];
//     }

//     // Detection Algorithm
//     for (int k = 0; k < n; k++) {
//         for (int i = 0; i < n; i++) {
//             if (Finish[i] == 0) { // Process hasn't finished yet
//                 int flag = 0;
//                 for (int j = 0; j < m; j++) {
//                     if (Request[i][j] > Work[j]) { // If Request > Work, it cannot proceed
//                         flag = 1; 
//                         break;
//                     }
//                 }

//                 // If current requests can be satisfied
//                 if (flag == 0) {
//                     for (int y = 0; y < m; y++) {
//                         Work[y] += Allocation[i][y]; // Reclaim its allocated resources
//                     }
//                     Finish[i] = 1; // Mark as able to finish
//                 }
//             }
//         }
//     }

//     // Check which processes could not finish (they are deadlocked)
//     int deadlock = 0;
//     printf("Deadlocked Processes: ");
//     for (int i = 0; i < n; i++) {
//         if (Finish[i] == 0) {
//             printf("P%d ", i);
//             deadlock = 1;
//         }
//     }

//     if (!deadlock) {
//         printf("None.\nThe system is deadlock-free.\n");
//     } else {
//         printf("\n");
//     }

//     return 0;
// }