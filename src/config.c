


#include <stdlib.h>
#include <stdio.h>
#include "main.h"

config_t *config_new() {
    config_t *config = malloc(sizeof(config_t));
    if (config == NULL) {
        fprintf(stderr, "Failed to allocate config_t");
        exit(EXIT_FAILURE);
    }

//    config->timeout_ms = timeout_ms;
//    config->message = message;

    return config;
}

void config_add(config_t *config) {
    node_t *prev = root.head;

    node_t *node = node_new();
    node->next = prev;
    node->config = config;

    root.head = node;
}

void config_free(config_t *config) {
    // todo:
}
