#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define BUF_SIZE 200

typedef struct waiter_info
{
    int earning;
    int TableNo;
} WI;

int main()
{
    // shared memory shared between manager.c and admin.c
    // This block is only reading messages from admin
    key_t key;
    int shmid;
    char *shm, buff[BUF_SIZE];  
    if ((key = ftok("admin", 10)) == -1)
    { // '.' = current directory
        perror("error in ftok()");
        exit(-1);
    }
    if ((shmid = shmget(key, sizeof(char[BUF_SIZE]), 0666 | IPC_CREAT)) == -1)
    {
        perror("error in shmget()");
        exit(-2);
    }
    shm = (char *)shmat(shmid, NULL, 0); // shmat returns a void pointer
    while (1)
    {
        if (shm[0] != '\0')
        {
            strcpy(buff, shm);
            printf("Message from Admin: %s\n", buff);
            memset(shm, '\0', sizeof(shm));
        }
    }
    shmdt(shm);
    shmctl(shmid, IPC_RMID, 0);

    // ------------------------------------------------------------------------------------------------------------
    // Hotel Manager Logic -- this is the main manager logic

    int total_tables;

    printf("Enter the total number of tables in the restaurant: ");
    scanf("%d", &total_tables);

    /*O - based indexing used for table nos - 1 to 10 , table 1 ==== waiter_shmid[0]*/
    int waiter_shmid[10];
    key_t waiter_key[10];
    WI *waiter_info[10];

    for (int i = 0; i < 10; i++)
    {
        //create 10 diff keys for 10 diff tables
        if ((waiter_key[i] = ftok("admin", i + 1)) == -1) // Use i+1 to generate unique keys
        {
            perror("error in ftok()");
            exit(-1);
        }
        // create 10 diff shared emmory segments for 10 diff tables
        if ((waiter_shmid[i] = shmget(waiter_key[i], sizeof(WI), 0666 | IPC_CREAT)) == -1)
        {
            perror("error in shmget()");
            exit(-2);
        }

        // Attach to the shared memory segment
        if ((waiter_info[i] = (WI *)shmat(waiter_shmid[i], NULL, 0)) == (WI *)-1)
        {
            perror("error in shmat()");
            exit(-3);
        }
    }



    return 0;
}