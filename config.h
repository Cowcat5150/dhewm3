#ifndef __DOOM3_CONFIG_H__
#define __DOOM3_CONFIG_H__

#if defined(__MORPHOS__)

#define BUILD_OS		"morphos"
#define BUILD_CPU		"ppc"

#define BUILD_LIBRARY_SUFFIX	".so"

#define BUILD_LIBDIR		""
#define BUILD_DATADIR		""

/* #undef HAVE_JPEG_MEM_SRC */

/* #define ID_ENABLE_CURL */

#else

#define BUILD_OS				"linux"
#define BUILD_CPU				"x86_64"

#define BUILD_LIBRARY_SUFFIX	".so"

#define BUILD_LIBDIR			"/usr/local/lib/dhewm3"
#define BUILD_DATADIR			"/usr/local/share/dhewm3"

/* #undef HAVE_JPEG_MEM_SRC */

/* #undef ID_ENABLE_CURL */

#endif

#endif
