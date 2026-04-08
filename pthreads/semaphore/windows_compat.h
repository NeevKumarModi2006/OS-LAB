#ifndef WINDOWS_COMPAT_H
#define WINDOWS_COMPAT_H

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>

typedef HANDLE sem_t;
typedef HANDLE pthread_t;
typedef void* pthread_attr_t;

// Semaphore implementation
static inline int sem_init(sem_t *sem, int pshared, unsigned int value) {
    *sem = CreateSemaphore(NULL, value, 0x7FFFFFFF, NULL);
    return (*sem == NULL) ? -1 : 0;
}

static inline int sem_wait(sem_t *sem) {
    return (WaitForSingleObject(*sem, INFINITE) == WAIT_OBJECT_0) ? 0 : -1;
}

static inline int sem_post(sem_t *sem) {
    return ReleaseSemaphore(*sem, 1, NULL) ? 0 : -1;
}

static inline int sem_destroy(sem_t *sem) {
    return CloseHandle(*sem) ? 0 : -1;
}

// Thread implementation
static inline int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
    *thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start_routine, arg, 0, NULL);
    return (*thread == NULL) ? -1 : 0;
}

static inline int pthread_join(pthread_t thread, void **retval) {
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}

#else
// Fallback for non-Windows
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#endif

#endif // WINDOWS_COMPAT_H
