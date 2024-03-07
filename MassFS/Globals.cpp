#include "Globals.h"

namespace helpers
{
	void move_cursor( const int x, const int y )
	{
		const HANDLE handle = GetStdHandle( STD_OUTPUT_HANDLE );
		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		GetConsoleScreenBufferInfo( handle, &consoleInfo );

		const COORD cursorPosition{ x != -1 ? static_cast<short>( x ) : consoleInfo.dwCursorPosition.X, y != -1 ? static_cast<short>( y ) : consoleInfo.dwCursorPosition.Y };
		SetConsoleCursorPosition( handle, cursorPosition );
	}

	void clear_previous_line()
	{
		std::print( "\r" );
		std::cout << std::string( 50, ' ' ) << "\r";
	}

	void print_percentage( const long long percent )
	{
		const size_t perten = static_cast<size_t>( floor( static_cast<double>( percent ) / 10 ) );
		move_cursor( -1, 1 + g_baseLine );
		clear_previous_line();
		std::cout << "   [" << std::string( perten, '|' ) << std::string( 10 - perten, ' ' ) << "] - " << percent << "%\n" << std::flush;

		if ( percent != 100 )
		{
			move_cursor( -1, 2 + g_baseLine );
			clear_previous_line();

			if ( percent != 0 )
			{
				const auto now = std::chrono::high_resolution_clock::now();
				const auto actionStartMs = ( std::chrono::duration_cast<std::chrono::milliseconds>( now - g_actionStart ) ).count();
				const double msPerPercent = static_cast<double>( actionStartMs ) / ( static_cast<double>( percent ) );
				const double remainingMs = ( ceil( msPerPercent * ( 100.0 - percent ) ) );
				std::cout << "   Time Remaining: " << static_cast<long long>( ceil( remainingMs / 1000 ) ) << "s.\n";
				return;
			}

			std::cout << "   Time Remaining: Calculating...\n";
		}
	}

	std::size_t get_directory_file_count( const std::filesystem::path& path )
	{
		using std::filesystem::directory_iterator;
		return std::distance( directory_iterator( path ), directory_iterator{} );
	}

	size_t get_directory_size( const std::filesystem::path& path )
	{
		uintmax_t totalSize = 0;

		for ( const auto& entry : std::filesystem::recursive_directory_iterator( path ) )
		{
			if ( is_regular_file( entry ) )
			{
				totalSize += file_size( entry );
			}
			else if ( is_directory( entry ) )
			{
				totalSize += get_directory_size( entry );
			}
		}

		return totalSize;
	}

	std::string format_bytes( const size_t bytes )
	{
		constexpr size_t scale_gb = static_cast<size_t>( 1024 * 1024 ) * 1024;
		constexpr size_t scale_mb = static_cast<size_t>( 1024 ) * 1024;
		constexpr size_t scale_kb = 1024;

		if ( bytes >= scale_gb )
		{
			return std::format( "{:.2f} GB", bytes / static_cast<double>( scale_gb ) );
		}

		if ( bytes >= scale_mb )
		{
			return std::format( "{:.2f} MB", bytes / static_cast<double>( scale_mb ) );
		}

		if ( bytes >= scale_kb )
		{
			return std::format( "{:.2f} KB", bytes / static_cast<double>( scale_kb ) );
		}

		return std::format( "{} B", bytes );
	}

	bool query_user_mismatch( const std::string& stemName, const std::string& actionValue )
	{
		std::println( "'{}' is a folder, but you called massfs {} with 'u' as a parameter, 'b' should be used for directories as they are by definition a batch operation.", stemName, actionValue );
		std::println( "Proceed with the deletion? Y/N" );

		while ( true )
		{
			std::string response{};
			std::cin >> response;

			if ( response == "Y" || response == "y" )
			{
				std::println( "Proceeding with Batch Deletion of files contained within {}", stemName );
				return true;
			}

			if ( response == "N" || response == "n" )
			{
				return false;
			}

			std::println( "Invalid Response! Please input Y or N." );
		}
	}
}
