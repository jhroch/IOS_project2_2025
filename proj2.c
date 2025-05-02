/**
 * @file proj2.c
 * @brief IOS Project 2 - Synchronization (Přívoz)
 * @author Jiří Hroch xhrochj00
 * @date 29.4.2025
 */

#include <stdlib.h>
#include <unistd.h> 
#include <fcntl.h>
#include <time.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdarg.h> 
#include "proj2.h"

SharedMemory *shm;  
FILE *outputFile; 
int N, O, K, TA, TP; 

int initFunction(void){
    outputFile = fopen("proj2.out", "w"); 
    if(!outputFile){ 
        fprintf(stderr, "Cannot open file\n");
        return 1;
    }

    shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shm == MAP_FAILED){
        fprintf(stderr, "Shared mem failed\n");
        fclose(outputFile);
        return 1;
    }

    shm->actionCounter = 0;
    shm->onboardCars = 0;
    shm->totalCars = N + O;
    shm->port = 0;  // start at port 0
    shm->waitingCars[0][0] = 0;
    shm->waitingCars[0][1] = 0;
    shm->waitingCars[1][0] = 0;
    shm->waitingCars[1][1] = 0;

    // init semaphores
    if(sem_init(&shm->sem_output, 1, 1) == -1 ||
    sem_init(&shm->sem_ferryReady, 1, 0) == -1 ||
    sem_init(&shm->sem_mutex, 1, 1) == -1 ||
    sem_init(&shm->sem_unboarded, 1, 0) == -1 ||
    sem_init(&shm->sem_boarded, 1, 0) == -1 ||
    sem_init(&shm->sem_ferryArrived[0], 1, 0) == -1 ||
    sem_init(&shm->sem_ferryArrived[1], 1, 0) == -1 ||
    sem_init(&shm->sem_unboarding[0], 1, 0) == -1 ||
    sem_init(&shm->sem_unboarding[1], 1, 0) == -1 ||
    sem_init(&shm->sem_boarding[0][0], 1, 0) == -1 ||
    sem_init(&shm->sem_boarding[0][1], 1, 0) == -1 ||
    sem_init(&shm->sem_boarding[1][0], 1, 0) == -1 ||
    sem_init(&shm->sem_boarding[1][1], 1, 0) == -1 ){
        fprintf(stderr, "Semaphore error\n");
        munmap(shm, sizeof(SharedMemory));
        fclose(outputFile);
        return 1;
    }
    return 0;
}

void cleanup(void) {
    fclose(outputFile);
    
    sem_destroy(&shm->sem_output);
    sem_destroy(&shm->sem_ferryReady);
    sem_destroy(&shm->sem_mutex);
    sem_destroy(&shm->sem_boarded);
    sem_destroy(&shm->sem_unboarded);
    sem_destroy(&shm->sem_ferryArrived[0]);
    sem_destroy(&shm->sem_ferryArrived[1]);
    sem_destroy(&shm->sem_unboarding[0]);
    sem_destroy(&shm->sem_unboarding[1]);
    sem_destroy(&shm->sem_boarding[0][0]);
    sem_destroy(&shm->sem_boarding[0][1]);
    sem_destroy(&shm->sem_boarding[1][0]);
    sem_destroy(&shm->sem_boarding[1][1]);
    
    munmap(shm, sizeof(SharedMemory));
}

// write to output
void fprintfOutput(const char *format, ...){
    sem_wait(&shm->sem_output);
    
    va_list args;
    va_start(args, format);
    fprintf(outputFile, "%d: ", ++(shm->actionCounter));
    vfprintf(outputFile, format, args); 
    fprintf(outputFile, "\n");
    fflush(outputFile);
    va_end(args);
    
    sem_post(&shm->sem_output);
}

// ferry process
void ferryProcess(void){
    fprintfOutput("P: started");
    int onboardUnits = 0;  // in normal car units (N = 3)
    
    while(shm->totalCars > 0 || shm->onboardCars > 0){ 
        // travel time to dock
        if(TP > 0) usleep(rand() % (TP + 1));
        fprintfOutput("P: arrived to %d", shm->port);

        // unload cars
        sem_wait(&shm->sem_mutex);
        int onboard_vehicles = shm->onboardCars;
        shm->onboardCars = 0;
        
        // signal unboarding
        for(int i = 0; i < onboard_vehicles; i++){
            sem_post(&shm->sem_unboarding[shm->port]);
        }
        
        // wait for all to unboard
        for(int i = 0; i < onboardUnits; i++){
            sem_wait(&shm->sem_unboarded);
        }
        onboardUnits = 0;

        // signal arrival
        int waiting = shm->waitingCars[shm->port][0] + shm->waitingCars[shm->port][1];
        for(int i = 0; i < waiting; i++){
            sem_post(&shm->sem_ferryArrived[shm->port]);
        }

        // load cars
        int loadedUnits = 0;
        int loadedVehicles = 0;
        int turn = 1;
        while(turn == 1 && loadedUnits < K && (shm->waitingCars[shm->port][0] > 0 || shm->waitingCars[shm->port][1] > 0)){
            if(shm->waitingCars[shm->port][1] > 0 && loadedUnits + 3 <= K){
                shm->waitingCars[shm->port][1]--;
                loadedUnits += 3;
                loadedVehicles++;
                sem_post(&shm->sem_boarding[shm->port][1]);
                turn = 0;
            } 
            else if(turn == 0 && shm->waitingCars[shm->port][0] > 0 && loadedUnits < K){
                shm->waitingCars[shm->port][0]--;
                loadedUnits += 1;
                loadedVehicles++;
                sem_post(&shm->sem_boarding[shm->port][0]);
                turn = 1;
            } 
            else if(shm->waitingCars[shm->port][0] > 0 && loadedUnits < K){
                shm->waitingCars[shm->port][0]--;
                loadedUnits += 1;
                loadedVehicles++;
                sem_post(&shm->sem_boarding[shm->port][0]);
            } 
            else{
                break;
            }
        }
        shm->onboardCars = loadedVehicles;
        onboardUnits = loadedUnits;
        sem_post(&shm->sem_mutex);

        // wait for all to be boarded
        for(int i = 0; i < loadedVehicles; i++){
            sem_wait(&shm->sem_boarded);
        }

        // leaving
        fprintfOutput("P: leaving %d", shm->port);
        for(int i = 0; i < loadedVehicles; i++){
            sem_post(&shm->sem_ferryReady);
        }
        shm->port = (shm->port + 1) % 2;  // switch port
    }

    usleep(rand() % (TP + 1));
    fprintfOutput("P: finish");
    exit(0);
}

// car process
void carProcess(int id, int type, int port){
    char carType;
    if(type == 1){
        carType = 'N';
    }
    else if(type == 0){
        carType = 'O';
    }
    fprintfOutput("%c %d: started", carType, id);
    
    // arrival time
    usleep(rand() % (TA + 1));
    fprintfOutput("%c %d: arrived to %d", carType, id, port);

    // add to waiting
    sem_wait(&shm->sem_mutex);
    shm->waitingCars[port][type]++;
    sem_post(&shm->sem_mutex);

    // wait for ferry
    sem_wait(&shm->sem_ferryArrived[port]);

    // board
    sem_wait(&shm->sem_boarding[port][type]);
    fprintfOutput("%c %d: boarding", carType, id);
    sem_post(&shm->sem_boarded);

    // wait for ride
    sem_wait(&shm->sem_ferryReady);

    // unboard
    int dest_port = (port + 1) % 2;
    sem_wait(&shm->sem_unboarding[dest_port]);
    fprintfOutput("%c %d: leaving in %d", carType, id, dest_port);
    
    // signal unboarding (truck=3, car=1)
    if(type == 1){
        sem_post(&shm->sem_unboarded);
        sem_post(&shm->sem_unboarded);
        sem_post(&shm->sem_unboarded);
    } else{
        sem_post(&shm->sem_unboarded);
    }

    sem_wait(&shm->sem_mutex);
    shm->totalCars--;
    sem_post(&shm->sem_mutex);

    exit(0);
}

// main function
int main(int argc, char *argv[]){
    if(argc != 6){
        fprintf(stderr, "Wrong number of arguments\n");
        return 1;
    }

    N = atoi(argv[1]);
    O = atoi(argv[2]);
    K = atoi(argv[3]);
    TA = atoi(argv[4]);
    TP = atoi(argv[5]);
    
    if(N >= MAX_CARS || O >= MAX_CARS || K < 3 || K > MAX_CAPACITY ||
    TA < 0 || TA > MAX_TA || TP < 0 || TP > MAX_TP ){
        fprintf(stderr, "Bad value of arguments\n");
        return 1;
    }

    srand(time(NULL)); 
    if(initFunction() == 1){
        cleanup();
        return 1;
    }

    // create ferry
    pid_t ferry_pid = fork();
    if(ferry_pid == 0){
        ferryProcess();
    } else if(ferry_pid < 0){
        fprintf(stderr, "Fork error\n");
        cleanup();
        return 1;
    }

    // create cars
    pid_t pids[O+N];
    int pid_count = 0;
    
    for(int i = 1; i <= O; i++){
        pids[pid_count] = fork();
        if(pids[pid_count] == 0){
            srand(time(NULL)*getpid()); 
            carProcess(i, 0, rand() % 2);  // random port for O
        } else if(pids[pid_count] < 0){
            fprintf(stderr, "Fork error\n");
            cleanup();
            return 1;
        }
        pid_count++;
    }
    
    for(int i = 1; i <= N; i++){
        pids[pid_count] = fork();
        if(pids[pid_count] == 0){
            srand(time(NULL)*getpid());
            carProcess(i, 1, rand() % 2);  // random port for N
        } else if(pids[pid_count] < 0){
            fprintf(stderr, "Fork error\n");
            cleanup();
            return 1;
        }
        pid_count++;
    }

    // wait for all children processes
    while(wait(NULL) > 0);

    cleanup();
    return 0;
}

/*** End of file proj2.c ***/