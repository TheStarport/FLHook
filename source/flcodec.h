#ifndef _FLCODEC_
#define _FLCODEC_

#include <FLCoreDefs.h>

EXPORT bool flc_decode(const char* ifile, const char* ofile);
EXPORT bool flc_encode(const char* ifile, const char* ofile);

#endif
