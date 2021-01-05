/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include <dos/dosextens.h>
#include <proto/dos.h>

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <SDL_main.h>

#include "sys/platform.h"
#include "framework/Licensee.h"
#include "framework/FileSystem.h"
#include "sys/posix/posix_public.h"
#include "sys/sys_local.h"

#include <locale.h>

static char path_argv[MAX_OSPATH];

#define DEFAULT_PATH ""

#if 0

bool Sys_GetPath(sysPath_t type, idStr &path) {
	const char *s;
	char buf[MAX_OSPATH];
	char buf2[MAX_OSPATH];
	struct stat st;
	size_t len;

	path.Clear();

	switch(type) {
	case PATH_BASE:
		if (stat(BUILD_DATADIR, &st) != -1 && S_ISDIR(st.st_mode)) {
			path = BUILD_DATADIR;
			return true;
		}

		common->Warning("base path '" BUILD_DATADIR "' does not exist");

		// try next to the executable..
		if (Sys_GetPath(PATH_EXE, path)) {
			path = path.StripFilename();
			// the path should have a base dir in it, otherwise it probably just contains the executable
			idStr testPath = path + "/" BASE_GAMEDIR;
			if (stat(testPath.c_str(), &st) != -1 && S_ISDIR(st.st_mode)) {
				common->Warning("using path of executable: %s", path.c_str());
				return true;
			} else {
				idStr testPath = path + "/demo/demo00.pk4";
				if(stat(testPath.c_str(), &st) != -1 && S_ISREG(st.st_mode)) {
					common->Warning("using path of executable (seems to contain demo game data): %s", path.c_str());
					return true;
				} else {
					path.Clear();
				}
			}
		}

		#if 0
		// fallback to vanilla doom3 install
		if (stat(LINUX_DEFAULT_PATH, &st) != -1 && S_ISDIR(st.st_mode)) {
			common->Warning("using hardcoded default base path: " LINUX_DEFAULT_PATH);

			path = LINUX_DEFAULT_PATH;
			return true;
		}
		#else // Cowcat
		if (stat(DEFAULT_PATH, &st) != -1 && S_ISDIR(st.st_mode)) {
			common->Warning("using hardcoded default base path: " DEFAULT_PATH);

			path = DEFAULT_PATH;
			return true;
		}

		#endif

		return false;

	case PATH_CONFIG:
		s = getenv("XDG_CONFIG_HOME");
		if (s)
			idStr::snPrintf(buf, sizeof(buf), "%s/dhewm3", s);
		else
			idStr::snPrintf(buf, sizeof(buf), "%s/.config/dhewm3", getenv("HOME"));

		path = buf;
		return true;

	case PATH_SAVE:
		s = getenv("XDG_DATA_HOME");
		if (s)
			idStr::snPrintf(buf, sizeof(buf), "%s/dhewm3", s);
		else
			idStr::snPrintf(buf, sizeof(buf), "%s/.local/share/dhewm3", getenv("HOME"));

		path = buf;
		return true;

	case PATH_EXE:

		#if !defined(__MORPHOS__)

		idStr::snPrintf(buf, sizeof(buf), "/proc/%d/exe", getpid());
		len = readlink(buf, buf2, sizeof(buf2));
		if (len != -1) {
			if (len < MAX_OSPATH) {
				buf2[len] = '\0';
			} else {
				buf2[MAX_OSPATH - 1] = '\0';
			}
			path = buf2;
			return true;
		}
		#endif

		if (path_argv[0] != 0) {
			path = path_argv;
			return true;
		}

		return false;
	}

	return false;
}

#else

bool Sys_GetPath(sysPath_t type, idStr &path)
{
    char buf[1024];
	BPTR pathlock;

	path.Clear();

	switch(type)
	{
    		case PATH_BASE:
    		case PATH_CONFIG:
    		case PATH_SAVE:

            		if ( (pathlock = Lock("PROGDIR:", SHARED_LOCK)) )
            		{
                		if ( NameFromLock( pathlock, buf, sizeof( buf ) ) )
                		{
                    			path = buf;
                		}

                		UnLock(pathlock);
            		}

			        return true;

    		case PATH_EXE:

            		if ( (pathlock = Lock("PROGDIR:", SHARED_LOCK)) )
            		{
                		if ( NameFromLock( pathlock, buf, sizeof( buf ) ) )
                		{
                    			struct Node *thisTask = (struct Node *)FindTask(NULL);

                   	 		    AddPart(buf, thisTask->ln_Name, 1024);

                    			path = buf;
                		}

                		UnLock(pathlock);
            		}
                    
                    return true;
	}

	return false;
}

#endif


/*
===============
Sys_Shutdown
===============
*/
void Sys_Shutdown( void )
{
	if (SocketBase)
		CloseLibrary(SocketBase);

	Posix_Shutdown();
}

/*
================
Sys_GetSystemRam
returns in megabytes
================
*/
int Sys_GetSystemRam( void )
{
	int mb;

	mb = ( ( AvailMem( MEMF_ANY ) / ( 1024 * 1024 ) ) + 8 ) & ~15;
	return mb;
}

/*
==================
Sys_DoStartProcess
if we don't fork, this function never returns
the no-fork lets you keep the terminal when you're about to spawn an installer

if the command contains spaces, system() is used. Otherwise the more straightforward execl ( system() blows though )
==================
*/

void Sys_DoStartProcess( const char *exeName, bool dofork )
{
	bool use_system = false;

	if ( strchr( exeName, ' ' ) ) {
		use_system = true;
	} else {
		// set exec rights when it's about a single file to execute
		struct stat buf;
		if ( stat( exeName, &buf ) == -1 ) {
			printf( "stat %s failed: %s\n", exeName, strerror( errno ) );
		} else {
			if ( chmod( exeName, buf.st_mode | S_IXUSR ) == -1 ) {
				printf( "cmod +x %s failed: %s\n", exeName, strerror( errno ) );
			}
		}
	}
	
	if ( use_system ) {
		printf( "system %s\n", exeName );
		if (system( exeName ) == -1)
			printf( "system failed: %s\n", strerror( errno ) );
		else
			sleep( 1 );	// on some systems I've seen that starting the new process and exiting this one should not be too close
	} else {
		printf( "execl %s\n", exeName );
		//execl( exeName, exeName, NULL );
		printf( "execl failed: %s\n", strerror( errno ) );
	}
	// terminate
	_exit( 0 );
}

/*
=================
Sys_OpenURL
=================
*/
void idSysLocal::OpenURL( const char *url, bool quit ) {
#if 0
	const char	*script_path;
	idFile		*script_file;
	char		cmdline[ 1024 ];

	static bool	quit_spamguard = false;

	if ( quit_spamguard ) {
		common->DPrintf( "Sys_OpenURL: already in a doexit sequence, ignoring %s\n", url );
		return;
	}

	common->Printf( "Open URL: %s\n", url );
	// opening an URL on *nix can mean a lot of things ..
	// just spawn a script instead of deciding for the user :-)

	// look in the savepath first, then in the basepath
	script_path = fileSystem->BuildOSPath( cvarSystem->GetCVarString( "fs_savepath" ), "", "openurl.sh" );
	script_file = fileSystem->OpenExplicitFileRead( script_path );
	if ( !script_file ) {
		script_path = fileSystem->BuildOSPath( cvarSystem->GetCVarString( "fs_basepath" ), "", "openurl.sh" );
		script_file = fileSystem->OpenExplicitFileRead( script_path );
	}
	if ( !script_file ) {
		common->Printf( "Can't find URL script 'openurl.sh' in either savepath or basepath\n" );
		common->Printf( "OpenURL '%s' failed\n", url );
		return;
	}
	fileSystem->CloseFile( script_file );

	// if we are going to quit, only accept a single URL before quitting and spawning the script
	if ( quit ) {
		quit_spamguard = true;
	}

	common->Printf( "URL script: %s\n", script_path );

	// StartProcess is going to execute a system() call with that - hence the &
	idStr::snPrintf( cmdline, 1024, "%s '%s' &",  script_path, url );
	sys->StartProcess( cmdline, quit );
#endif
}

/*
===============
main
===============
*/

struct Library *SocketBase;

int main(int argc, char **argv)
{
	SocketBase = OpenLibrary("bsdsocket.library", 0);


	if ( argc > 1 )
	{
		common->Init( argc-1, &argv[1] );
	}

	else
	{
		common->Init( 0, NULL );
	}


	while (1)
	{
		common->Frame();
	}

	return 0;
}
