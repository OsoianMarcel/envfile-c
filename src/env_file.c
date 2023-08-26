#include "env_file.h"

static char create_envf_var(EnvF_Var_t **env_var, const char *name, const char *value);
static void free_envf_var(EnvF_Var_t *env_var);
static void push_envf_var(EnvF_t *envf_inst, EnvF_Var_t *var);
static char reclaim_envf_var(EnvF_t *envf_inst);

enum ENVF_ERROR envf_init(char *file_name, EnvF_t **envf_inst)
{
    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        return FILE_OPEN_ERROR;
    }

    *envf_inst = calloc(1, sizeof(EnvF_t));
    if (*envf_inst == NULL)
    {
        return MALLOC_ERROR;
    }

    char var_name[ENVF_VAR_NAME_MAX_LEN];
    char var_value[ENVF_VAR_VALUE_MAX_LEN];
    char c = 0;
    int i = 0;
    char rf = 0;

    do
    {
        c = fgetc(file);

        if (c == EOF && i == 0)
        {
            break;
        }

        // Reading the variable name.
        if (rf == 0)
        {
            if (i >= ENVF_VAR_NAME_MAX_LEN)
            {
                log_error("var_name overflow");
                exit(1);
            }

            if (c == '=')
            {
                // printf("/n: %d\n", i);
                var_name[i++] = '\0';
                rf = 1;
                i = 0;
            }
            else
            {
                // printf("+%c: %d\n", c, i);
                var_name[i++] = c;
            }
        }
        else
        {
            if (i >= ENVF_VAR_VALUE_MAX_LEN)
            {
                log_error("var_value overflow");
                exit(1);
            }

            if (c == '\n' || c == EOF)
            {
                // printf("/v: %d\n", i);
                var_value[i++] = '\0';
                rf = 0;
                i = 0;

                EnvF_Var_t *env_var = NULL;
                if (create_envf_var(&env_var, var_name, var_value) > 0) {
                    // TODO: Handle the problem.
                }
                push_envf_var(*envf_inst, env_var);

                // log_debug("%s: %s", var_name, var_value);
            }
            else
            {
                // printf("+%c: %d\n", c, i);
                var_value[i++] = c;
            }
        }
    } while (c != EOF);

    fclose(file);

    reclaim_envf_var(*envf_inst);

    return UNKNOWN_ERROR;
}

void envf_deinit(EnvF_t *envf_inst)
{
    if (envf_inst == NULL)
    {
        log_debug("deinit: Nothing to deinit");
        return;
    }

    // Free all the variable structs, and their string members.
    while (envf_inst->cap--)
    {
        EnvF_Var_t *var = envf_inst->vars[envf_inst->cap];
        // NULL means the pointer does not hold an env variable.
        if (var == NULL)
        {
            free(var);
            log_debug("deinit: var %zu is null, skip", envf_inst->cap);
            continue;
        }

        log_debug("deinit: free var[%zu]->(name|value) & var", envf_inst->cap);
        free_envf_var(var);
    }

    if (envf_inst->vars != NULL)
    {
        log_debug("deinit: envf_inst->vars (array of pointers)");
        free(envf_inst->vars);
    }
    log_debug("deinit: envf_inst");
    free(envf_inst);
}

EnvF_Var_t *envf_get(EnvF_t *envf_inst, char *var_name)
{
    for (size_t i = 0; i < envf_inst->len; i++)
    {
        if (strcmp(var_name, envf_inst->vars[i]->name) == 0)
        {
            return envf_inst->vars[i];
        }
    }

    return NULL;
}

char *envf_val(EnvF_t *envf_inst, char *var_name)
{
    EnvF_Var_t *var = envf_get(envf_inst, var_name);

    if (var == NULL)
    {
        return NULL;
    }

    return var->value;
}

static char create_envf_var(EnvF_Var_t **env_var, const char *name, const char *value)
{
    *env_var = malloc(sizeof(EnvF_Var_t));
    if (*env_var == NULL)
    {
        return 1;
    }

    (*env_var)->name = malloc(strlen(name) + 1);
    (*env_var)->value = malloc(strlen(value) + 1);

    if ((*env_var)->name == NULL || (*env_var)->value == NULL)
    {
        // Cleanup on error.
        free((*env_var)->name);
        free((*env_var)->value);
        free(*env_var);
        // Set the pointer to NULL.
        *env_var = NULL;

        return 2;
    }

    strcpy((*env_var)->name, name);
    strcpy((*env_var)->value, value);

    return 0;
}

static void free_envf_var(EnvF_Var_t *env_var)
{
    free(env_var->name);
    free(env_var->value);
    free(env_var);
}

static void push_envf_var(EnvF_t *envf_inst, EnvF_Var_t *var)
{
    // Initial array memory allocation.
    if (envf_inst->cap == 0)
    {
        size_t inital_cap = 1;
        envf_inst->vars = malloc(sizeof(EnvF_Var_t *) * inital_cap);
        if (envf_inst->vars != NULL)
        {
            envf_inst->cap = inital_cap;
            envf_inst->len = 0;
        }
    }
    else if (envf_inst->cap == envf_inst->len)
    {
        // Double the envf_inst capacity.
        size_t new_cap = envf_inst->cap * 2;
        EnvF_Var_t **old_ptr = envf_inst->vars;
        envf_inst->vars = realloc(envf_inst->vars, sizeof(EnvF_Var_t *) * new_cap);
        if (envf_inst->vars == NULL)
        {
            log_error("Unable to increase the envf_inst->vars capacity.");
            envf_inst->vars = old_ptr;
            return;
        }
        memset(envf_inst->vars + envf_inst->cap, 0, sizeof(EnvF_Var_t *) * (new_cap - envf_inst->cap));
        envf_inst->cap = new_cap;
        log_debug("Info: New envf_inst cap: %d; len: %d", envf_inst->cap, envf_inst->len);
    }

    envf_inst->vars[envf_inst->len++] = var;
    log_debug("items addr: %p", envf_inst->vars);
}

static char reclaim_envf_var(EnvF_t *envf_inst)
{
    if (envf_inst->len == envf_inst->cap)
    {
        return 0;
    }

    EnvF_Var_t **relloc_ptr = realloc(envf_inst->vars, sizeof(EnvF_Var_t *) * envf_inst->len);
    if (relloc_ptr == NULL)
    {
        return 1;
    }

    envf_inst->vars = relloc_ptr;
    envf_inst->cap = envf_inst->len;

    return 0;
}