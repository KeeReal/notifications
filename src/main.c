#include "main.h"

int main(int argc, char **argv) {
    if (notify_init(NOTIFICATIONS_APP_NAME)) {
        fprintf(stdout, "libnotifiy successfully initialized\n");
    }

    fprintf(stdout, "Strted with pid#%d\n", getpid());
    root.shutdown = 0;
    root.head = NULL;
    root.notification = NULL;

    root_setup_signals();

    parser_readconfig(argc == 2 ? argv[1] : NULL);
    print_configs();

    root_init_epoll();

    node_t *node = root.head;
    while (node != NULL) {
        root_init_timer(node->config);
        root_add_timer(node->config);
        node = node->next;
    }

    int events_count;
    struct epoll_event ev;
    uint64_t timer_result;

    while (!root.shutdown) {
        events_count = epoll_wait(root.epoll_fd, &ev, 1, 100);
        if (events_count <= 0) {
            continue;
        }

        config_t *config = (config_t*) ev.data.ptr;
        read(config->timer_fd, &timer_result, sizeof(uint64_t));

        notification_close();
        notification_show(config);
    }

    return EXIT_SUCCESS;
}

void root_setup_signals(void) {
    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = root_signal_handler;
    sigaction(SIGUSR1, &act, NULL);
//    sigaction(SIGINT, &act, NULL);
}

void root_signal_handler(int sig) {
    char *msg = NULL;

    switch (sig) {

        case SIGINT:
            msg = "Received SIGINT, stop all...";
            break;

        case SIGTERM:
            msg = "Received SIGTERM, stop all...";
            break;

        default:
            msg = "Received signal, stop all ...";
            break;
    }

    if (root.shutdown && sig == SIGINT) {
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "%s\n", msg);
    root.shutdown = 1;
    root_stop();
}

void root_stop() {
    fprintf(stdout, "Stop all\n");
    // todo: free all

    if (root.notification != NULL) {
        g_object_unref(G_OBJECT(root.notification));
    }
    notify_uninit();
}


void root_init_epoll() {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Failed to create epoll instance");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Epoll#%d instance created\n", epoll_fd);
    root.epoll_fd = epoll_fd;
};

void root_init_timer(config_t *config) {
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timer_fd == -1) {
        perror("Failed to create time");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Timer#%d created\n", timer_fd);
    config->timer_fd = timer_fd;

    struct itimerspec spec;
    spec.it_value.tv_nsec = 0;
    spec.it_value.tv_sec = config->timeout_s;

    spec.it_interval.tv_nsec = 0;
    spec.it_interval.tv_sec = config->timeout_s;

    if (timerfd_settime(timer_fd, 0, &spec, NULL) < 0) {
        perror("Failed to arm timer");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Timer#%d armed\n", timer_fd);
};

void root_add_timer(config_t *config) {
    struct epoll_event ev;
    ev.data.ptr = config;
    ev.events = EPOLLIN;

    if (epoll_ctl(root.epoll_fd, EPOLL_CTL_ADD, config->timer_fd, &ev) == -1) {
        perror("Failed to add timer to epoll set");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Timer#%d added to epoll#%d\n", config->timer_fd, root.epoll_fd);
};

void print_configs() {
    fprintf(stdout, "Configs:\n");
    node_t *node = root.head;
    while (node != NULL) {
        print_config(node->config);
        node = node->next;
    }
}

void print_config(config_t *config) {
    fprintf(stdout,
            "    message: '%s' with timeout %ld\n",
            config->message,
            config->timeout_s);
}