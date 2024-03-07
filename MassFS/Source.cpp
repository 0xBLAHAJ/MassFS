#include <filesystem>
#include <print>
#include <functional>
#include <windows.h>

#include "Globals.h"

namespace
{
	void print_default_usage()
	{
		std::println( "The syntax of this command is:" );
		std::println( "massfs\n       [? | zero | delete | move | rename]\n       [b | u]\n       [file | path to file]" );
		std::println( "To get started, run massfs /? or massfs help to get a list of what every argument does." );
	}

	uint8_t handle_action( const int argc, char* argv[ ], const uint64_t actionHash )
	{
		bool actionSuccess = false;

		const char* source = argv[ 0 ];
		const char* action = argv[ 1 ];
		const char* modifier = argc > 2 ? argv[ 2 ] : "u";

		switch ( actionHash )
		{
		HELP_HASHES
			help::execute( argc > 2 ? std::optional{ HASH( argv[ 2 ] ) } : std::nullopt );
			return 2;
		SOFT_DELETE_HASHES
			if ( argc <= 3 )
			{
				std::println( "Not enough arguments for this command, the correct usage is massfs r <b/u> <path>" );
				break;
			}
			actionSuccess = soft_delete::execute( std::filesystem::path( argv[ 3 ] ), modifier );
			break;
		ZERO_OUT_HASHES
			if ( argc <= 3 )
			{
				std::println( "Not enough arguments for this command, the correct usage is massfs z <b/u> <path>" );
				break;
			}
			actionSuccess = zero_out::execute( std::filesystem::path( argv[ 3 ] ), modifier );
			break;
		SAFE_DELETE_HASHES
			if ( argc <= 3 )
			{
				std::println( "Not enough arguments for this command, the correct usage is massfs z <b/u> <path>" );
				break;
			}
			actionSuccess = safe_delete::execute( std::filesystem::path( argv[ 3 ] ), modifier );
			break;
		case HASH( "path" ):
			actionSuccess = path_manager::execute( std::filesystem::path( source ), argc > 2 && strcmp( argv[ 2 ], "ps" ) == 0 );
			break;
		default:
			actionSuccess = false;
			std::println( "Could not find command {}, please run 'massfs help' for more information", action );
			break;
		}

		return actionSuccess;
	}
}

int main( const int argc, char* argv[ ] )
{
	if ( argc <= 1 )
	{
		print_default_usage();
		return 0;
	}

	const HANDLE handle = GetStdHandle( STD_OUTPUT_HANDLE );
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	GetConsoleScreenBufferInfo( handle, &consoleInfo );
	g_baseLine = consoleInfo.dwCursorPosition.Y;

	const uint8_t actionSuccess = handle_action( argc, argv, HASH( argv[ 1 ] ) );

	switch ( actionSuccess )
	{
	case 0:
		std::println( "Action couldn't be processed. See above for further information." );
		return 1;
	case 1:
		std::println( "Action was processed succesfully!" );
		return 0;
	default:
		return 0;
	}
}
