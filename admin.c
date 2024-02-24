#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define BUF_SIZE 200

int main()
{
    key_t key;
    int shmid;
    char *shm, buff[BUF_SIZE];
    if ((key = ftok("admin.c", 10)) == -1)
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
    shm[0] = '\0';
    while (1)
    {
        printf("Do you want to close the hotel? Enter Y for Yes and N for No.\n");
        while (fgets(buff, sizeof(buff), stdin) != NULL)
        {
    
            int len = strlen(buff);
            if (buff[len - 1] == '\n')
                buff[len - 1] = '\0';
            strcpy(shm, buff);
            if ( strcmp(buff , "Y") == 0 || strcmp(buff , "y") == 0)
            {
                shmdt(shm);
                shmctl(shmid, IPC_RMID, 0);
                exit(0);
            }
        }
    }
    
    return 0;
}
