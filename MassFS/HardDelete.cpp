#include <fstream>

#include "Globals.h"

namespace hard_delete
{
	namespace
	{
		bool zero_out_file( const std::filesystem::path& path )
		{
			if ( std::ofstream fileStream( path, std::ios::binary ); fileStream )
			{
				const auto fileSize = static_cast<long long>( file_size( path ) );
				constexpr char buffer[ BUFFER_SIZE ] = { 0 };

				for ( long long written = 0; written < fileSize; written += BUFFER_SIZE )
				{
					if ( const long long currentPercentage = static_cast<long long>( static_cast<double>( g_currentBytes ) / static_cast<double>( g_maxBytes ) * 100 ); g_previousPercentage != currentPercentage )
					{
						g_previousPercentage = currentPercentage;
						helpers::print_percentage( currentPercentage );
					}

					const auto bytesCount = min( BUFFER_SIZE, fileSize - written );
					fileStream.write( buffer, bytesCount );
					g_currentBytes += bytesCount;
				}

				fileStream.close();
			}
			else
			{
				std::println( "Couldn't open {}. Access Denied.", path.stem().string() );
				return false;
			}

			return true;
		}

		bool zero_out_directory( const std::filesystem::path& path )
		{
			return std::ranges::all_of( std::filesystem::directory_iterator( path ), [] ( const std::filesystem::directory_entry& entry ) { return zero_out_file( entry ); } );
		}

		bool query_user( const std::string& stemName )
		{
			std::println( "'{}' is a folder, but you called massfs d with 'u' as a parameter, 'b' should be used for directories as they are by definition a batch operation.", stemName );
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

	bool execute( const std::filesystem::path& fileToDelete, const char* modifier )
	{
		if ( !exists( fileToDelete ) )
		{
			std::println( "Couldn't find {} in current directory.", fileToDelete.stem().string() );
			return false;
		}

		const std::string stemName = fileToDelete.stem().string();
		const bool isBatch = strcmp( modifier, "b" ) == 0 || strcmp( modifier, "batch" ) == 0;
		const bool isDir = is_directory( fileToDelete );

		// User is trying to delete a directory without explicitly calling for a batch delete, we're making sure that this is intended
		if ( isDir && !isBatch )
		{
			if ( !query_user( stemName ) )
			{
				return false;
			}
		}

		g_actionStart = std::chrono::high_resolution_clock::now();
		std::print( "Calculating amount of bytes to write..." );
		g_maxBytes = isDir ? helpers::get_directory_size( fileToDelete ) : file_size( fileToDelete );
		std::print( " Found {}.\n", helpers::format_bytes( g_maxBytes ) );

		if ( isDir )
		{
			if ( !zero_out_directory( fileToDelete ) )
			{
				return false;
			}

			remove_all( fileToDelete );
		}
		else
		{
			if ( !zero_out_file( fileToDelete ) )
			{
				return false;
			}

			remove( fileToDelete );
		}

		helpers::print_percentage( 100 );
		std::println( "\33[2K\rDone Deleting {}", stemName );
		return true;
	}
}
