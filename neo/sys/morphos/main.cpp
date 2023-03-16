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

#include <dos/dosextens.h>
#include <proto/dos.h>
#include <proto/exec.h>
//#include <locale.h>

//static char path_argv[MAX_OSPATH];

struct Library *SocketBase;

extern "C"
{
	int __stack = 4*1024*1024;
}

#define DEFAULT_PATH ""

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
}

/*
===============
main
===============
*/

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
