/* Config file support (assign.sys) */

#include <stdbool.h>
#include <stdint.h>
#include <osbind.h>
#include "utils.h"

char *assignsys = 0L;

int16_t boot_drive;
static void get_boot_drive(void) { boot_drive = 'A' + *((uint16_t*)0x446/*_bootdev TOS variable*/); }

char assignsys_path[] = "x:\\assign.sys";
int32_t assignsys_size;

bool assignsys_init(void) {
    Supexec(get_boot_drive);
    assignsys_path[0] = boot_drive;

    int16_t ret = util_read_file_to_memory(assignsys_path, (void**)&assignsys, &assignsys_size); /* FIXME should use boot drive */
    if (ret < 0)
        return false;
    
    return true;
}

void assignsys_deinit(void) {
    if (assignsys)
        Mfree(assignsys);
}
