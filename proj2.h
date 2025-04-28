/**
 * @file proj2.h
 * @brief Header file for IOS Project 2 2025 Synchronization (Přívoz)
 * @author Jiří Hroch xhrochj00
 * @date 2025-04-27
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
  * @brief Structure for shared memory with variables and unnamed semaphores
  */
 typedef struct {
     int action_counter;       
     int waiting_cars[2][2];      // Waiting cars [port][0=car, 1=truck]
     int onboard_cars;            // Number of loaded cars (in normal car units)
     int total_cars;              // Total number of cars to transport (N + O)
     int port;                    // Current port (0 or 1)
     sem_t sem_output;            // Sem for output
     sem_t sem_ferry_ready;       // Ferry ready for departure
     sem_t sem_mutex;             // Sem for variables
     sem_t sem_ferry_arrived[2];  // Ferry arrived [port]
     sem_t sem_unboarding[2];     // Unboarding [port]
     sem_t sem_boarding[2][2];    // Boarding [port][0=car, 1=truck]
     sem_t sem_boarded;           // Finished boarding
     sem_t sem_unboarded;         // Finished unboarding
 } SharedMemory;
 
 /**
  * @brief Initializes resources
  * @return 0 success, 1 failure.
  */
 int init_resources(void);
 
 /**
  * @brief Cleans up resources
  */
 void cleanup_resources(void);
 
 /**
  * @brief Basically fprintf with fflush and output semaphore 
  */
 void write_output(const char *format, ...);
 
 /**
  * @brief Ferry process: loads/unloads cars.
  */
 void ferry_process(void);
 
 /**
  * @brief Car process: arrives at port, boards, unboards.
  * @param id Id of the car.
  * @param type Type of car (0=car, 1=truck).
  * @param port Starting port (0 or 1).
  */
 void car_process(int id, int type, int port);
 
 #endif // PROJ2_H