/**
 * @file proj2.c
 * @brief IOS Project 2 2025 Synchronization (Přívoz)
 * @author Jiří Hroch xhrochj00
 * @date 2025-04-27
 */

 #include <stdlib.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/wait.h>
 #include <sys/mman.h>
 #include <fcntl.h>
 #include <time.h>
 #include <stdarg.h>
 #include "proj2.h"
 
 SharedMemory *shm;
 FILE *output_file;
 int N, O, K, TA, TP;
 
 int init_resources(void) {
     // Open output file
     output_file = fopen("proj2.out", "w");
     if (!output_file) {
         fprintf(stderr, "Failed to open output file\n");
         return 1;
     }
 
     // Allocate shared memory
     shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
     if (shm == MAP_FAILED) {
         fprintf(stderr, "Failed to initialize shared memory\n");
         fclose(output_file);
         return 1;
     }
     shm->action_counter = 0;
     shm->onboard_cars = 0;
     shm->total_cars = N + O;
     shm->port = 0;
     for (int i = 0; i < 2; i++) {       // Port loop
         for (int j = 0; j < 2; j++) {   // Car type loop
             shm->waiting_cars[i][j] = 0;
         }
     }
 
     // Initialize unnamed semaphores
     if (sem_init(&shm->sem_output, 1, 1) == -1 ||
         sem_init(&shm->sem_ferry_ready, 1, 0) == -1 ||
         sem_init(&shm->sem_mutex, 1, 1) == -1 ||
         sem_init(&shm->sem_unboarded, 1, 0) == -1 ||
         sem_init(&shm->sem_boarded, 1, 0) == -1 ) {
         fprintf(stderr, "Failed to initialize semaphores\n");
         munmap(shm, sizeof(SharedMemory));
         fclose(output_file);
         return 1;
     }
     for (int i = 0; i < 2; i++) {           // Port loop
         if (sem_init(&shm->sem_ferry_arrived[i], 1, 0) == -1 ||
             sem_init(&shm->sem_unboarding[i], 1, 0) == -1) {
             fprintf(stderr, "Failed to initialize semaphores\n");
             munmap(shm, sizeof(SharedMemory));
             fclose(output_file);
             return 1;
         }
         for (int j = 0; j < 2; j++) {       // Car type loop
             if (sem_init(&shm->sem_boarding[i][j], 1, 0) == -1) {
                 fprintf(stderr, "Failed to initialize semaphores\n");
                 munmap(shm, sizeof(SharedMemory));
                 fclose(output_file);
                 return 1;
             }
         }
     }
     return 0;
 }
 
 void cleanup_resources(void) {
     fclose(output_file);
     sem_destroy(&shm->sem_output);
     sem_destroy(&shm->sem_ferry_ready);
     sem_destroy(&shm->sem_mutex);
     sem_destroy(&shm->sem_boarded);
     sem_destroy(&shm->sem_unboarded);
     for (int i = 0; i < 2; i++) {
         sem_destroy(&shm->sem_ferry_arrived[i]);
         sem_destroy(&shm->sem_unboarding[i]);
         for (int j = 0; j < 2; j++) {
             sem_destroy(&shm->sem_boarding[i][j]);
         }
     }
     munmap(shm, sizeof(SharedMemory));
 }
 
 void write_output(const char *format, ...) {
     sem_wait(&shm->sem_output);
     va_list args;
     va_start(args, format);
     fprintf(output_file, "%d: ", ++(shm->action_counter));
     vfprintf(output_file, format, args);
     fprintf(output_file, "\n");
     fflush(output_file);
     va_end(args);
     sem_post(&shm->sem_output);
 }
 
 void ferry_process(void) {
     write_output("P: started");
     while (shm->total_cars > 0 || shm->onboard_cars > 0) {
         // Port travel
         if (TP > 0) usleep(rand() % (TP + 1));
         write_output("P: arrived to %d", shm->port);
 
         // Signal arrival to waiting cars
         sem_wait(&shm->sem_mutex);
         int waiting = shm->waiting_cars[shm->port][0] + shm->waiting_cars[shm->port][1];
         sem_post(&shm->sem_mutex);
         for (int i = 0; i < waiting; i++) {
             sem_post(&shm->sem_ferry_arrived[shm->port]); // Unlocks semaphore for cars
         }
 
         // Unload cars
         sem_wait(&shm->sem_mutex);
         int onboard = shm->onboard_cars;
         shm->onboard_cars = 0;
         sem_post(&shm->sem_mutex);
         int unboarded = 0;
         for (int i = 0; i < onboard; i++) {
             sem_post(&shm->sem_unboarding[shm->port]);
             unboarded++;
         }
         for(int i = 0; i < unboarded; i++){
             sem_wait(&shm->sem_unboarded);
         }
 
         // Load cars (alternate truck/car)
         sem_wait(&shm->sem_mutex);
         int loaded = 0;
         int cars = 0;
         int turn = 1; // 1=truck, 0=car
         while (loaded < K && (shm->waiting_cars[shm->port][0] > 0 || shm->waiting_cars[shm->port][1] > 0)) {
             if (turn == 1 && shm->waiting_cars[shm->port][1] > 0 && loaded + 3 <= K) {
                 (shm->waiting_cars[shm->port][1])--;
                 loaded += 3;
                 cars += 1;
                 sem_post(&shm->sem_boarding[shm->port][1]);
                 turn = 0;
             } else if (turn == 0 && shm->waiting_cars[shm->port][0] > 0 && loaded < K) {
                 (shm->waiting_cars[shm->port][0])--;
                 loaded += 1;
                 cars += 1;
                 sem_post(&shm->sem_boarding[shm->port][0]);
                 turn = 1;
             } else if (shm->waiting_cars[shm->port][0] > 0 && loaded < K) {
                 (shm->waiting_cars[shm->port][0])--;
                 loaded += 1;
                 cars += 1;
                 sem_post(&shm->sem_boarding[shm->port][0]);
             } else {
                 break;
             }
         }
         shm->onboard_cars = loaded;
         sem_post(&shm->sem_mutex);
 
         // Depart from port
         for(int i = 0; i < cars; i++){       // Wait for every car to board
             sem_wait(&shm->sem_boarded);
         }   
         write_output("P: leaving %d", shm->port);
         for (int i = 0; i < loaded; i++) {
             sem_post(&shm->sem_ferry_ready);
         }
         shm->port = (shm->port + 1) % 2;
     }
 
     // Finish
     if (TP > 0) usleep(rand() % (TP + 1));
     write_output("P: finish");
     exit(0);
 }
 
 void car_process(int id, int type, int port) {
     char type_char = type ? 'N' : 'O';      // 1 = "N", 0 = "O"
     write_output("%c %d: started", type_char, id);
     if (TA > 0) usleep(rand() % (TA + 1));
     write_output("%c %d: arrived to %d", type_char, id, port);
 
     // Increment waiting cars count
     sem_wait(&shm->sem_mutex);
     (shm->waiting_cars[port][type])++;
     sem_post(&shm->sem_mutex);
 
     // Wait for ferry arrival
     sem_wait(&shm->sem_ferry_arrived[port]);
 
     // Wait for boarding permission
     sem_wait(&shm->sem_boarding[port][type]);
     write_output("%c %d: boarding", type_char, id);
     sem_post(&shm->sem_boarded);
 
     // Wait for ferry departure
     sem_wait(&shm->sem_ferry_ready);
 
     // Unboard at destination port
     int dest_port = (port + 1) % 2;
     sem_wait(&shm->sem_unboarding[dest_port]);
     write_output("%c %d: leaving in %d", type_char, id, dest_port);
     if(type == 1){
         for(int i = 0; i < 3; i++){
             sem_post(&shm->sem_unboarded);
         }
     }
     else{
         sem_post(&shm->sem_unboarded);
     }
 
     // Decrease total cars count
     sem_wait(&shm->sem_mutex);
     (shm->total_cars)--;
     sem_post(&shm->sem_mutex);
 
     exit(0);
 }
 
 int main(int argc, char *argv[]) {
     // Validate number of arguments
     if (argc != 6) {
         fprintf(stderr, "Invalid number of arguments\n");
         return 1;
     }
 
     // Validate and parse input arguments
     if (sscanf(argv[1], "%d", &N) != 1 || sscanf(argv[2], "%d", &O) != 1 ||
         sscanf(argv[3], "%d", &K) != 1 || sscanf(argv[4], "%d", &TA) != 1 ||
         sscanf(argv[5], "%d", &TP) != 1 ||
         N < 0 || N >= MAX_CARS || O < 0 || O >= MAX_CARS ||
         K < 3 || K > MAX_CAPACITY || TA < 0 || TA > MAX_TA || TP < 0 || TP > MAX_TP) {
         fprintf(stderr, "Invalid input parameters\n");
         return 1;
     }
 
     // Initialize random seed and resources
     srand(time(NULL));
     if (init_resources()) {
         cleanup_resources();
         return 1;
     }
 
     // Create ferry process
     pid_t ferry_pid = fork();
     if (ferry_pid == 0) {
         ferry_process();
     } else if (ferry_pid < 0) {
         fprintf(stderr, "Fork failed\n");
         cleanup_resources();
         return 1;
     }
 
     // Create car processes
     pid_t pids[O+N];
     int pid_count = 0;
     for (int i = 1; i <= O; i++) {
         pids[pid_count] = fork();
         if (pids[pid_count] == 0) {
             srand(time(NULL)*getpid());
             car_process(i, 0, rand() % 2); // Random port for normal cars
         } else if (pids[pid_count] < 0) {
             fprintf(stderr, "Fork failed\n");
             cleanup_resources();
             return 1;
         }
         pid_count++;
     }
     for (int i = 1; i <= N; i++) {
         pids[pid_count] = fork();
         if (pids[pid_count] == 0) {
             srand(time(NULL)*getpid());
             car_process(i, 1, rand() % 2); // Random port for trucks
         } else if (pids[pid_count] < 0) {
             fprintf(stderr, "Fork failed\n");
             cleanup_resources();
             return 1;
         }
         pid_count++;
     }
 
     // Wait for all child processes
     while(wait(NULL) > 0);
 
     cleanup_resources();
     return 0;
 }
 /*** End of file proj2.c ***/