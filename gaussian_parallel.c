/***************************************************************************
 *
 *  Parallel version of Gaussian elimination
 *  Written by Emil Karlström &
               Samuel Jonsson

            Blekinge Institute of Technology
            DVAMI19h
            DV2606
 *
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#define MAX_SIZE 4096
#define MIN(x,y) (x < y ? x : y)
typedef double matrix[MAX_SIZE][MAX_SIZE];

int N;              /* matrix size		*/
int maxnum;         /* max number of element*/
char *Init;         /* matrix init type	*/
int PRINT;          /* print switch		*/
matrix A;           /* matrix A		*/
double b[MAX_SIZE]; /* vector b             */
double y[MAX_SIZE]; /* vector y             */

/* forward declarations */
void work(void);
void Init_Matrix(void);
void Print_Matrix(void);
void Init_Default(void);
int Read_Options(int, char **);

/* Thread declarations */
void* gaussian_elimination(void* args);
void* back_substitution(void* args);
void* normalize_row(void* args);

/* Global thread variables */
pthread_t *thread_pool;     // Our pool of threads to use
pthread_barrier_t barrier;  // A barrier to synchronize threads with
const int thread_count = 16;      // Amount of threads to use
int* thread_id;

int main(int argc, char **argv)
{
    int i, timestart, timeend, iter;

    /*--------------------------------------------------------*/
    /* ---------- Initiate thread pool and barrier ---------- */
    thread_pool = (pthread_t*)malloc(sizeof(pthread_t)*thread_count);
    pthread_barrier_init(&barrier, NULL, thread_count);

    thread_id = (int*)malloc(sizeof(int) * thread_count);
    for(int i = 0; i < thread_count; i++)
        thread_id[i] = i;

    /*--------------------------------------------------------*/

    Init_Default();           /* Init default values	*/
    Read_Options(argc, argv); /* Read arguments	*/
    Init_Matrix();            /* Init the matrix	*/
    work();
    if (PRINT == 1)
        Print_Matrix();

    /* Free stuff */
    free(thread_pool);
    free(thread_id);
}

void *gaussian_elimination(void *args)
{
    int i,j,l;                            /* Indices */
    int thread_index = *(int*)args;     
    int num, start, end;                /* Start and ends at*/
    float r;                              /* the division ratio we are working with */

    for(l = 0; l < N; l++)
    { 
        num = N - l;
        start = l + 1 + thread_index * num / thread_count;
        end = l + 1 + (thread_index + 1) * num / thread_count;  
        /* This delegates all the rows as evenly as possible among the threads */
        for(i = start; i < end; i++)
        {
            r = A[i][l] / A[l][l];          /* Division ratio between pivot and row to eliminate from  */
            for(j = l + 1; j < N; j++)      /* For every column... */
                A[i][j] -= r * A[l][j];     /* Elimination */
            b[i] -= r * b[l]; 
            A[i][l] = 0;
        }

        pthread_barrier_wait(&barrier);     /* Because every thread uses its own loop for the pivot cell
                                               we will use a barrier to synchronize those loops*/
    }
    pthread_exit(0);                    /* Exit thread with code 0 */
    return NULL;
}

void* normalize_row(void* args)
{
    int i, j;
    int thread_index = *(int*)args;;

    int start = thread_index * N / thread_count;
    int end   = (thread_index + 1) * N / thread_count;

    for(i = start; i < end; i++)
        for(j = N - 1; j >= i; j--)
            A[i][j] = A[i][j] / A[i][i];

    pthread_exit(0);                    /* Exit thread with code 0 */
    pthread_barrier_wait(&barrier);     /* Sync threads */
    return NULL;
}

/*
    
*/
void work(void)
{
    int k,l;
    /* Start the gaussian elimination process on each thread */ 
    for(k = 0; k < thread_count; k++)
        pthread_create(&thread_pool[k], NULL, gaussian_elimination, (void*)&thread_id[k]);

    for(k = 0; k < thread_count; k++)
        pthread_join(thread_pool[k], NULL);

    // Evaluate Y
    for(l = 0; l < N; l++)/* Evaluate Y vector*/
        y[l] = b[l] / A[l][l];

    /* Update the matrix A with the normalized rows */
    for(k = 0; k < thread_count; k++)
        pthread_create(&thread_pool[k], NULL, normalize_row, (void*)&thread_id[k]);
    
    for(k = 0; k < thread_count; k++)
        pthread_join(thread_pool[k],NULL);
}

void Init_Matrix()
{
    int i, j;

    printf("\nsize      = %dx%d ", N, N);
    printf("\nmaxnum    = %d \n", maxnum);
    printf("Init	  = %s \n", Init);
    printf("Initializing matrix...");

    if (strcmp(Init, "rand") == 0)
    {
        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N; j++)
            {
                if (i == j) /* diagonal dominance */
                    A[i][j] = (double)(rand() % maxnum) + 5.0;
                else
                    A[i][j] = (double)(rand() % maxnum) + 1.0;
            }
        }
    }
    if (strcmp(Init, "fast") == 0)
    {
        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N; j++)
            {
                if (i == j) /* diagonal dominance */
                    A[i][j] = 5.0;
                else
                    A[i][j] = 2.0;
            }
        }
    }

    /* Initialize vectors b and y */
    for (i = 0; i < N; i++)
    {
        b[i] = 2.0;
        y[i] = 1.0;
    }

    printf("done \n\n");
    if (PRINT == 1)
        Print_Matrix();
}

void Print_Matrix()
{
    int i, j;

    printf("Matrix A:\n");
    for (i = 0; i < N; i++)
    {
        printf("[");
        for (j = 0; j < N; j++)
            printf(" %5.2f,", A[i][j]);
        printf("]\n");
    }
    printf("Vector b:\n[");
    for (j = 0; j < N; j++)
        printf(" %5.2f,", b[j]);
    printf("]\n");
    printf("Vector y:\n[");
    for (j = 0; j < N; j++)
        printf(" %5.2f,", y[j]);
    printf("]\n");
    printf("\n\n");
}

void Init_Default()
{
    N = 2048;
    Init = "rand";
    maxnum = 15.0;
    PRINT = 0;
}

int Read_Options(int argc, char **argv)
{
    char *prog;

    prog = *argv;
    while (++argv, --argc > 0)
        if (**argv == '-')
            switch (*++*argv)
            {
            case 'n':
                --argc;
                N = atoi(*++argv);
                break;
            case 'h':
                printf("\nHELP: try sor -u \n\n");
                exit(0);
                break;
            case 'u':
                printf("\nUsage: gaussian [-n problemsize]\n");
                printf("           [-D] show default values \n");
                printf("           [-h] help \n");
                printf("           [-I init_type] fast/rand \n");
                printf("           [-m maxnum] max random no \n");
                printf("           [-P print_switch] 0/1 \n");
                exit(0);
                break;
            case 'D':
                printf("\nDefault:  n         = %d ", N);
                printf("\n          Init      = rand");
                printf("\n          maxnum    = 5 ");
                printf("\n          P         = 0 \n\n");
                exit(0);
                break;
            case 'I':
                --argc;
                Init = *++argv;
                break;
            case 'm':
                --argc;
                maxnum = atoi(*++argv);
                break;
            case 'P':
                --argc;
                PRINT = atoi(*++argv);
                break;
            default:
                printf("%s: ignored option: -%s\n", prog, *argv);
                printf("HELP: try %s -u \n\n", prog);
                break;
            }
}
