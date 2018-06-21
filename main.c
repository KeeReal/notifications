#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <zconf.h>
#include <inttypes.h>

typedef struct config {
    int timer_fd;
    int timeout_ms;
    int message;
} config_t;

typedef struct node {
    struct node *next;
    config_t *config;
} node_t;

typedef struct root {
    node_t *head;
    int epoll_fd;
} root_t;

config_t* create_config(int timeout_ms, int message) {
    config_t *config = malloc(sizeof(config_t));
    assert(config != NULL);

    config->message = message;
    config->timeout_ms = timeout_ms;
    return config;
}

int initialize_timer(config_t *config) {
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timer_fd == -1) {
        perror("Failed to create timer");
        return EXIT_FAILURE;
    }
    printf("Timer#%d created\n", timer_fd);
    config->timer_fd = timer_fd;

    __syscall_slong_t nsec = config->timeout_ms * 1000000;

    struct itimerspec spec;
    spec.it_value.tv_sec = 0;
    spec.it_value.tv_nsec = nsec;
    spec.it_interval.tv_sec = 0;
    spec.it_interval.tv_nsec = nsec;

    if (timerfd_settime(timer_fd, 0, &spec, NULL) < 0) {
        perror("Failed to arm timer");
        return EXIT_FAILURE;
    }
    printf("Timer#%d armed\n", timer_fd);
    return EXIT_SUCCESS;
}


void push_config(root_t *root, config_t *config) {
    node_t *prev = root->head;

    node_t *node = malloc(sizeof(node_t));
    node->next = prev;
    node->config = config;

    root->head = node;
}

void print_config(config_t *config) {
    printf("Timer#%d message '%d' with timeout: %d\n",
           config->timer_fd,
           config->message,
           config->timeout_ms);
}

void print_configs(root_t *root) {
    node_t *node = root->head;
    while (node != NULL) {
        print_config(node->config);
        node = node->next;
    }
}

int init_epoll(root_t *root) {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Failed to create epoll instance");
        return EXIT_FAILURE;
    }

    printf("Epoll#%d instance created\n", epoll_fd);

    root->epoll_fd = epoll_fd;
    return EXIT_SUCCESS;
}

int push_to_epoll_set(root_t *root, config_t * config) {
    struct epoll_event ev;
    ev.data.ptr = config;
    ev.events = EPOLLIN;

    if (epoll_ctl(root->epoll_fd,
                  EPOLL_CTL_ADD,
                  config->timer_fd,
                  &ev) == -1) {
        perror("Failed to add timer to epoll set");
        return EXIT_FAILURE;
    }
    printf("Timer#%d added to epoll#%d\n",
           config->timer_fd,
           root->epoll_fd);
    return EXIT_SUCCESS;
}

int main() {
    node_t *head = NULL;
    root_t root;

    root.head = head;

    config_t *config = NULL;

    if (init_epoll(&root) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    config = create_config(100, 100);
    if (initialize_timer(config) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    push_config(&root, config);
    push_to_epoll_set(&root, config);

    config = create_config(800, 800);
    if (initialize_timer(config) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    push_config(&root, config);
    push_to_epoll_set(&root, config);

    print_configs(&root);

    int running = 1;
    int events_count;
    struct epoll_event ev;
    uint64_t timer_result;

    while (running) {
        events_count = epoll_wait(root.epoll_fd, &ev, 1, 100);
        if (events_count <= 0) {
            continue;
        }

        config = (config_t *) ev.data.ptr;

        int r = (int) read(config->timer_fd,
                           &timer_result,
                           sizeof(uint64_t));
        printf("Event! read result: %d, timer_result: %" PRIu64 "\n", r, timer_result);
        print_config(config);
    }

    // todo: wait until timers fired

    // create timers
    // add to epoll set
    // and wait
    // when timer event happened
    // call notification function with id of timer (by timer_fd i can find notification)
    // event ptr -> points to nconfig_t
    // create timer with specific delay and specific message (will be int)



    return 0;
}