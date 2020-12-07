#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define INPUT_LENGTH 256
#define KATAR_LENGTH 10

char *read_input(char *path);
int search_katar(char *str, char *katar);
char *substring(char *str, int start_index, int length);
void partition_input(char *input, char *katar, char *local_input, int p, int my_rank);

int main(int argc, char *argv[])
{
    int my_rank;
    int size;
    int p;
    int i;
    MPI_Status status;
    char *input;
    char katar[KATAR_LENGTH] = "CATCGTTCAG";
    int *katar_locations;
    int local_katar_location;
    char *local_input;
    int size_per_proc;
    int partition_size;
    int katar_index;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    size_per_proc = (int)INPUT_LENGTH / p;
    partition_size = size_per_proc + KATAR_LENGTH - 1;

    if (my_rank == 0)
    {
        //If my rank is equal to 0 then initialize the variables
        input = read_input("./input.txt");
        katar_locations = (int *) malloc(sizeof(int)*p);
    }

    local_input = (char *) malloc(sizeof(char)*partition_size);
    partition_input(input,katar,local_input,p,my_rank);
    local_katar_location = search_katar(local_input,katar);

    MPI_Gather(&local_katar_location,1,MPI_INT,
                katar_locations,1,MPI_INT,
                0,MPI_COMM_WORLD);

    if(my_rank == 0)
    {
        printf("==================\n");
        for(i=0;i<p;i++)
        {
            if(katar_locations[i] == -1)
            {
                printf("Process %d: Cannot find the katar!\n",i);
            }
            else
            {
                katar_index = size_per_proc * i +katar_locations[i];
                printf("Process %d: Katar is found at index of %d\n",i,katar_index);
            }
        }
    }

    MPI_Finalize();
    return 0;
}

char *read_input(char *path)
{
    FILE *inputFile = fopen(path, "r");
    char *inputText = (char *)malloc(sizeof(char) * INPUT_LENGTH);
    if (inputFile == NULL)
    {
        printf("Cannot read input!\n");
        exit(1);
    }

    fscanf(inputFile, "%s", inputText);
    fclose(inputFile);
    return inputText;
}

int search_katar(char *str, char *katar)
{
    int str_iterator, katar_iterator, last_index;
    int is_search_started = 0; //0: false, 1: true

    last_index = 0;
    katar_iterator = 0;

    for (str_iterator = 0; str_iterator < INPUT_LENGTH; str_iterator++)
    {
        if (is_search_started == 1)
        {
            //Matched again, continue checking
            if (str[str_iterator] == katar[katar_iterator])
            {
                katar_iterator++;

                // Searching successfully completed, return the index of the start index of the katar
                if (katar_iterator == KATAR_LENGTH)
                {
                    return last_index;
                }
            }
            else
            {
                //Does not match, stop searching, set str_iterator to last_index+1
                str_iterator = last_index + 1;
                //Reset bool value
                is_search_started = 0;
                //Reset katar iterator
                katar_iterator = 0;
            }
        }
        else if (str[str_iterator] == katar[katar_iterator])
        {
            //If there isn't a search and find a match, start searching
            is_search_started = 1;
            katar_iterator++;
            last_index = str_iterator;
        }
        else if (str_iterator >= (INPUT_LENGTH - KATAR_LENGTH - 1))
        {
            //There is no need to continue for searching
            //Because the number of the rest of the characters is less than size of katar
            break;
        }
    }

    // -1 means that we have no match
    return -1;
}

char *substring(char *str, int start_index, int length)
{
    int i;
    char *new_string = (char *)calloc(length, sizeof(char));

    //Init the string
    for (i = 0; i < length; i++)
        new_string[i] = '\0';

    for (i = 0; i < length; i++)
    {
        // There is no characters in the string
        if (i + start_index >= INPUT_LENGTH)
            break;

        new_string[i] = str[i + start_index];
    }

    return new_string;
}

void partition_input(char *input, char *katar, char *local_input, int p, int my_rank)
{   
    MPI_Status status;
    int i;
    int size_per_proc = (int)INPUT_LENGTH / p;
    int partition_size = size_per_proc + KATAR_LENGTH - 1;
    char *partition, *buf;

    if(my_rank == 0)
    {
        printf("Size Per Process: %d | Partition Size: %d\n",size_per_proc,partition_size);
        for(i=0; i<p; i++)
        {
            partition = substring(input, i * size_per_proc, partition_size);
            if(i == 0)
            {
                strcpy(local_input,partition);
            }
            else
            {
                MPI_Send(partition,partition_size,MPI_CHAR,i,0,MPI_COMM_WORLD);
            }
            printf("PARTITION %d: %s\n",i+1,partition);
        }
    }
    else
    {
        MPI_Recv(local_input,partition_size,MPI_CHAR,0,0,MPI_COMM_WORLD,&status);
    }
    
}