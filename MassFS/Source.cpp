#include <filesystem>
#include <print>
#include <functional>
#include <windows.h>

#include "Globals.h"

int main( const int argc, char* argv[ ] )
{
	if ( argc <= 1 )
	{
		std::println( "The syntax of this command is:" );
		std::println( "massfs\n       [? | zero | delete | move | rename]\n       [b | u]\n       [file | path to file]" );
		std::println( "To get started, run massfs /? or massfs help to get a list of what every argument does." );
		return 0;
	}

	const HANDLE handle = GetStdHandle( STD_OUTPUT_HANDLE );
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	GetConsoleScreenBufferInfo( handle, &consoleInfo );
	g_baseLine = consoleInfo.dwCursorPosition.Y;

	const char* source = argv[ 0 ];
	const char* action = argv[ 1 ];
	const char* modifier = argc > 2 ? argv[ 2 ] : "u";

	bool succeeded = false;

	switch ( fnv1a64( action ) )
	{
	case fnv1a64( "h" ):
	case fnv1a64( "?" ):
	case fnv1a64( "help" ):
	case fnv1a64( "/?" ):
		succeeded = true;
		help::execute();
		break;
	case fnv1a64( "d" ):
	case fnv1a64( "delete" ):
	case fnv1a64( "r" ):
	case fnv1a64( "remove" ):
		if ( argc <= 3 )
		{
			std::println( "Not enough arguments for this command, the correct usage is massfs r <b/u> <path>" );
			break;
		}
		succeeded = soft_delete::execute( std::filesystem::path( argv[ 3 ] ), modifier );
		break;
	case fnv1a64( "z" ):
	case fnv1a64( "zero" ):
	case fnv1a64( "0" ):
	case fnv1a64( "hard_delete" ):
		if ( argc <= 3 )
		{
			std::println( "Not enough arguments for this command, the correct usage is massfs z <b/u> <path>" );
			break;
		}
		succeeded = hard_delete::execute( std::filesystem::path( argv[ 3 ] ), modifier );
		break;
	case fnv1a64( "path" ):
		succeeded = path_manager::execute( std::filesystem::path( source ), argc > 2 && strcmp( argv[ 2 ], "ps" ) == 0 );
		break;
	default:
		succeeded = false;
		std::println( "Could not find command {}, please run 'massfs help' for more information", action );
		break;
	}

	if ( !succeeded )
	{
		std::println( "Action couldn't be processed. See above for further information." );
		return 1;
	}

	std::println( "Action was processed succesfully!" );
	return 0;
}
