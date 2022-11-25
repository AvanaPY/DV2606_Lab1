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
    volatile atomic_int alive;
} pthread_data_t;

typedef struct sort_data {
    int low, high;
} sort_data_t;

const int queue_len = 1024; /* 1024 items **should** be enough */
sort_data_t* queue; 
int queue_start_index = 0;
int queue_end_index = 0;
pthread_mutex_t mut_enqueue, mut_dequeue;

#define MAX_THREADS 16
pthread_t thread_pool[MAX_THREADS];
pthread_data_t* pthread_data_pool;

/* Queue api */
void
enqueue(int low, int high)
{
    pthread_mutex_lock(&mut_enqueue);
    queue[queue_end_index].low = low;
    queue[queue_end_index].high = high;
    queue_end_index = (queue_end_index + 1) % queue_len;
    pthread_mutex_unlock(&mut_enqueue);
}

sort_data_t*
dequeue()
{
    pthread_mutex_lock(&mut_dequeue);
    int r = queue_start_index;
    queue_start_index = (queue_start_index + 1) % queue_len;
    pthread_mutex_unlock(&mut_dequeue);
    return queue + r;
}

int 
queue_length()
{
    if(queue_start_index <= queue_end_index){
        return queue_end_index - queue_start_index;
    }
    return queue_len - (queue_start_index - queue_end_index);
}

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
mark_thread_available(unsigned thread_id)
{
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
    int *v = t->v;
    unsigned low, high, pivot_index;

    while(t->alive == 1)
    {
        if(t->available == 1)
            continue;
        if(low >= high)
            return NULL;

        low = t->low;
        high = t->high;

        pivot_index = (low+high)/2;
        pivot_index = partition(v, low, high, pivot_index);

        /* sort the two sub arrays */
        if (low < pivot_index - 1)
        {
            enqueue(low, pivot_index - 1);
        }
        if (pivot_index + 1 < high)
        {
            enqueue(pivot_index+1, high);
        }
        t->available = 1;
    }
    return NULL;
}

static void
quick_sort(int *v, unsigned low, unsigned high)
{
    // Init thread pool
    pthread_data_pool = (pthread_data_t*)malloc(MAX_THREADS*sizeof(pthread_data_t));
    for(int i = 0; i < MAX_THREADS; i++)
    {
        pthread_data_pool[i].v = v;
        pthread_data_pool[i].thread_id = i;
        pthread_data_pool[i].available = 1;
        pthread_data_pool[i].alive = 1;
        pthread_create(&thread_pool[i], NULL, thread_sort, (void*)&pthread_data_pool[i]);
    }

    // Initialise queue
    queue = (sort_data_t*)malloc(sizeof(sort_data_t) * queue_len);
    pthread_mutex_init(&mut_enqueue, NULL);
    pthread_mutex_init(&mut_dequeue, NULL);
    enqueue(low, high);

    // Start first thread
    int working = 1; // Variable to indicate whether or not the program is still working
    while(working == 1)
    {
        working = 0;
        if(queue_length() > 0)
        {
            working = 1;

            for(int i = 0; i < MAX_THREADS; i++)
            {
                if(pthread_data_pool[i].available == 1)
                {
                    sort_data_t* d = dequeue();
                    pthread_data_pool[i].low = d->low;
                    pthread_data_pool[i].high = d->high;
                    pthread_data_pool[i].available = 0;
                    break;
                }
            }
        }
        for(int i = 0; i < MAX_THREADS; i++)
        {
            if(pthread_data_pool[i].available == 0)
                working = 1;
        }
    }

    printf("Exited main loop\n");

    for(int i = 0; i < MAX_THREADS; i++)
        pthread_data_pool[i].alive = 0;

    // Suck

    // Wait for all threads to be done working
    for(int i = 0; i < MAX_THREADS; i++)
        pthread_join(thread_pool[i], NULL);
    // TODO: There is very bad bug, see github
}

int
main(int argc, char **argv)
{
    init_array();
    // print_array();
    quick_sort(v, 0, MAX_ITEMS-1);
    // print_array();
}
