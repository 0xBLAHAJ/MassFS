#include <filesystem>
#include <print>
#include <fstream>
#include <iostream>
#include <thread>
#include <windows.h>

// anonymous namespace instead of static usage on functions
namespace
{
	namespace fs = std::filesystem;
	int base_line = 0;
	std::chrono::high_resolution_clock::time_point action_start;
	constexpr long long buffer_size = 4096;
	const char* version = "0.01";

	constexpr uint64_t fnv1a64( const char* apStr )
	{
		uint64_t hash = 14695981039346656037ULL; // 64 bit offset_basis = 14695981039346656037

		for ( uint32_t idx = 0; apStr[ idx ] != 0; ++idx )
		{
			// 64 bit FNV_prime = 240 + 28 + 0xb3 = 1099511628211
			hash = 1099511628211ULL * ( hash ^ static_cast<unsigned char>( apStr[ idx ] ) );
		}

		return hash;
	}

	void MoveCursor( const int x = -1, const int y = -1 )
	{
		const HANDLE handle = GetStdHandle( STD_OUTPUT_HANDLE );
		CONSOLE_SCREEN_BUFFER_INFO console_info;
		GetConsoleScreenBufferInfo( handle, &console_info );

		const COORD new_coord{ x != -1 ? static_cast<short>( x ) : console_info.dwCursorPosition.X, y != -1 ? static_cast<short>( y ) : console_info.dwCursorPosition.Y };
		SetConsoleCursorPosition( handle, new_coord );
	}

	void ClearPreviousLine()
	{
		std::print( "\r" );
		std::cout << std::string( 50, ' ' ) << "\r";
	}

	void PrintPercentage( const long long percent )
	{
		const size_t perten = static_cast<size_t>( floor( static_cast<double>( percent ) / 10 ) );
		MoveCursor( -1, 1 + base_line );
		ClearPreviousLine();
		std::cout << "   [" << std::string( perten, '|' ) << std::string( 10 - perten, ' ' ) << "] - " << percent << "%\n" << std::flush;

		if ( percent != 100 )
		{
			MoveCursor( -1, 2 + base_line );
			ClearPreviousLine();
			if ( percent != 0 )
			{
				const auto now = std::chrono::high_resolution_clock::now();
				const auto time_since_action_start_ms = ( std::chrono::duration_cast<std::chrono::milliseconds>( now - action_start ) ).count();
				const double ms_per_percent = static_cast<double>( time_since_action_start_ms ) / ( static_cast<double>( percent ) );
				const double remaining_ms = ( ceil( ms_per_percent * ( 100.0 - percent ) ) );
				std::cout << "   Time Remaining: " << static_cast<long long>( ceil( remaining_ms / 1000 ) ) << "s.\n";
				return;
			}

			std::cout << "   Time Remaining: Calculating...\n";
		}
	}

	bool SoftDelete( const fs::path& file_to_delete )
	{
		if ( !exists( file_to_delete ) )
		{
			std::println( "File {} does not exist in that directory.", file_to_delete.stem().string() );
			return false;
		}

		if ( !remove( file_to_delete ) )
		{
			std::println( "Could not delete {}.", file_to_delete.stem().string() );
			return false;
		}

		return true;
	}

	bool HardDelete( const fs::path& file_to_delete )
	{
		action_start = std::chrono::high_resolution_clock::now();
		if ( !exists( file_to_delete ) )
		{
			std::println( "Couldn't find {} in current directory.", file_to_delete.stem().string() );
			return false;
		}

		const auto file_size = static_cast<long long>( fs::file_size( file_to_delete ) );
		const std::string stem_name = file_to_delete.stem().string();
		std::println( "Currently Zeroing {}...\n", stem_name );

		if ( std::ofstream file_stream( file_to_delete, std::ios::binary ); file_stream )
		{
			constexpr char buffer[ buffer_size ] = { 0 };

			long long last_percent = -1;

			for ( long long written = 0; written < file_size; written += buffer_size )
			{
				if ( const long long current_percent = static_cast<long long>( static_cast<double>( written ) / static_cast<double>( file_size ) * 100 ); last_percent != current_percent )
				{
					last_percent = current_percent;
					PrintPercentage( current_percent );
				}

				file_stream.write( buffer, min( buffer_size, file_size - written ) );
			}

			file_stream.close();
			remove( file_to_delete );
			PrintPercentage( 100 );

			std::println( "\33[2K\rDone Deleting {}", stem_name );
			return true;
		}

		std::println( "Couldn't access {}: Access Denied.", stem_name );
		return false;
	}

	void PrintHelp()
	{
		std::print( "MassFS - v{}\n", version );
		std::print( "\nhelp - Shows the list of available commands and their use" );
		std::print( "\ndelete - Simply Deletes a File, the same way your OS would do it" );
		std::print( "\nzero - Hard Deletes a File, this file will not be recoverable using recovery software" );
		std::print( "\npath - Adds massfs to the path - Requires Admin Privileges" );

		std::print( "\n\nYou can also use help <command> to get more details about a specific command" );
	}

	DWORD ReRunAsAdmin( const fs::path& current_execution_path )
	{
		const auto operation = L"runas";
		const auto program = L"powershell.exe";

		std::wstringstream wss;
		wss << L"-Command \"& '" << current_execution_path.wstring() << L"' 'path' 'ps'\"";

		const auto args = wss.str();

		SHELLEXECUTEINFO sei = {  };
		sei.lpVerb = operation;
		sei.lpFile = program;
		sei.lpParameters = args.c_str();
		sei.nShow = SW_SHOW;
		sei.fMask = SEE_MASK_DEFAULT;

		if ( !ShellExecuteEx( &sei ) )
		{
			const DWORD dwError = GetLastError();
			return dwError;
		}

		return 0;
	}

	bool AddToSystemPath( const fs::path& current_execution_path, const bool is_executed_by_powershell = false )
	{
		if ( !is_executed_by_powershell )
		{
			const DWORD res = ReRunAsAdmin( current_execution_path );
			switch ( res )
			{
			case 0:
				return true;
			case ERROR_CANCELLED:
				std::print( "Please click 'Yes' on the UAC Request if you want to add MassFS to the System Path, this action requires Admin Privileges.\n" );
				return false;
			default:
				std::print( "dwError was {}", res );
				return false;
			}
		}

		const std::string string_path = current_execution_path.parent_path().string();
		std::print( "Called with {}\n", string_path );

		HKEY hKey;
		const auto sub_key = R"(SYSTEM\CurrentControlSet\Control\Session Manager\Environment)";
		const auto value_name = "Path";
		DWORD size;
		DWORD value_type = REG_EXPAND_SZ;

		std::print( "Opening HKEY_LOCAL_MACHINE/System/CurrentControlSet/Control/Session Manager/Environment/Path...\n" );
		if ( const LONG open_res = RegOpenKeyExA( HKEY_LOCAL_MACHINE, sub_key, 0, KEY_READ | KEY_WRITE, &hKey ); open_res == ERROR_SUCCESS )
		{
			std::print( "Querying Current Path...\n" );
			if ( LONG query_res = RegQueryValueExA( hKey, value_name, nullptr, nullptr, nullptr, &size ); query_res == ERROR_SUCCESS || query_res == ERROR_MORE_DATA )
			{
				std::vector<char> value( size );
				query_res = RegQueryValueExA( hKey, value_name, nullptr, &value_type, reinterpret_cast<LPBYTE>( value.data() ), &size );
				if ( query_res == ERROR_SUCCESS )
				{
					auto new_value = std::string( value.begin(), value.end() );
					std::print( "Ensuring we don't already exist in Path...\n" );
					std::print( "\n{}\n\n", new_value );
					if ( new_value.find( string_path ) == std::string::npos )
					{
						if ( !new_value.empty() )
						{
							switch ( new_value.back() )
							{
							case '\0': // Remove pre-existing null-terminator
							case ' ':  // Remove space (malformated Path)
								new_value.pop_back();
								break;
							case ';': // Don't add a double ;
								break;
							default:
								new_value += ';';
								break;
							}
						}

						new_value += string_path + ";";

						std::print( "Setting the new Registry Value...\n" );
						if ( const LONG set_res = RegSetValueExA( hKey, value_name, 0, value_type, reinterpret_cast<const BYTE*>( new_value.c_str() ), new_value.size() + 1 ); set_res != ERROR_SUCCESS )
						{
							RegCloseKey( hKey );
							std::print( "Failed to set new value in Registry! Ensure you have the necessary permissions.\n" );
							return false;
						}
					}
					else
					{
						std::print( "{} is already in the Path variable!\n", string_path );
						std::print( "\n{}\n\n", new_value );
					}
				}
				else
				{
					RegCloseKey( hKey );
					std::print( "Failed to query existing value in Registry! Ensure you have the necessary permissions.\n" );
					return false;
				}
			}
			else
			{
				RegCloseKey( hKey );
				std::print( "Failed to query existing size in Registry! Ensure you have the necessary permissions.\n" );
				return false;
			}
		}
		else
		{
			std::print( "Failed to open registry key! Ensure you have the necessary permissions.\n" );
			return false;
		}

		SendMessageTimeout( HWND_BROADCAST, WM_SETTINGCHANGE, 0, reinterpret_cast<LPARAM>( "Environment" ), SMTO_ABORTIFHUNG, 5000, nullptr );
		std::print( "Successfully added {} to the path!\n", string_path );
		RegCloseKey( hKey );
		return true;
	}
}

int main( const int argc, char* argv[ ] )
{
	if ( argc <= 1 )
	{
		std::print( "The syntax of this command is:" );
		std::print( "massfs\n       [? | zero | delete | move | rename]\n       [b | u]\n       [file | path to file]\n" );
		std::print( "To get started, run massfs /? or massfs help to get a list of what every argument does." );
		return 0;
	}

	const HANDLE handle = GetStdHandle( STD_OUTPUT_HANDLE );
	CONSOLE_SCREEN_BUFFER_INFO console_info;
	GetConsoleScreenBufferInfo( handle, &console_info );
	base_line = console_info.dwCursorPosition.Y;

	const char* RESERVED = argv[ 0 ];
	const char* ACTION = argv[ 1 ];

	bool Succeeded = false;

	switch ( fnv1a64( ACTION ) )
	{
	case fnv1a64( "z" ):
	case fnv1a64( "zero" ):
	case fnv1a64( "0" ):
	case fnv1a64( "hard_delete" ):
		if ( argc <= 3 )
		{
			std::print( "Not enough arguments for this command, the correct usage is massfs z <b/u> <path>\n" );
			break;
		}
		Succeeded = HardDelete( fs::path( argv[ 3 ] ) );
		break;
	case fnv1a64( "h" ):
	case fnv1a64( "?" ):
	case fnv1a64( "help" ):
	case fnv1a64( "/?" ):
		Succeeded = true;
		PrintHelp();
		break;
	case fnv1a64( "path" ):
		Succeeded = AddToSystemPath( fs::path( RESERVED ), argc > 2 && strcmp( argv[ 2 ], "ps" ) == 0 );
		break;
	default:
		Succeeded = false;
		std::print( "Could not find command {}, please run 'massfs help' for more information", ACTION );
		break;
	}

	if ( !Succeeded )
	{
		std::print( "\nAction couldn't be processed. See above for further information." );
		return 1;
	}

	std::print( "\nAction was processed succesfully!" );
	return 0;
}
