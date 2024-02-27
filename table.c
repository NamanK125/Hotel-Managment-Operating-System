#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>

#define MAX_CUSTOMERS 5
#define MENU_FILE "menu.txt"

typedef struct {
    int table_number;
    int num_customers;
    int order[MAX_CUSTOMERS];
    int total_bill;
    int waiterSignal;
    int terminate_waiter;
    bool closeHotel;
} TableData;

// Function to create a waiter process
void createWaiterProcess(int tableNumber)
{
    if (fork() == 0)
    {
        // Child process runs the waiter.c code
        execlp("./waiter.out", "waiter.out", NULL);
        perror("Error in execlp for waiter process\n");
        exit(1);
    }
}

int main() {
    key_t key;
    int shmid;
    TableData *shmPtr;

    // Generate key for shared memory
    if ((key = ftok("table.c", 'A')) == -1) {
        perror("Error in ftok\n");
        return 1;
    }

    // Create a shared memory segment
    shmid = shmget(key, sizeof(TableData), 0644 | IPC_CREAT);
    if (shmid == -1) {
        perror("Error in shmget\n");
        return 1;
    }

    // Attach the shared memory segment to the process
    shmPtr = (TableData *)shmat(shmid, NULL, 0);
    if (shmPtr == (void *)-1) {
        perror("Error in shmat\n");
        return 1;
    }

    // Table process logic
    while (1) {
        printf("Enter Table Number: ");
        scanf("%d", &(shmPtr->table_number));

        printf("Enter Number of Customers at Table (maximum no. of customers can be 5): ");
        scanf("%d", &(shmPtr->num_customers));

        // Read menu from file
        FILE *menuFile = fopen(MENU_FILE, "r");
        if (menuFile == NULL) {
            perror("Error opening menu file\n");
            return 1;
        }

        printf("Menu:\n");
        char menu_item[100];
        int menu_index = 1;
        while (fgets(menu_item, sizeof(menu_item), menuFile) != NULL) {
            printf("%d. %s", menu_index++, menu_item);
        }
        fclose(menuFile);

        // Customer order input
        for (int i = 0; i < shmPtr->num_customers; ++i) {
            printf("Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done: ");
            scanf("%d", &(shmPtr->order[i]));
        }

        // Communicate with waiter (shared memory)
        // (Waiter process is not implemented in this code)
        // For each table process, create a corresponding waiter process
        for (int i = 1; i <= 10; i++)
        {
            createWaiterProcess(i);
        }

        // Display total bill
        printf("The total bill amount is %d INR.\n", shmPtr->total_bill);

        // Ask if new customers will be seated
        int new_customers;
        printf("Enter the number of new customers (1-5) or -1 to terminate: ");
        scanf("%d", &new_customers);

        if (new_customers == -1) {
            // Inform waiter to terminate
            shmPtr->terminate_waiter = 1;
            // Detach shared memory
            if (shmdt(shmPtr) == -1) {
                perror("Error in shmdt\n");
                return 1;
            }

            break; // Exit the loop and terminate the table process
        }

        // Signal waiter to terminate under some condition (modify as needed)
        if (new_customers == -1) {
            shmPtr->waiterSignal = 1;
        }

        // Detach shared memory
        if (shmdt(shmPtr) == -1) {
            perror("Error in shmdt\n");
            return 1;
        }

        // Hotel closure check
        if (shmPtr->closeHotel) {
            printf("Hotel is closing. Table process terminating.\n");
            // Detach shared memory
            if (shmdt(shmPtr) == -1) {
                perror("Error in shmdt\n");
                return 1;
            }
            exit(0);
        }
    }

    return 0;
}
