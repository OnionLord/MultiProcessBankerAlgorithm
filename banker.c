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


#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 4

#define INIT 0
#define SAFE 1
#define UNSAFE 2
#define WAKEUP 3
#define SLEEP 4

//I Take Some Sleep()s. May Took more times than others.
int finish[NUMBER_OF_CUSTOMERS];



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
//10 5 7 2
int main(int argc, char * argv[])
{
    int fd;
    int r = 0;
    int i,j, x, y;
    int cnt_fin;
    
    int safe;
    
    int ret, sval;
    
    int end_count[NUMBER_OF_CUSTOMERS];
    int final_cnt = 0;
    int fail_cnt = 0;
    

    shm_unlink("/banker");
        fd= shm_open("/banker",O_CREAT|O_RDWR,0600);
    if(fd == -1)
    {
        printf("Open failed\n");
        shm_unlink("/banker");
        return 1;
    }
    
    
    ftruncate(fd, sizeof(inform));
    if(r!=0)
    {
        printf("Truncate Error\n");
        return 1;
    }
    
    totalPacket = mmap(NULL, sizeof(inform),PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);
    if (totalPacket  == MAP_FAILED) {
        printf("Mapping Error!\n");
        return 1;
    }
    
	sem_init(&(totalPacket)->sem,1,1);
    sem_init(&(totalPacket)->round,1,1);
    (totalPacket)->numberOfCustomer = 0;
    (totalPacket)->isReady = 0;
    (totalPacket)->maxcomplete = 0;
    //(totalPacket)->readcount = 0;
    for ( i = 0 ; i < NUMBER_OF_RESOURCES ; i ++)
    {
        (totalPacket)->available[i] = atoi(argv[i+1]);
    }
    
    for (i = 0 ; i < NUMBER_OF_CUSTOMERS ; i ++)
    {
        end_count[i] = 0;
        (totalPacket)->join_customer[i] = 0;
    }
    
    while(1)
    {
        
        if((totalPacket)->isReady == 1)
        {
            for ( i = 0 ; i < NUMBER_OF_CUSTOMERS ; i ++)
            {
                for ( j = 0 ; j < NUMBER_OF_RESOURCES ; j ++)
                {
                    (totalPacket)->need[i][j] = (totalPacket)->maximum[i][j];
                }
            }
            if((totalPacket)->maxcomplete >= 5)
                (totalPacket)->isReady = 2;
        }
        else if((totalPacket)->isReady == 2)
        {
            if((totalPacket)->switching == WAKEUP)
            {
                continue;
            }
            sem_wait(&(totalPacket)->sem);
            
            if((totalPacket)->data[(totalPacket)->accepted].status == -1 &&                 end_count[(totalPacket)->accepted] != 1)
            {
                end_count[(totalPacket)->accepted] = 1;
            }
            else
            {
                
                
                if((totalPacket)->switching == SLEEP)
                {
                    //printf("works\n");
                    //7.5.3.2 Resource-Request Algorithm p.328,Need < Request -> error
                    /*printf("%d's turn : ",(totalPacket)->accepted);
                    for ( x = 0 ; x < NUMBER_OF_RESOURCES ; x++)
                    {
                        printf("%d ",(totalPacket)->data[(totalPacket)->accepted].request[x]);
                    }
                    printf("\n");*/
                    sleep(1);
                    for ( x = 0 ; x < NUMBER_OF_RESOURCES ; x ++)
                    {
                        if( (totalPacket)->need[(totalPacket)->accepted][x] < (totalPacket)->data[(totalPacket)->accepted].request[x] )
                        {
                            break;
                        }
                    }
                    
                    
                    
                    for ( i = 0 ; i < NUMBER_OF_RESOURCES ; i ++)
                    {
                        if( (totalPacket)->available[i] < (totalPacket)->data[(totalPacket)->accepted].request[i])
                        {
                            break;
                        }
                    }
                    if(x != NUMBER_OF_RESOURCES)
                    {
                        (totalPacket)->finish[(totalPacket)->accepted] = 4;
                        
                        (totalPacket)->switching = WAKEUP;
                        (totalPacket)->isSafe[(totalPacket)->accepted] = 0;
                        //printf("#3\n");
                        sem_post(&(totalPacket)->sem);
                        continue;
                    }
                    if( i == NUMBER_OF_RESOURCES && x == NUMBER_OF_RESOURCES)
                    {
                        //Resource-Request Algorithm's Step3.
                        for ( j = 0 ; j < NUMBER_OF_RESOURCES ; j ++)
                        {
                            (totalPacket)->need[(totalPacket)->accepted][j] -= (totalPacket)->data[(totalPacket)->accepted].request[j];
                            (totalPacket)->allocation[(totalPacket)->accepted][j] += (totalPacket)->data[(totalPacket)->accepted].request[j];
                            (totalPacket)->available[j] -= (totalPacket)->data[(totalPacket)->accepted].request[j];
                        }
                        
                        
                        for ( i = 0 ;i < NUMBER_OF_RESOURCES ; i ++)
                        {
                            (totalPacket)->work[i] = (totalPacket)->available[i];
                        }
                        for ( i = 0 ; i < NUMBER_OF_CUSTOMERS ; i ++)
                        {
                            finish[i] = 0;
                        }
                        
                        j = 0;
                        cnt_fin = 0;
                        
                        
                        while(1)
                        {
                            i = cnt_fin;
                            for ( x = 0 ; x < NUMBER_OF_CUSTOMERS ; x++)
                            {
                                for ( y = 0 ; y<NUMBER_OF_RESOURCES ; y++)
                                {
                                    if((totalPacket)->need[x][y] > (totalPacket)->work[y])
                                        break;
                                }
                                
                                if ( y == NUMBER_OF_RESOURCES && finish[x] == 0)
                                {
                                    for ( j = 0 ; j < NUMBER_OF_RESOURCES ; j ++)
                                    {
                                        (totalPacket)->work[j] += (totalPacket)->allocation[x][j];
                                    }
                                    finish[x] = 1;
                                    cnt_fin++;
                                }
                            }
                            
                            if(cnt_fin == NUMBER_OF_CUSTOMERS)
                            {
                                safe = 1;
                                break;
                            }
                            if(cnt_fin == i)
                            {
                                safe = 0;
                                break;
                            }
                        }
                        if( safe == 1)
                        {
                            //it is success!
                            (totalPacket)->isSafe[(totalPacket)->accepted] = SAFE;
                            (totalPacket)->data[(totalPacket)->accepted].done_line++;
                            //when finished
                            if((totalPacket)->data[(totalPacket)->accepted].lines ==(totalPacket)->data[(totalPacket)->accepted].done_line)
                            {
                                (totalPacket)->finish[(totalPacket)->accepted] = 1;
                                //release
                                for ( x = 0 ; x < NUMBER_OF_RESOURCES ; x++)
                                {
                                    if((totalPacket)->need[(totalPacket)->accepted][x] !=0)
                                        break;
                                }
                                
                                if(x == NUMBER_OF_RESOURCES)
                                {
                                    
                                    for ( y = 0 ; y < NUMBER_OF_RESOURCES ; y++)
                                    {
                                        (totalPacket)->need[(totalPacket)->accepted][y] += (totalPacket)->allocation[(totalPacket)->accepted][y];
                                        (totalPacket)->available[y] += (totalPacket)->allocation[(totalPacket)->accepted][y];
                                        (totalPacket)->allocation[(totalPacket)->accepted][y] = 0;
                                    }
                                }
                            }
                            
                        }
                        else
                        {
                            //this turn is fail
                            //and roll back;
                            for ( i = 0 ; i < NUMBER_OF_RESOURCES ; i ++)
                            {
                                (totalPacket)->need[(totalPacket)->accepted][i] += (totalPacket)->data[(totalPacket)->accepted].request[i];
                                (totalPacket)->allocation[(totalPacket)->accepted][i] -= (totalPacket)->data[(totalPacket)->accepted].request[i];
                                (totalPacket)->available[i] += (totalPacket)->data[(totalPacket)->accepted].request[i];
                            }
                            
                            (totalPacket)->isSafe[(totalPacket)->accepted] = UNSAFE;
                            
                        }
                    
                    }
                    else
                    {
                        
                        (totalPacket)->switching = WAKEUP;
                        (totalPacket)->isSafe[(totalPacket)->accepted] = UNSAFE;
                        //printf("#2\n");
                        //sem_post(&(totalPacket)->sem);

                    }
                    
                }
            }
            final_cnt = 0;
            fail_cnt = 0;
            for ( i = 0 ; i < NUMBER_OF_CUSTOMERS;i++)
            {
                if((totalPacket)->finish[i] == 2)
                {
                    final_cnt++;
                }
                if((totalPacket)->finish[i] == 3)
                {
                    fail_cnt ++;
                }
            }
            
            if(final_cnt + fail_cnt == NUMBER_OF_CUSTOMERS)
            {
                if(fail_cnt >= 1)
                {
                    sem_post(&(totalPacket)->sem);
                    sem_wait(&(totalPacket)->sem);
                    printf("unsafe state\n");
                    //(totalPacket)->result = 2;
                    break;
                }
                else
                {
                    sem_post(&(totalPacket)->sem);
                    sem_wait(&(totalPacket)->sem);
                    printf("Safely Terminated\n");
                    break;
                }
            }
            else
            {
                fail_cnt = 0;
                for ( i = 0 ; i < NUMBER_OF_CUSTOMERS ; i ++)
                {
                    if((totalPacket)->data[i].status != -1)
                    {
                        if((totalPacket)->isSafe[i]==UNSAFE)
                        {
                            fail_cnt++;
                        }
                    }
                }
                
                if(fail_cnt == (totalPacket)->numberOfCustomer)
                {
                    for ( i = 0 ; i < NUMBER_OF_CUSTOMERS ; i ++)
                    {
                        if((totalPacket)->data[i].status != -1)
                        {
                            if((totalPacket)->isSafe[i]==UNSAFE)
                            {
                                (totalPacket)->finish[i] = 5;
                            }
                        }
                    }
                }
            }
            
            
            
            (totalPacket)->switching = WAKEUP;
            sleep(1);
            //printf("#1\n");
            sem_post(&(totalPacket)->sem);
        }
        else
        {
            sem_wait(&(totalPacket)->sem);
            //printf("%d Customers Joined\n",(totalPacket)->numberOfCustomer);
            if((totalPacket)->numberOfCustomer >= 5)
            {
                (totalPacket)->isReady = 1;
                //sem_init(&(totalPacket)->round,1,1);

                //printf("ready\n");
            }
            //sleep(1);
            sem_post(&(totalPacket)->sem);

            (totalPacket)->switching = INIT;
            
            sem_wait(&(totalPacket)->sem);
            sem_post(&(totalPacket)->sem);
            
        }

    }
    
    munmap(totalPacket, sizeof(inform));
    shm_unlink("/banker");
    return 0;
}
