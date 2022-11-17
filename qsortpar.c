#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#define KILO (1024)
#define MEGA (1024*1024)
#define MAX_ITEMS (64*MEGA)
#define swap(v, a, b) {unsigned tmp; tmp=v[a]; v[a]=v[b]; v[b]=tmp;}

static int *v;

typedef struct pthread_data_t {
    int *v;
    unsigned low, high;
    unsigned thread_id;
    volatile atomic_int available;
    volatile atomic_int mark_available;
} pthread_data_t;

#define MAX_THREADS 16
#define MIN_SUBARRAY_SORT_SIZE 1024 * 4
pthread_t thread_pool[MAX_THREADS];
pthread_data_t* pthread_data_pool;

static int start_thread_qsort(int *v, unsigned low, unsigned high, int force);

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
        v[i] = rand() % MAX_ITEMS;
}

static void 
init_pthread_data_pool(void)
{
    pthread_data_pool = (pthread_data_t*)malloc(MAX_THREADS*sizeof(pthread_data_t));
    for(int i = 0; i < MAX_THREADS; i++)
    {
        pthread_data_pool[i].thread_id = i;
        pthread_data_pool[i].available = 1;
        pthread_data_pool[i].mark_available = 1;
    }
}

static void
mark_thread_available(unsigned thread_id)
{
    printf("Marked %d as available\n", thread_id);
    pthread_data_pool[thread_id].available = 1;
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
    int mark = t->mark_available;
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
        int thread_id = start_thread_qsort(v, low, pivot_index-1, 0); 
        if(thread_id == -1){
            
            t->available = 0;
            t->low = low;
            t->high = pivot_index-1;
            t->mark_available = 0;
            thread_sort((void*)t);
        }
    }
    if (pivot_index < high)
    {
        t->low = pivot_index+1;
        t->high = high;
        t->mark_available = 0;
        thread_sort((void*)t);
    }
    if(mark == 1)
        mark_thread_available(t->thread_id);
    return NULL;
}
/*
    Starts a thread to sort from low to high in the array v
*/
static int
start_thread_qsort(int *v, unsigned low, unsigned high, int force)
{
    // If the size of the subarray is smaller than a given size it is more efficient to instead simply continue to process the remaining
    // subarrays on the remaining threads than to start up more threads as it introduces lots of overhead
    if(force == 0 && high - low < MIN_SUBARRAY_SORT_SIZE){
        return -1;
    }

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
        t->mark_available = 1;

        pthread_create(thread_pool + thread_id, NULL, thread_sort, (void*)(t));
    }
    return thread_id;
}

static void
quick_sort(int *v, unsigned low, unsigned high)
{
    // Init thread pool

    // Start first thread
    start_thread_qsort(v, low, high, 1);

    // Suck

    // Wait for all threads to be done working
    int finished = 0;
    while(finished == 0) {
        finished = 1;
        for(int i = 0; i < MAX_THREADS; i++)
            finished &= pthread_data_pool[i].available;
    }
    // TODO: There is very bad bug, see github
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
