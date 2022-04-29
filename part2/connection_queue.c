#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "connection_queue.h"

int connection_queue_init(connection_queue_t *queue) {
    // TODO Not yet implemented
    // allocate memory
    // initialize syncrhonization primitives
    // mutexes and condition variables
    memset(&queue->client_fds, 0, sizeof(int)*CAPACITY);
    queue->length = CAPACITY;
    queue->read_idx = 0; 
    queue->write_idx = 0;
    queue->shutdown = 1; // change to 0 for shutdown
    int result;
    if ((result = pthread_mutex_init(&queue->lock, NULL)) != 0) {
        fprintf(stderr, "pthread_mutex_init: %s\n", strerror(result));
        return -1;
    }
    if ((result = pthread_cond_init(&queue->queue_full, NULL)) != 0) {
        fprintf(stderr, "pthread_cond_init: %s\n", strerror(result));
        return -1;
    }
    if ((result = pthread_cond_init(&queue->queue_empty, NULL)) != 0) {
        fprintf(stderr, "pthread_cond_init: %s\n", strerror(result));
        return -1;
    }
    return 0;
}

int connection_enqueue(connection_queue_t *queue, int connection_fd) {
    // TODO Not yet implemented
    int result;
    if ((result = pthread_mutex_lock(&queue->lock)) != 0) {
        fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(result));
        return -1;
    }
    while (queue->read_idx == (queue->write_idx+1) % queue->length) {
        printf("yurp\n");
        if ((result = pthread_cond_wait(&queue->queue_full, &queue->lock)) != 0) {
            fprintf(stderr, "pthread_cond_wait: %s\n", strerror(result));
            return -1;
        }
    }

    if (queue->shutdown == 0) {
        printf("enqueue successful\n");
        if ((result = pthread_mutex_unlock(&queue->lock)) != 0) {
            fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(result));
            return -1;
        }
        return -1;
    } else {
        queue->client_fds[queue->write_idx] = connection_fd;
        queue->write_idx = (queue->write_idx + 1) % queue->length;
        if ((result = pthread_cond_signal(&queue->queue_empty)) != 0) {
            fprintf(stderr, "pthread_cond_signal: %s\n", strerror(result));
            pthread_mutex_unlock(&queue->lock);
            return -1;
        }
    }

    if ((result = pthread_mutex_unlock(&queue->lock)) != 0) {
        fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(result));
        return -1;
    }

    return 0;
}

int connection_dequeue(connection_queue_t *queue) {
    int result;
    if ((result = pthread_mutex_lock(&queue->lock)) != 0) {
        fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(result));
        return -1;
    }
    while (queue->write_idx == queue->read_idx) {
        if ((result = pthread_cond_wait(&queue->queue_empty, &queue->lock)) != 0) {
            fprintf(stderr, "pthread_cond_wait: %s\n", strerror(result));
            return -1;
        }
    }

    if (queue->shutdown == 0) {
        printf("dequeue successful\n");
        if ((result = pthread_mutex_unlock(&queue->lock)) != 0) {
            fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(result));
            return -1;
        }
        return -1;
    } else {
        int temp = queue->client_fds[queue->read_idx];
        queue->read_idx = (queue->read_idx + 1) % queue->length;
        if ((result = pthread_cond_signal(&queue->queue_full)) != 0) {
            fprintf(stderr, "pthread_cond_signal: %s\n", strerror(result));
            pthread_mutex_unlock(&queue->lock);
            return -1;
        }
        if ((result = pthread_mutex_unlock(&queue->lock)) != 0) {
            fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(result));
            return -1;
        }
        return temp;
    }    
    return 0;
}

int connection_queue_shutdown(connection_queue_t *queue) {
    int result;

    
    if ((result = pthread_mutex_lock(&queue->lock)) != 0) {
        fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(result));
        return -1;
    }
    queue->shutdown = 0;
    if ((result = pthread_cond_broadcast(&queue->queue_empty)) != 0) {
        fprintf(stderr, "pthread_cond_signal: %s\n", strerror(result));
        pthread_mutex_unlock(&queue->lock);
        return -1;
    }

    if ((result = pthread_cond_broadcast(&queue->queue_full)) != 0) {
        fprintf(stderr, "pthread_cond_signal: %s\n", strerror(result));
        pthread_mutex_unlock(&queue->lock);
        return -1;
    }

    if ((result = pthread_mutex_unlock(&queue->lock)) != 0) {
        fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(result));
        return -1;
    }
    if ((result = connection_queue_free(queue)) == -1) {
        fprintf(stderr, "failed to free queue\n");
        return 1;
    }

       
    return 0;
}

int connection_queue_free(connection_queue_t *queue) {

    int result;
    if((result = pthread_mutex_destroy(&queue->lock)) == -1){
        fprintf(stderr, "pthread_mutex_destroy(lock): %s\n", strerror(result));
        return -1;
    }
    if((result = pthread_cond_destroy(&queue->queue_full)) == -1){
        fprintf(stderr, "pthread_cond_destroy(full): %s\n", strerror(result));
        return -1;
    }
    //works when we have this commented, but I know that we need to destroy this condition variable 
    // if((result = pthread_cond_destroy(&queue->queue_empty)) == -1){
    //     fprintf(stderr, "pthread_cond_destroy(empty): %s\n", strerror(result));
    //     return -1;
    // }
        
    return 0;
}
