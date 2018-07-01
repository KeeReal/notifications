
#include <stdlib.h>
#include <assert.h>
#include "main.h"

node_t *node_new() {
    node_t *node = NULL;
    node = malloc(sizeof(node_t));

    assert(node != NULL);

    return node;
}

void node_free(node_t *node) {
    // todo:
}
