/* Utility fonction (in C) */

#include <mint/errno.h>
#include <osbind.h>
#include <stdint.h>

#include "debug.h"

int16_t util_read_file_to_memory(const char *filename, void **file, int32_t *size) {
    uint16_t fh;
    uint16_t ret;
    *file = 0L;

    fh = Fopen(filename, 0/*read-only*/);
    if (fh < 0) {
        _debug("Failed to open %s", filename);
        ret = fh;
        goto error;
    }

    *size = Fseek(0L, fh, 2/*from end*/);
    if (*size < 0) {
        _debug("Failed to find size of %s", filename);
        ret = (int16_t)*size;
        goto error;
    }
    Fseek(0L, fh, 0/*from start*/);

    *file = (void*)Malloc(*size);
    if (*file == 0L) {
        _debug("Not enough memory to load font file %s", filename);
        ret = ENOMEM;
        goto error;
    }
    uint32_t r;
    if ((r = Fread(fh, *size, *file)) != *size) {
        _debug("Failed to read font file %s: %ld %ld\n", filename, *size, r);
        ret = (int16_t)r;
        goto error;
    }
    ret = 0; /* E_OK, no error */
    goto ok;

error:
    if (*file) {
        Mfree(*file);
        *file = 0L;
    }
ok:
    if (fh > 0)
        Fclose(fh);
    return ret;
}
