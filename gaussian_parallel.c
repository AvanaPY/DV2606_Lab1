/***************************************************************************
 *
 * Sequential version of Gaussian elimination
 *
 ***************************************************************************/

#include <stdio.h>
#include <pthread.h>

#include <stdatomic.h>

#define MAX_SIZE 4096
#define MAX_THREADS 32


typedef double matrix[MAX_SIZE][MAX_SIZE];

int	N;		/* matrix size		*/
int	maxnum;		/* max number of element*/
char	*Init;		/* matrix init type	*/
int	PRINT;		/* print switch		*/
matrix	A;		/* matrix A		*/
double	b[MAX_SIZE];	/* vector b             */
double	y[MAX_SIZE];	/* vector y             */

/* forward declarations */
void work(void);
void Init_Matrix(void);
void Print_Matrix(void);
void Init_Default(void);
int Read_Options(int, char **);

int
main(int argc, char **argv)
{
    int i, timestart, timeend, iter;

    Init_Default();		/* Init default values	*/
    Read_Options(argc,argv);	/* Read arguments	*/
    Init_Matrix();		/* Init the matrix	*/
    work();
    if (PRINT == 1)
	   Print_Matrix();
}

typedef struct threadData {
    int row;
    atomic_int *finished;
} threadData_t;

/*
    This function is meant to be called from a thread.

    It does gaussian elimination on one row and takes in a threadData_t type as a parameter.
    It then sets the finished flag inside the threadData_t type to 1 to indicate that the thread can
    once again be used to compute another row. 
*/
void* myThreadFunction(void *ptargs)
{
    threadData_t *td = (threadData_t*)ptargs;
    int row = td->row;

    int k = row;
    for (int j = k+1; j < N; j++)
        A[k][j] = A[k][j] / A[k][k]; /* Division step */
    y[k] = b[k] / A[k][k];
    A[k][k] = 1.0;
    for (int i = k+1; i < N; i++) {
        for (int j = k+1; j < N; j++)
            A[i][j] = A[i][j] - A[i][k]*A[k][j]; /* Elimination step */
        b[i] = b[i] - A[i][k]*y[k];
        A[i][k] = 0.0;
    }
    
    *(td->finished) = 1;
}

/*
    In order to multi-thread the program, we settled on the simple solution of simply delegating
    each row in the matrix to a thread which evaluates the gaussian elimination for said row.
*/
void
work(void)
{
    pthread_t thread_pool[MAX_THREADS];
    atomic_int thread_finished[MAX_THREADS];
    threadData_t thread_data[N];

    // Initialize data
    for(int i = 0; i < MAX_THREADS; i++)
        thread_finished[i] = 1;
    for(int i = 0; i < N; i++)
        thread_data[i].row = i;
    

    /* 
        Delgating each row to a new thread.

        We iterate over each row, finding an available thread from our thread pool which to delegate the row to.
    */
    int t = 0;
    for (int found_thread = 0, k = 0; k < N; k++) { /* Outer loop */
	    // Delegate 
        found_thread = 0;
        while(found_thread == 0)
        {
            for(t = 0; t < MAX_THREADS; t++)
                if(thread_finished[t] == 1)
                {
                    // printf("Starting thread %d on row %d\n", t, k);
                    thread_finished[t] = 0;
                    thread_data[k].finished = thread_finished + t;

                    pthread_create(thread_pool + t, NULL, myThreadFunction, thread_data + k);
                    found_thread = 1;
                    break;
                }
            // t = (t + 1) % MAX_THREADS;
        }
    }

    // Wait for all threads to complete before exiting
    for(int i = 0; i < MAX_THREADS; i++)
        pthread_join(thread_pool[i], NULL);
}

void
Init_Matrix()
{
    int i, j;

    printf("\nsize      = %dx%d ", N, N);
    printf("\nmaxnum    = %d \n", maxnum);
    printf("Init	  = %s \n", Init);
    printf("Initializing matrix...");

    if (strcmp(Init,"rand") == 0) {
        for (i = 0; i < N; i++){
            for (j = 0; j < N; j++) {
                if (i == j) /* diagonal dominance */
                    A[i][j] = (double)(rand() % maxnum) + 5.0;
                else
                    A[i][j] = (double)(rand() % maxnum) + 1.0;
            }
        }
    }
    if (strcmp(Init,"fast") == 0) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                if (i == j) /* diagonal dominance */
                    A[i][j] = 5.0;
                else
                    A[i][j] = 2.0;
            }
        }
    }

    /* Initialize vectors b and y */
    for (i = 0; i < N; i++) {
        b[i] = 2.0;
        y[i] = 1.0;
    }

    printf("done \n\n");
    if (PRINT == 1)
        Print_Matrix();
}

void
Print_Matrix()
{
    int i, j;

    printf("Matrix A:\n");
    for (i = 0; i < N; i++) {
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

void
Init_Default()
{
    N = 2048;
    Init = "rand";
    maxnum = 15.0;
    PRINT = 0;
}

int
Read_Options(int argc, char **argv)
{
    char    *prog;

    prog = *argv;
    while (++argv, --argc > 0)
        if (**argv == '-')
            switch ( *++*argv ) {
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
                    printf("\n          Init      = rand" );
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
