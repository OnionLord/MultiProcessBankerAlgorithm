#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <semaphore.h>
#include <sys/stat.h>
#include<time.h>



#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 4

#define INIT 0
#define SAFE 1
#define UNSAFE 2
#define WAKEUP 3
#define SLEEP 4

struct socket
{
    int customerNo;
    int status;
    int request[NUMBER_OF_RESOURCES];
    int done_line;
    int lines;
};
typedef struct socket data;


struct information
{
    int numberOfCustomer;
    int join_customer[NUMBER_OF_CUSTOMERS];
    data data[NUMBER_OF_CUSTOMERS];
    int isReady;
    int finish[NUMBER_OF_CUSTOMERS];
    int work[NUMBER_OF_RESOURCES];
    int accepted;
    int isSafe[NUMBER_OF_CUSTOMERS];
    int switching;
    int available[NUMBER_OF_RESOURCES];
    int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
    int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
    int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
    int maxcomplete;
    sem_t round;
    sem_t sem;

};typedef struct information inform;

inform *totalPacket;


int main(int argc, char * argv[])
{
    int fd= shm_open("/banker",O_RDWR,0600);
    
    int ret, sval;
    
    int i, j, temp;
    
    int myCustomerNumber;
    
    int numberOfRequest;
    int requests[NUMBER_OF_RESOURCES];
    
    int inputStatus = 0;
    
    int running = 0;
    
    int timer_result;
    
    int allow_go = 0;
    
    FILE *file;
    
    struct timespec ts = {60,0};
    
    time_t start, now;
    
    if(argc != 3)
    {
        printf("Enter Process's Number and Filename");
        return 1;
    }
    
    
    
    if(fd == -1)
    {
        printf("Open failed\n");
        return 1;
    }
    ftruncate(fd, sizeof(inform));
    
    
    totalPacket = mmap(NULL, sizeof(inform),PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd, 0);
    if (totalPacket  == MAP_FAILED) {
        printf("Mapping Error!\n");
        return 1;
    }
    
    //input customer number and filename by argument
    myCustomerNumber = atoi(argv[1]);
    file = fopen(argv[2],"r");
    
    
    fscanf(file,"%d", &numberOfRequest);

    if((totalPacket)->join_customer[myCustomerNumber] != 0)
    {
        printf("The Number %d is already Connected. Please Try Later\n", myCustomerNumber);
        return 1;
    }
    (totalPacket)->data[myCustomerNumber].lines = numberOfRequest;
    (totalPacket)->data[myCustomerNumber].done_line = 0;
    
    while(1)
    {

        //printf("%d\n", running);
        if(running == 1 && (totalPacket)->isReady == 2)
        {
            //printf("1q2w3e4r\n");
            sem_wait(&(totalPacket)->round);
            //printf("EEEEEEEE\n");
            sem_wait(&(totalPacket)->sem);
            //printf("state : %d\n",(totalPacket)->switching);
            if((totalPacket)->switching == WAKEUP)
            {
                //printf("working\n");
                //In This Comopound-statement, Doing Main Algorithm of Customer
                //Iteration statement for customer request
                (totalPacket)->accepted = myCustomerNumber;
                if((totalPacket)->isSafe[myCustomerNumber] == SAFE || allow_go == 1)//Step 3
                {
                    allow_go = 0;//For 1st request
                    if((totalPacket)->finish[myCustomerNumber] == 1)//Step 4
                    {
                        (totalPacket)->accepted = myCustomerNumber;
                        (totalPacket)->data[myCustomerNumber].status = -1;
                        (totalPacket)->finish[myCustomerNumber] = 2;
                        sem_post(&(totalPacket)->sem);
                        //printf("#5\n");
                        sem_post(&(totalPacket)->round);
                        (totalPacket)->switching = 2;
                        (totalPacket)->numberOfCustomer = (totalPacket)->numberOfCustomer - 1;
                        sleep(2);
                        break;
                    }
                    else//Step 5
                    {
                        
                        
                        for ( i = 0 ; i < NUMBER_OF_RESOURCES ; i ++)//Step 1
                        {
                            fscanf(file,"%d", &temp);
                            (totalPacket)->data[myCustomerNumber].request[i]=temp;
                            //input requests
                            time(&start);
                            //get start time, timeout starts
                        }
                        
                        
                    }
                }
                else
                {
                    //when request > need -> fail.
                    if((totalPacket)->finish[myCustomerNumber] == 4)
                    {
                        //printf("%d:something is wrong\n",myCustomerNumber);
                        (totalPacket)->accepted = myCustomerNumber;
                        (totalPacket)->data[myCustomerNumber].status = -1;
                        (totalPacket)->finish[myCustomerNumber] = 3;
                        sem_post(&(totalPacket)->sem);
                        //printf("#1\n");
                        sem_post(&(totalPacket)->round);
                        (totalPacket)->switching = 2;
                        (totalPacket)->numberOfCustomer = (totalPacket)->numberOfCustomer - 1;
                        //sleep(2);
                        break;
                    }
                    //when all customer gets unsafe
                    if((totalPacket)->finish[myCustomerNumber] == 5)
                    {
                        (totalPacket)->accepted = myCustomerNumber;
                        (totalPacket)->data[myCustomerNumber].status = -1;
                        (totalPacket)->finish[myCustomerNumber] = 3;
                        sem_post(&(totalPacket)->sem);
                        //printf("#2\n");
                        sem_post(&(totalPacket)->round);
                        (totalPacket)->switching = 2;
                        (totalPacket)->numberOfCustomer = (totalPacket)->numberOfCustomer - 1;
                        //sleep(2);
                        break;
                    }
                    time(&now);//get what time is now
                    if(difftime(now,start) < 60) //before 60 sec
                    {
                    }
                    else//over 60sec -> error message print and quit.
                    {
                        printf("%d:something is wrong\n",myCustomerNumber);
                        (totalPacket)->accepted = myCustomerNumber;
                        (totalPacket)->data[myCustomerNumber].status = -1;
                        (totalPacket)->finish[myCustomerNumber] = 3;
                        sem_post(&(totalPacket)->sem);
                        //printf("#3\n");
                        sem_post(&(totalPacket)->round);
                        (totalPacket)->switching = 2;
                        (totalPacket)->numberOfCustomer = (totalPacket)->numberOfCustomer - 1;
                        //sleep(2);
                        break;
                    }
                }
                
                (totalPacket)->switching = SLEEP;
                //sleep(1);
                //customer's turn complete. and the customer sleeps
            }
            
            sem_post(&(totalPacket)->sem);
            //printf("#4\n");
            sem_post(&(totalPacket)->round);
            
            

        }
        else
        {
            sem_wait(&(totalPacket)->sem);
            if((totalPacket)->join_customer[myCustomerNumber] == 0)
            {
                //printf("Joinning\n");
                (totalPacket)->join_customer[myCustomerNumber] = 1;
                (totalPacket)->numberOfCustomer += 1;
                (totalPacket)->data[myCustomerNumber].customerNo = myCustomerNumber;
                
            }
            if((totalPacket)->isReady == 1)
            {
                //printf("I am Ready!");
                //after get ready state from banker
                
                //input max(shared memory)
                for ( i = 0 ; i < NUMBER_OF_RESOURCES ; i ++)
                {
                    fscanf(file,"%d", &(totalPacket)->maximum[myCustomerNumber][i]);
                    
                }
                (totalPacket)->data[myCustomerNumber].status = 0;
                (totalPacket)->maxcomplete++;
                allow_go = 1;
                //notice to banker that customer's work is end.
                running = 1;
                
            }
            sem_post(&(totalPacket)->sem);
        }

        sleep(1);
    }
    (totalPacket)->join_customer[myCustomerNumber] = 0;
    fclose(file);

    
    return 0;
}

