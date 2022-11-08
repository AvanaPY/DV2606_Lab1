/***************************************************************************
 *
 * Not-Sequential version of Quick sort
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>

typedef struct pthread_data_t {
    int *v;
    unsigned low, high;
    unsigned thread_id;
    atomic_int available;
} pthread_data_t;


#define MAX_THREADS 2

#define KILO (1024)
#define MEGA (1024*1024)
#define MAX_ITEMS (64*MEGA)
#define swap(v, a, b) {unsigned tmp; tmp=v[a]; v[a]=v[b]; v[b]=tmp;}

static int *v;

pthread_t thread_pool[MAX_THREADS];
pthread_data_t* pthread_data_pool;

static int
start_thread_qsort(int *v, unsigned low, unsigned high);

static void
print_array(void)
{
    int i;
    for (i = 0; i < MAX_ITEMS; i++)
        printf("%d ", v[i]);
    printf("\n");
}

static void
init_array(void)
{
    int i;
    v = (int *) malloc(MAX_ITEMS*sizeof(int));
    for (i = 0; i < MAX_ITEMS; i++)
        v[i] = rand();
}

static void 
init_pthread_data_pool(void)
{
    pthread_data_pool = (pthread_data_t*)malloc(MAX_THREADS*sizeof(pthread_data_t));
    for(int i = 0; i < MAX_THREADS; i++)
    {
        pthread_data_pool[i].thread_id = i;
        pthread_data_pool[i].available = 1;
    }
}

static unsigned
partition(int *v, unsigned low, unsigned high, unsigned pivot_index)
{
    /* move pivot to the bottom of the vector */
    if (pivot_index != low)
        swap(v, low, pivot_index);

    pivot_index = low;
    low++;

    /* invariant:
     * v[i] for i less than low are less than or equal to pivot
     * v[i] for i greater than high are greater than pivot
     */

    /* move elements into place */
    while (low <= high) {
        if (v[low] <= v[pivot_index])
            low++;
        else if (v[high] > v[pivot_index])
            high--;
        else
            swap(v, low, high);
    }

    /* put pivot back between two groups */
    if (high != pivot_index)
        swap(v, pivot_index, high);
    return high;
}
static void*
thread_sort(void* ptargs)
{
    pthread_data_t* t = (pthread_data_t*)ptargs;

    int *v = t->v;
    unsigned low = t->low, high = t->high;
    unsigned pivot_index;

    if(low >= high)
        return NULL;

    pivot_index = (low+high)/2;
    pivot_index = partition(v, low, high, pivot_index);
    
    /* sort the two sub arrays */
    if (low < pivot_index)
    {
        int thread_id = start_thread_qsort(v, low, pivot_index-1); 
        if(thread_id == -1){
            t->low = low;
            t->high = pivot_index-1;
            thread_sort((void*)t);
        }
    }
    if (pivot_index < high)
    {
        //int thread_id = start_thread_qsort(v, pivot_index+1, high); // TODO: Change so we continue this computation on this thread instead
        t->low = pivot_index+1;
        t->high = high;
        thread_sort((void*)t);
    }
    t->available = 1;
}
/*
    Starts a thread to sort from low to high in the array v
*/
static int
start_thread_qsort(int *v, unsigned low, unsigned high)
{
    int thread_id = -1;
    for(int i = 0; i < MAX_THREADS; i++)
    {
        if(pthread_data_pool[i].available == 1){
            thread_id = i;
            break;
        }
    }
    if(thread_id != -1){
        pthread_data_t *t = pthread_data_pool + thread_id;
        t->available = 0;
        t->v = v;
        t->low = low;
        t->high = high;
        printf("Starting thread... %d\n", thread_id);
        printf("    with data low=%d high=%d\n", t->low, t->high);

        pthread_create(thread_pool + thread_id, NULL, thread_sort, (void*)(t));
    }
    return thread_id;
}

static void
quick_sort(int *v, unsigned low, unsigned high)
{
    // Init thread pool

    // Start first thread
    start_thread_qsort(v, low, high);

    // Suck

    for(int i = 0; i < MAX_THREADS; i++)
        pthread_join(thread_pool[i], NULL);
}

int
main(int argc, char **argv)
{
    init_array();
    init_pthread_data_pool();
    //print_array();
    quick_sort(v, 0, MAX_ITEMS-1);
    //print_array();
}
