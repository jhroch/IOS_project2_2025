/**
 * @file proj2.h
 * @brief Header file for IOS Project 2 2025 Synchronization (Přívoz)
 * @author Jiří Hroch xhrochj00
 * @date 29.4.2025
 */

#ifndef PROJ2_H
#define PROJ2_H

#include <stdio.h>
#include <semaphore.h>

#define MAX_CARS 10000 
#define MAX_CAPACITY 100
#define MAX_TA 10000
#define MAX_TP 1000

/**
 * @brief Struct for shared memory with variables and unnamed semaphores
 */
typedef struct {
    int actionCounter;       
    int waitingCars[2][2];      // Waiting cars [port][0=car, 1=truck]
    int onboardCars;            // Number of loaded cars (in normal car units -> N = 3)
    int totalCars;              // Total number of cars to transport (N + O)
    int port;                    // Current port (0 or 1)
    sem_t sem_output;            // Sem for output
    sem_t sem_ferryReady;       // Ferry ready for departure
    sem_t sem_mutex;             // Sem for editing variables
    sem_t sem_ferryArrived[2];  // Ferry arrived [port]
    sem_t sem_unboarding[2];     // Unboarding [port]
    sem_t sem_boarding[2][2];    // Boarding [port][0=car, 1=truck]
    sem_t sem_boarded;           // Finished boarding
    sem_t sem_unboarded;         // Finished unboarding
} SharedMemory;

/**
 * @brief Init resources
 * @return 0 success, 1 failed.
 */
int initFunction(void);

/**
 * @brief Cleans up resources
 */
void cleanup(void);

/**
 * @brief Basically fprintf with fflush and output semaphore 
 */
void fprintfOutput(const char *format, ...);

/**
 * @brief Ferry process
 */
void ferryProcess(void);

/**
 * @brief Car process
 * @param id Id of the car.
 * @param type Type of car (0=car, 1=truck).
 * @param port Starting port (0 or 1).
 */
void carProcess(int id, int type, int port);

#endif // PROJ2_H