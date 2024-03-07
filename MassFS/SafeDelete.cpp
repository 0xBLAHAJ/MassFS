#include <random>
#include <fstream>

#include "Globals.h"

namespace safe_delete
{
	bool safe_delete_file( const std::filesystem::path& path )
	{
		if ( std::ofstream fileStream( path, std::ios::binary ); fileStream )
		{
			const auto fileSize = static_cast<long long>( file_size( path ) );

			std::random_device rd;
			std::mt19937 gen( rd() );
			std::uniform_int_distribution<> dis( 0, 255 );
			constexpr std::size_t bufferSize = static_cast<size_t>( 1024 ) * 1024;
			std::vector<char> buffer( bufferSize );

			for ( long long written = 0; written < fileSize; written += BUFFER_SIZE )
			{
				if ( const long long currentPercentage = static_cast<long long>( static_cast<double>( g_currentBytes ) / static_cast<double>( g_maxBytes ) * 100 ); g_previousPercentage != currentPercentage )
				{
					g_previousPercentage = currentPercentage;
					helpers::print_percentage( currentPercentage );
				}

				const auto bytesCount = min( bufferSize, fileSize - written );

				for ( std::size_t i = 0; i < bytesCount; ++i )
				{
					buffer[ i ] = static_cast<char>( dis( gen ) );
				}

				fileStream.write( buffer.data(), bytesCount );

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

	bool safe_delete_directory( const std::filesystem::path& path )
	{
		return std::ranges::all_of( std::filesystem::directory_iterator( path ), [] ( const std::filesystem::directory_entry& entry ) { return safe_delete_file( entry ); } );
	}

	bool execute( const std::filesystem::path& fileToDelete, const char* modifier )
	{
		if ( !exists( fileToDelete ) )
		{
			std::println( "File {} does not exist in that directory.", fileToDelete.stem().string() );
			return false;
		}

		const std::string stemName = fileToDelete.stem().string();
		const bool isBatch = strcmp( modifier, "b" ) == 0 || strcmp( modifier, "batch" ) == 0;
		const bool isDir = is_directory( fileToDelete );

		// User is trying to delete a directory without explicitly calling for a batch delete, we're making sure that this is intended
		if ( isDir && !isBatch && !helpers::query_user_mismatch( stemName, "sd" ) )
		{
			return false;
		}

		g_actionStart = std::chrono::high_resolution_clock::now();
		std::print( "Calculating amount of bytes to write..." );
		g_maxBytes = isDir ? helpers::get_directory_size( fileToDelete ) : file_size( fileToDelete );
		std::print( " Found {}.\n", helpers::format_bytes( g_maxBytes ) );

		if ( isDir )
		{
			if ( !safe_delete_directory( fileToDelete ) )
			{
				return false;
			}

			remove_all( fileToDelete );
		}
		else
		{
			if ( !safe_delete_file( fileToDelete ) )
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
