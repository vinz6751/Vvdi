/* Config file support (assign.sys) */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "utils.h"

char *assignsys = 0L;

static char assignsys_path[] = "x:\\assign.sys";
static int32_t assignsys_size;

bool assignsys_init(void) {
    assignsys_path[0] = get_boot_drive();

    int16_t ret = util_read_file_to_memory(assignsys_path, (void**)&assignsys, &assignsys_size); /* FIXME should use boot drive */
    if (ret < 0)
        return false;
    
    return true;
}

void assignsys_deinit(void) {
    if (assignsys)
        free(assignsys);
}
