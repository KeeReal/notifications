#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <stdio.h>
#include "main.h"

int parser_readconfig(char *config) {
    fprintf(stdout, "Parse config\n");
    parser_initialize();
    parser_openfile(config);

    size_t n = 0;
    int i = 0;

    while ((n = fread(parser.buffer, 1, BUFFER_SIZE, parser.file)) > 0) {
        for (i = 0; i < n; ++i) {
            parser_process_char(parser.buffer[i]);
        }
    }

    fprintf(stdout, "Config has been pared\n");
}

void parser_initialize() {
    parser.buffer = malloc(BUFFER_SIZE);
    if (parser.buffer == NULL) {
        perror("Failed to allocate 'parser.buffer'");
        exit(EXIT_FAILURE);
    }

    parser.accumulator_index = 0;
    parser.accumulator_allocated_length = ACCUMULATOR_INITIAL_SIZE;
    parser.accumulator = malloc(parser.accumulator_allocated_length);

    if (parser.accumulator == NULL) {
        perror("Failed to allocate 'parser.accumulator'");
        exit(EXIT_FAILURE);
    }

    parser.state = PARSER_STATE_INITIAL;
}

void parser_openfile(char *config) {
    char *file = config;
    if (file == NULL) {
        file = PARSER_DEFAULT_CONFIG_FILE;
    }

    fprintf(stdout, "Use config at '%s'\n", file);

    struct stat sb;
    if (stat(file, &sb) < 0) {
        perror("Could not stat file");
        exit(EXIT_FAILURE);
    }

    if (!S_ISREG(sb.st_mode)) {
        fprintf(stderr, "Config must be a regular file\n");
        exit(EXIT_FAILURE);
    }

    parser.file = fopen(file, "r");
    if (parser.file == NULL) {
        perror("Failed to open a file");
        exit(EXIT_FAILURE);
    }
}

void parser_process_char(char ch) {
    if (parser.accumulator_index == parser.accumulator_allocated_length) {
        parser_reallocate_accumulator();
    }

    switch (parser.state) {

        case PARSER_STATE_INITIAL:
            if (ch == '\'') {
                parser.state = PARSER_STATE_READ_MESS;
                parser_reset_accumulator();
                parser.config_out = config_new();
            }
            break;

        case PARSER_STATE_READ_MESS:
            if (ch == '\'') {
                parser.state = PARSER_STATE_EXPECT_COLON;
                parser.accumulator[parser.accumulator_index] = '\0';
                fprintf(stdout, "    message: '%s'\n", parser.accumulator);

                parser.config_out->message = malloc(parser.accumulator_index);
                memcpy(parser.config_out->message,
                       parser.accumulator,
                       parser.accumulator_allocated_length);

                break;
            }
            parser.accumulator[parser.accumulator_index++] = ch;
            break;

        case PARSER_STATE_EXPECT_COLON:
            if (ch != ':') {
                fprintf(stderr, "Expect ':' after message\n");
                exit(EXIT_FAILURE);
            }
            parser.state = PARSER_STATE_READ_TIMEOUT;
            parser_reset_accumulator();
            break;

        case PARSER_STATE_READ_TIMEOUT:
            if (parser_is_digit(ch)) {
                parser.accumulator[parser.accumulator_index++] = ch;
                break;
            }

            if (ch != 's') {
                fprintf(stderr, "Expect 's' after timeout value");
                exit(EXIT_FAILURE);
            }

            uintmax_t timeout = strtoumax(parser.accumulator, NULL, 10);
            if (errno == ERANGE) {
                fprintf(stderr, "Failed to convert timeout to number");
                exit(EXIT_FAILURE);
            }

            if (timeout <= 0) {
                fprintf(stderr, "Timeout must be positive");
                exit(EXIT_FAILURE);
            }

            fprintf(stdout, "    timeout: %ld\n", timeout);
            parser.state = PARSER_STATE_EXPECT_LF;
            parser.timeout = timeout;

            parser.config_out->timeout_s = timeout;
            config_add(parser.config_out);
            parser.config_out = NULL;
            // finish config, add to root !
            break;

        case PARSER_STATE_EXPECT_LF:
            if (ch != '\n') {
                fprintf(stderr, "Expect new line after units name");
                exit(EXIT_FAILURE);
            }
            parser.state = PARSER_STATE_INITIAL;
            break;

        default:
            break;

    }
}

void parser_reallocate_accumulator() {
    parser.accumulator_allocated_length *= 2;
    parser.accumulator = realloc(parser.accumulator,
                                 parser.accumulator_allocated_length);

    if (parser.accumulator == NULL) {
        perror("Failed to reallocate 'parser.accumulator'");
        exit(EXIT_FAILURE);
    }
}

void parser_reset_accumulator() {
    parser.accumulator_index = 0;
    memset(parser.accumulator, 0, parser.accumulator_allocated_length);
}

int parser_is_digit(char ch) {
    return strchr(DIGITS, ch) != NULL;
}

void parser_create_config() {

}

void parser_cleanup() {
    if (parser.buffer != NULL) {
        free(parser.buffer);
    }

    if (parser.accumulator != NULL) {
        free(parser.accumulator);
    }

    if (parser.file != NULL) {
        fclose(parser.file);
    }
}


