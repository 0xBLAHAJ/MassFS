#include "Globals.h"

namespace help
{
	void execute()
	{
		std::println( "MassFS - v{}", VERSION );
		std::println( "help - Shows the list of available commands and their use" );
		std::println( "delete - Simply Deletes a File, the same way your OS would do it" );
		std::println( "zero - Hard Deletes a File, this file will not be recoverable using recovery software" );
		std::println( "path - Adds massfs to the path - Requires Admin Privileges" );

		std::println( "\nYou can also use help <command> to get more details about a specific command" );
	}
}
