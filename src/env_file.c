#include "env_file.h"

static char create_envf_var(EnvF_Var_t **env_var, const char *name, const char *value);
static void free_envf_var(EnvF_Var_t *env_var);
static char push_envf_var(EnvF_t *envf_inst, EnvF_Var_t *var);
static char reclaim_envf_var(EnvF_t *envf_inst);

ENVF_ERROR_t envf_init(char *file_name, EnvF_t **envf_inst)
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
            // Reset char carret on each new line.
            if (c == '\n') {
                i = 0;
                continue;
            }

            if (i >= ENVF_VAR_NAME_MAX_LEN)
            {
                log_error("Env variable name is too long (len: %d).", i);
                envf_deinit(*envf_inst);
                return ENVVAR_TOO_LONG;
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
                log_error("Env variable value is too long (len: %d).", i);
                envf_deinit(*envf_inst);
                return ENVVAR_TOO_LONG;
            }

            if (c == '\n' || c == EOF)
            {
                var_value[i++] = '\0';
                rf = 0;
                i = 0;

                EnvF_Var_t *env_var = NULL;
                if (create_envf_var(&env_var, var_name, var_value) != 0)
                {
                    envf_deinit(*envf_inst);
                    return MALLOC_ERROR;
                }
                if (push_envf_var(*envf_inst, env_var) != 0)
                {
                    envf_deinit(*envf_inst);
                    return MALLOC_ERROR;
                }
            }
            else
            {
                var_value[i++] = c;
            }
        }
    } while (c != EOF);

    fclose(file);

    char reclaimErr = reclaim_envf_var(*envf_inst);
    if (reclaimErr != 0)
    {
        log_warn("Error during reclaiming the memory. Error code: %d.", reclaimErr);
    }

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
            log_debug("deinit: var[%zu] is null, skip", envf_inst->cap);
            continue;
        }

        log_debug("deinit: free var[%zu]", envf_inst->cap);
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

char *envf_get(EnvF_t *envf_inst, char *var_name)
{
    for (size_t i = 0; i < envf_inst->len; i++)
    {
        if (strcmp(var_name, envf_inst->vars[i]->name) == 0)
        {
            return envf_inst->vars[i]->value;
        }
    }

    return NULL;
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

static char push_envf_var(EnvF_t *envf_inst, EnvF_Var_t *var)
{
    if (envf_inst->cap == 0)
    {
        // Initial array memory allocation.
        size_t inital_cap = 1;
        envf_inst->vars = malloc(sizeof(EnvF_Var_t *) * inital_cap);
        if (envf_inst->vars == NULL)
        {
            return 1;
        }

        envf_inst->cap = inital_cap;
        envf_inst->len = 0;
    }
    else if (envf_inst->cap == envf_inst->len)
    {
        // Increase the capacity.
        size_t old_cap = envf_inst->cap;
        size_t new_cap = envf_inst->cap * 2;
        EnvF_Var_t **realloc_ptr = realloc(envf_inst->vars, sizeof(EnvF_Var_t *) * new_cap);
        if (realloc_ptr == NULL)
        {
            log_error("Unable to increase the envf_inst->vars capacity.");
            return 2;
        }
        memset(realloc_ptr + old_cap, 0, sizeof(EnvF_Var_t *) * (new_cap - old_cap));
        envf_inst->vars = realloc_ptr;
        envf_inst->cap = new_cap;
        log_debug("New capacity: %zu -> %zu.", old_cap, new_cap);
    }

    envf_inst->vars[envf_inst->len++] = var;

    return 0;
}

static char reclaim_envf_var(EnvF_t *envf_inst)
{
    if (envf_inst->len == envf_inst->cap)
    {
        return 0;
    }

    EnvF_Var_t **realloc_ptr = realloc(envf_inst->vars, sizeof(EnvF_Var_t *) * envf_inst->len);
    if (realloc_ptr == NULL)
    {
        return 1;
    }

    envf_inst->vars = realloc_ptr;
    envf_inst->cap = envf_inst->len;

    return 0;
}