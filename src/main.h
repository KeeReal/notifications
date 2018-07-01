

#ifndef EPOLL_EXPERIMENTS_MAIN_H
#define EPOLL_EXPERIMENTS_MAIN_H

#include <sys/stat.h>
#include <stdint.h>
#include <zconf.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <signal.h>
#include <libnotify/notify.h>

// ======================================================
//              TYPES
// ======================================================
typedef struct configuration {
    int timer_fd;
    __time_t timeout_s;
    char *message; // message that will be fired when timer ticks
} config_t;

typedef struct node {
    struct node *next;
    config_t *config;
} node_t;

struct rootObj {
    node_t *head;
    int shutdown;
    int epoll_fd; // epoll instance FD
    NotifyNotification *notification;
};
// ======================================================
//              MAIN.c
// ======================================================
struct rootObj root;

void root_init_epoll();
void root_init_timer(config_t *config);
void root_add_timer(config_t *config);

void print_configs();
void print_config(config_t *);

void root_setup_signals(void);
void root_signal_handler(int);

void root_stop();

// ======================================================
//              Notifications
// ======================================================
#define NOTIFICATIONS_APP_NAME "Notifications"
#define NOTIFICATIONS_SUMMARY "Reminder"
#define NOTIFICATIONS_ICON "dialog-information"


void notification_close(void);
void notification_show(config_t*);
// ======================================================
//              CONFIG.h
// ======================================================

void config_add(config_t *config);
config_t* config_new();
void config_free(config_t *config);

// ======================================================
//              NODE.h
// ======================================================
node_t* node_new();
void node_free(node_t *node);
// ======================================================
//              PARSER.h
// ======================================================
#define BUFFER_SIZE                     10
#define ACCUMULATOR_INITIAL_SIZE        10
#define PARSER_STATE_INITIAL            0
#define PARSER_STATE_READ_MESS          1
#define PARSER_STATE_EXPECT_COLON       2
#define PARSER_STATE_READ_TIMEOUT       3
#define PARSER_STATE_EXPECT_LF          4

#define PARSER_DEFAULT_CONFIG_FILE "/etc/notifications/config"

static char *DIGITS = "0123456789";

struct configurationParser {
    FILE *file;
    char *buffer;
    int state;
    uintmax_t timeout;
    char *accumulator;
    size_t accumulator_index;
    size_t accumulator_allocated_length;
    config_t *config_out;
};



struct configurationParser parser;

int parser_readconfig(char *);
void parser_initialize();
void parser_openfile(char *);
void parser_process_char(char ch);
void parser_reallocate_accumulator();
void parser_reset_accumulator();
int parser_is_digit(char);

void parser_cleanup();

#endif //EPOLL_EXPERIMENTS_MAIN_H
