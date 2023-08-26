#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "./logging.h"
#include "./env_file.h"

int main()
{
    log_info("Start.");

    EnvF_t *envf = NULL;
    enum ENVF_ERROR err = envf_init("filename2.txt", &envf);
    if (err != UNKNOWN_ERROR) {
        log_error("Unable to initialize envf. Error: %d", err);
        return 1;
    }

    log_debug("len: %zu; cap: %zu;\n", envf->len, envf->cap);
    for (size_t i = 0; i < envf->len; i++)
    {
        EnvF_Var_t *var = envf->vars[i];
        log_debug("%zu: %s=%s", i, var->name, var->value);
    }

    char *found = envf_val(envf, "env");
    log_debug("find: %s", found);

    envf_deinit(envf);

    return EXIT_SUCCESS;
}