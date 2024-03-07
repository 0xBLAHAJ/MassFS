#include "Globals.h"

namespace help
{
	namespace
	{
		struct Command
		{
			const char* Name{};
			const char* Description{};
		};

		const std::vector<Command> commands =
		{
			{ "help", "Shows the list of available commands" },
			{ "delete", "Simply Deletes a File, the same way your OS would do it" },
			{ "zero", "Properly deletes a file, this file will not be recoverable using recovery software" },
			{ "safedel", "Randomizes a file's data before deletion, this is slower than 'zero' for no real privacy advantage" },
			{ "path", "Adds massfs to the path - Requires Admin Privileges" }
		};

		void print_formatted_multi( const char* str1, const char* str2, const char* str3 )
		{
			std::cout << str1;

			const HANDLE handle = GetStdHandle( STD_OUTPUT_HANDLE );
			CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
			GetConsoleScreenBufferInfo( handle, &consoleInfo );
			const int x = consoleInfo.dwCursorPosition.X;

			std::cout << str2 << "\n";
			helpers::move_cursor( x );
			std::cout << str3;
		}

		void default_print()
		{
			const auto longestNameIterator = std::ranges::max_element( commands, [] ( const Command& a, const Command& b ) { return std::strlen( a.Name ) < std::strlen( b.Name ); } );

			const HANDLE handle = GetStdHandle( STD_OUTPUT_HANDLE );
			CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
			GetConsoleScreenBufferInfo( handle, &consoleInfo );
			const int x = consoleInfo.dwCursorPosition.X + static_cast<int>( strlen( longestNameIterator->Name ) ) + 1;

			const std::function print_command = [&x] ( const char* name, const char* desc )
			{
				std::cout << name << " ";
				helpers::move_cursor( x );
				std::cout << "- " << desc << "\n";
			};

			print_command( "MassFS", VERSION );
			std::println( "" );

			for ( const auto& [ name, desc ] : commands )
			{
				print_command( name, desc );
			}


			std::println( "\nYou can also use help <command> to get more details about a specific command" );
		}
	}

	void execute( const std::optional<uint64_t> actionHash )
	{
		if ( !actionHash.has_value() )
		{
			default_print();
			return;
		}

		switch ( actionHash.value() )
		{
		SOFT_DELETE_HASHES
			print_formatted_multi( "massfs d <u(nique)/b(atch)> <path> - ", "Simply deletes the file/directory at the given path. For directories, use massfs d b.", "This action is recoverable by simple recovery softwares, but will not appear in the Recycle Bin." );
			break;
		ZERO_OUT_HASHES
			print_formatted_multi( "massfs z <u(nique)/b(atch)> <path> - ", "Replaces all the file(s)'s bytes in memory with a '0' then deletes the file. For directories, use massfs z b.", "This action makes the file(s) in question very unlikely to be recovered." );
			break;
		SAFE_DELETE_HASHES
			print_formatted_multi( "massfs sd <u(nique)/b(atch)> <path> - ", "Randomizes a file's bytes in memory then deletes the file. For directories, use massfs sd b.", "This is slower than massfs z and doesn't bring any benefit, usage is not suggested." );
			break;
		HELP_HASHES
			print_formatted_multi( "massfs ? [arg] - ", "If used without an argument, sends a list of all available commands.", "If used with an argument, gives specific information about that given command." );
			break;
		case HASH( "path" ):
			print_formatted_multi( "massfs path - ", "Adds MassFS to the System Environment Variables, so that MassFS can be used anywhere in a CMD or a PowerShell", "This action requires elevated privileges." );
			break;
		default:
			std::println( "Could not find that command, please run massfs help to get a list of all available commands." );
			break;
		}
	}
}
