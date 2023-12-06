#include <iostream>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    pthread_cond_t cond1;
    pthread_mutex_t lock;
    int data;
    int ready;
} for_mutex;

for_mutex global_data = {
        .cond1 = PTHREAD_COND_INITIALIZER,
        .lock = PTHREAD_MUTEX_INITIALIZER,
        .data = 0,
        .ready = 0,
};

int consumer_handler(for_mutex *channel) {
    pthread_mutex_lock(&channel->lock);
    while (channel->ready == 0) {
        pthread_cond_wait(&channel->cond1, &channel->lock);
    }
    pthread_mutex_unlock(&channel->lock);
    int data = channel->data;
    channel->ready = 0;
    return data;
}

void* consumer(void*) {

    while (true) {
        int data = consumer_handler(&global_data);
        if (data == 14) {
            break;
        }
        std::cout<<"Consumed "<<data<<std::endl;
    }
    return nullptr;
}

void provide_handler(for_mutex *channel, int data) {
    while (true) {
        pthread_mutex_lock(&channel->lock);
        if (channel->ready == 0) {
            break;
        }
        pthread_mutex_unlock(&channel->lock);
    }
    pthread_cond_signal(&channel->cond1);
    pthread_mutex_unlock(&channel->lock);
    channel->data = data;
    channel->ready = 1;
}

void* provider(void *) {
    for (int i = 1; i < 15; i++) {
        std::cout<<"Provided "<<i<<std::endl;
        provide_handler(&global_data, i);
        sleep(1); // delay 1sec
    }
    return nullptr;
}

int main() {
    pthread_t thread_provider;
    pthread_t thread_consumer;
    int provider_res = pthread_create(&thread_provider, nullptr, provider, nullptr);
    if (provider_res != 0) {
        std::cout<<"pthread_create fail: "<<provider_res<<std::endl;
        return provider_res;
    }
    int consumer_res = pthread_create(&thread_consumer, nullptr, consumer, nullptr);
    if (provider_res != 0) {
        std::cout<<"pthread_create failed: "<<consumer_res<<std::endl;
        return consumer_res;
    }
    pthread_join(thread_provider, nullptr);
    pthread_join(thread_consumer, nullptr);

    return EXIT_SUCCESS;
}
