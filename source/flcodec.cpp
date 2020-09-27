/*
Freelancer .FL Savegame encode/decoder

Credits to Sherlog <sherlog@t-online.de> for finding out the algorithm

(c) 2003 by Jor <flcodec@jors.net>

This is free software. Permission to copy, store and use granted as long
as this copyright note remains intact.

Compilation in a POSIX environment:

   cc -O -o flcodec flcodec.c

Or in Wintendo 32 (get the free lcc compiler):

   lcc -O flcodec.c
   lcclnk -o flcodec.exe flcodec.obj

*******
EDITED by mc_horst for use in FLHook

*/

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include <unistd.h>
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "flcodec.h"

/* Very Secret Key - this is Microsoft Security In Action[tm] */
const char gene[] = "Gene";

bool flc_decode(const char *ifile, const char *ofile) {
    int ifd, ofd, i, l, len, rc;
    char *mem, *buff, c, k, r;

    _sopen_s(&ifd, ifile, O_RDONLY | _O_BINARY, _SH_DENYWR,
             _S_IREAD | _S_IWRITE);

    if (ifd == -1)
        return false;

    len = _lseek(ifd, 0, SEEK_END);
    _lseek(ifd, 0, SEEK_SET);

    mem = (char *)malloc(len + 1);
    if (mem == NULL) {
        _close(ifd);
        return false;
    }

    rc = _read(ifd, mem, len);
    /*  if(rc != len)
      {
              free(mem);
              close(ifd);
              return false;
      } */

    _close(ifd);

    if (strncmp(mem, "FLS1", 4) != 0) {
        free(mem);
        return false;
    }

    _sopen_s(&ofd, ofile, O_CREAT | O_TRUNC | O_WRONLY | _O_BINARY, _SH_DENYWR,
             _S_IREAD | _S_IWRITE);
    if (ofd == -1) {
        free(mem);
        return false;
    }

    /* skip FLS1 */
    buff = mem + 4;
    l = len - 4;

    i = 0;
    while (i < l) {

        c = buff[i];
        k = (gene[i % 4] + i) % 256;

        r = c ^ (k | 0x80);

        rc = _write(ofd, &r, 1);
        if (rc != 1) {
            free(mem);
            _close(ofd);
            return false;
        }

        i++;
    }

    free(mem);
    _close(ofd);
    return true;
}

bool flc_encode(const char *ifile, const char *ofile) {
    int ifd, ofd, i, l, len, rc;
    char *mem, *buff, c, k, r;

    _sopen_s(&ifd, ifile, O_RDONLY | _O_BINARY, _SH_DENYWR,
             _S_IREAD | _S_IWRITE);

    if (ifd == -1)
        return false;

    len = _lseek(ifd, 0, SEEK_END);
    _lseek(ifd, 0, SEEK_SET);

    mem = (char *)malloc(len + 1);
    memset(mem, 0, len + 1);
    if (mem == NULL) {
        _close(ifd);
        return false;
    }

    rc = _read(ifd, mem, len);
    /*  if (rc != len)
      {
                    free(mem);
                    close(ifd);
                    return false;
      } */

    _close(ifd);

    _sopen_s(&ofd, ofile, O_CREAT | O_TRUNC | O_WRONLY | _O_BINARY, _SH_DENYWR,
             _S_IREAD | _S_IWRITE);
    if (ofd == -1) {
        free(mem);
        return false;
    }

    buff = mem;
    l = len;

    /* write magic token */
    rc = _write(ofd, "FLS1", 4);
    if (rc != 4) {
        free(mem);
        _close(ofd);
        return false;
    }

    i = 0;
    while (i < l) {

        c = buff[i];
        k = (gene[i % 4] + i) % 256;

        r = c ^ (k | 0x80);

        rc = _write(ofd, &r, 1);
        if (rc != 1) {
            free(mem);
            _close(ofd);
            return false;
        }

        i++;
    }
    free(mem);
    _close(ofd);
    return true;
}
