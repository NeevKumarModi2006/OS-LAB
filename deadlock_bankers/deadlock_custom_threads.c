
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_THREADS 10
#define STACK_SIZE 32768

typedef enum { FREE, READY, RUNNING, FINISHED, BLOCKED } state_t;

// --- DATA STRUCTURES ---

typedef struct {
    int id;
    ucontext_t context;
    char *stack;
    state_t state;
    int priority; 
    int burst_time;       
    int arrival_time;     
    int waiting_thread_id; 
    void (*func)(); 
} thread_t;

typedef struct {
    int is_locked;                     
    int owner_id;                      
    int waiting_threads[MAX_THREADS];  
    int wait_count;                    
} mutex_t;

typedef struct {
    int waiting_threads[MAX_THREADS];  
    int wait_count;                    
} condition_t;

// --- GLOBAL VARIABLES ---
thread_t threads[MAX_THREADS];
int current_thread = -1;
int global_time_counter = 0; 
int ticks = 0; 

// SCHEDULING ALGORITHM SELECTOR:
// 0: FCFS | 1: SJF | 2: Priority | 3: Round Robin
int scheduling_algorithm = 3; 

void schedule();
void timer_handler(int signum);

// --- CRITICAL SECTION PROTECTION ---
void disable_timer() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &set, NULL);
}

void enable_timer() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
}

// --- MUTEX IMPLEMENTATION ---
void mutex_init(mutex_t *m) {
    m->is_locked = 0;
    m->owner_id = -1;
    m->wait_count = 0;
}

void mutex_lock(mutex_t *m) {
    disable_timer(); 
    if (m->is_locked == 0) {
        m->is_locked = 1;
        m->owner_id = current_thread;
    } else {
        m->waiting_threads[m->wait_count++] = current_thread;
        threads[current_thread].state = BLOCKED;
        schedule(); 
    }
    enable_timer(); 
}

void mutex_unlock(mutex_t *m) {
    disable_timer(); 
    if (m->is_locked == 1 && m->owner_id == current_thread) {
        if (m->wait_count > 0) {
            int next_owner = m->waiting_threads[0];
            for (int i = 1; i < m->wait_count; i++) {
                m->waiting_threads[i - 1] = m->waiting_threads[i];
            }
            m->wait_count--;
            threads[next_owner].state = READY;
            m->owner_id = next_owner; 
        } else {
            m->is_locked = 0;
            m->owner_id = -1;
        }
    }
    enable_timer(); 
}

// --- CONDITION VARIABLE IMPLEMENTATION ---
void condition_init(condition_t *cond) {
    cond->wait_count = 0;
}

void condition_wait(condition_t *cond, mutex_t *m) {
    mutex_unlock(m); // Unlock BEFORE sleeping to prevent deadlocks
    
    disable_timer(); 
    cond->waiting_threads[cond->wait_count++] = current_thread;
    threads[current_thread].state = BLOCKED;
    schedule(); 
    enable_timer(); 
    
    mutex_lock(m); // Re-acquire lock upon waking up
}

void condition_signal(condition_t *cond) {
    disable_timer(); 
    if (cond->wait_count > 0) {
        int woken_thread = cond->waiting_threads[0];
        for (int i = 1; i < cond->wait_count; i++) {
            cond->waiting_threads[i - 1] = cond->waiting_threads[i];
        }
        cond->wait_count--;
        threads[woken_thread].state = READY;
    }
    enable_timer(); 
}

void condition_broadcast(condition_t *cond) {
    disable_timer(); 
    for (int i = 0; i < cond->wait_count; i++) {
        int woken_thread = cond->waiting_threads[i];
        threads[woken_thread].state = READY;
    }
    cond->wait_count = 0; 
    enable_timer(); 
}

// --- THREAD LIFECYCLE MANAGEMENT ---
void init_threading() {
    for (int i = 0; i < MAX_THREADS; i++) {
        threads[i].state = FREE;
        threads[i].waiting_thread_id = -1;
    }
    threads[0].id = 0;
    threads[0].state = RUNNING;
    threads[0].priority = 0;
    threads[0].burst_time = 9999; 
    threads[0].arrival_time = global_time_counter++;
    threads[0].waiting_thread_id = -1;
    current_thread = 0;

    struct sigaction sa;
    sa.sa_handler = timer_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGVTALRM, &sa, NULL);

    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 50000; 
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 50000;
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

void thread_wrapper() {
    enable_timer(); 
    threads[current_thread].func();
    thread_exit();  
}

int thread_create(void (*func)(), int priority, int burst_time) {
    disable_timer(); 
    int id = -1;
    for (int i = 1; i < MAX_THREADS; i++) {
        if (threads[i].state == FREE) {
            id = i;
            break;
        }
    }
    if (id == -1) {
        enable_timer();
        return -1; 
    }

    getcontext(&threads[id].context);
    threads[id].stack = malloc(STACK_SIZE);
    threads[id].context.uc_stack.ss_sp = threads[id].stack;
    threads[id].context.uc_stack.ss_size = STACK_SIZE;
    threads[id].context.uc_link = NULL; 
    
    threads[id].func = func;
    makecontext(&threads[id].context, thread_wrapper, 0);
    
    threads[id].id = id;
    threads[id].state = READY;
    threads[id].priority = priority;
    threads[id].burst_time = burst_time;                  
    threads[id].arrival_time = global_time_counter++;     
    threads[id].waiting_thread_id = -1;

    enable_timer(); 
    return id;
}

void thread_exit() {
    disable_timer(); 
    threads[current_thread].state = FINISHED;
    if (threads[current_thread].stack != NULL) {
        free(threads[current_thread].stack);
        threads[current_thread].stack = NULL;
    }

    int waiting_id = threads[current_thread].waiting_thread_id;
    if (waiting_id != -1 && threads[waiting_id].state == BLOCKED) {
        threads[waiting_id].state = READY;
    }
    schedule(); 
}

int thread_join(int target_id) {
    disable_timer(); 
    if (target_id < 0 || target_id >= MAX_THREADS || threads[target_id].state == FREE) {
        enable_timer();
        return -1; 
    }
    if (threads[target_id].state == FINISHED) {
        enable_timer();
        return 0; 
    }
    threads[current_thread].state = BLOCKED;
    threads[target_id].waiting_thread_id = current_thread;
    schedule(); 
    enable_timer(); 
    return 0;
}

// --- PRIORITY RECALCULATION & SCHEDULING ---
void recompute_priority() {
    for (int i = 1; i < MAX_THREADS; i++) {
        if (threads[i].state == READY) {
            threads[i].priority += 1; 
        } else if (threads[i].state == RUNNING && threads[i].priority > 0) {
            threads[i].priority -= 1; 
        }
    }
}

void timer_handler(int signum) {
    ticks++;
    if (ticks % 10 == 0) recompute_priority();

    if (current_thread != -1 && threads[current_thread].state == RUNNING) {
        threads[current_thread].state = READY;
        if (threads[current_thread].burst_time > 0) threads[current_thread].burst_time--; 
    }
    schedule();
}

void schedule() {
    int prev_thread = current_thread;
    int next_thread = -1;

    if (scheduling_algorithm == 0) {
        if (current_thread != -1 && (threads[current_thread].state == RUNNING || threads[current_thread].state == READY)) {
            next_thread = current_thread;
        } else {
            int min_arrival = 999999;
            for (int i = 0; i < MAX_THREADS; i++) {
                if (threads[i].state == READY && threads[i].arrival_time < min_arrival) {
                    min_arrival = threads[i].arrival_time;
                    next_thread = i;
                }
            }
        }
    } 
    else if (scheduling_algorithm == 1) {
        int min_burst = 999999;
        for (int i = 0; i < MAX_THREADS; i++) {
            if (threads[i].state == READY && threads[i].burst_time < min_burst) {
                min_burst = threads[i].burst_time;
                next_thread = i;
            }
        }
    }
    else if (scheduling_algorithm == 2) {
        int max_pri = -1;
        for (int i = 0; i < MAX_THREADS; i++) {
            if (threads[i].state == READY && threads[i].priority > max_pri) {
                max_pri = threads[i].priority;
                next_thread = i;
            }
        }
    }
    else if (scheduling_algorithm == 3) {
        for (int i = 1; i <= MAX_THREADS; i++) {
            int check = (current_thread + i) % MAX_THREADS;
            if (threads[check].state == READY) {
                next_thread = check;
                break;
            }
        }
    }

    if (next_thread == -1 && current_thread != -1 && (threads[current_thread].state == READY || threads[current_thread].state == RUNNING)) {
        threads[current_thread].state = RUNNING;
        return;
    }

    int all_finished = 1;
    for(int i = 1; i < MAX_THREADS; i++) {
        if(threads[i].state != FINISHED && threads[i].state != FREE) {
            all_finished = 0;
            break;
        }
    }
    if (all_finished && threads[0].state == BLOCKED) {
        exit(0); 
    } else if (next_thread == -1) {
        return; 
    }

    current_thread = next_thread;
    threads[current_thread].state = RUNNING;

    if (prev_thread != current_thread) {
        swapcontext(&threads[prev_thread].context, &threads[current_thread].context);
    }
}

// ========================================================
// TEST APPLICATION: The Producer-Consumer Problem
// ========================================================

#define BUFFER_SIZE 5
int buffer[BUFFER_SIZE];
int count = 0;

mutex_t m;
condition_t cond_full;
condition_t cond_empty;

void producer() {
    for (int i = 1; i <= 10; i++) {
        mutex_lock(&m);
        
        // Wait if buffer is full
        while (count == BUFFER_SIZE) {
            printf("Producer: Buffer full. Waiting...\n");
            condition_wait(&cond_full, &m);
        }
        
        // Produce item
        buffer[count] = i;
        count++;
        printf("Producer: Added item %d (Buffer count: %d)\n", i, count);
        
        // Signal consumer that buffer is no longer empty
        condition_signal(&cond_empty);
        mutex_unlock(&m);
        
        for(volatile int j=0; j<10000000; j++); // Simulate work
    }
}

void consumer() {
    for (int i = 1; i <= 10; i++) {
        mutex_lock(&m);
        
        // Wait if buffer is empty
        while (count == 0) {
            printf("Consumer: Buffer empty. Waiting...\n");
            condition_wait(&cond_empty, &m);
        }
        
        // Consume item
        int item = buffer[count - 1];
        count--;
        printf("Consumer: Removed item %d (Buffer count: %d)\n", item, count);
        
        // Signal producer that buffer is no longer full
        condition_signal(&cond_full);
        mutex_unlock(&m);
        
        for(volatile int j=0; j<20000000; j++); // Simulate work
    }
}

int main() {
    init_threading();
    scheduling_algorithm = 3; // Round Robin for aggressive preemption
    
    mutex_init(&m);
    condition_init(&cond_full);
    condition_init(&cond_empty);
    
    printf("Starting Producer-Consumer test...\n\n");

    int prod_id = thread_create(producer, 1, 10);
    int cons_id = thread_create(consumer, 1, 10);

    thread_join(prod_id);
    thread_join(cons_id);

    printf("\nAll threads finished successfully. No deadlocks occurred.\n");
    return 0;
}