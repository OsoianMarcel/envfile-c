#ifndef ENV_FILE_H
#define ENV_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logging.h"

#define ENVF_VAR_NAME_MAX_LEN 32
#define ENVF_VAR_VALUE_MAX_LEN 256

typedef enum ENVF_ERROR_ENUM
{
    UNKNOWN_ERROR,
    FILE_OPEN_ERROR,
    MALLOC_ERROR,
    ENVVAR_TOO_LONG,
} ENVF_ERROR_t;

typedef struct EnvF_Var_s
{
        char *name;
        char *value;
} EnvF_Var_t;

typedef struct EnvF_s
{
        size_t len;
        size_t cap;
        EnvF_Var_t **vars;
} EnvF_t;

extern ENVF_ERROR_t envf_init(char *file_name, EnvF_t **envf_inst);
extern void envf_deinit(EnvF_t *envf_inst);
extern char* envf_get(EnvF_t *envf_inst, char *var_name);

#endif