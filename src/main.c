#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "./logging.h"
#include "./env_file.h"

int main()
{
    log_info("Start.");

    EnvF_t *envf = NULL;
    ENVF_ERROR_t err = envf_init("filename2.txt", &envf);
    if (err != UNKNOWN_ERROR) {
        log_error("Unable to initialize envf. Error: %d", err);
        return 1;
    }

    log_debug("len: %zu; cap: %zu;", envf->len, envf->cap);
    for (size_t i = 0; i < envf->len; i++)
    {
        EnvF_Var_t *var = envf->vars[i];
        log_debug("%zu: %s=%s", i, var->name, var->value);
    }

    char *found = envf_get(envf, "ENV");
    log_debug("found: %s", found);

    char *notFound = envf_get(envf, "inexistent");
    log_debug("not found: %s", notFound);

    envf_deinit(envf);

    log_info("End.");

    return EXIT_SUCCESS;
}