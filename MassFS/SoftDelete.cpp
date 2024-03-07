#include <filesystem>

#include "Globals.h"

namespace soft_delete
{
	bool execute( const std::filesystem::path& fileToDelete, const char* modifier )
	{
		if ( !exists( fileToDelete ) )
		{
			std::println( "File {} does not exist in that directory.", fileToDelete.stem().string() );
			return false;
		}

		const std::string stemName = fileToDelete.stem().string();
		const bool isBatch = strcmp( modifier, "b" ) == 0 || strcmp( modifier, "batch" ) == 0;

		if ( is_directory( fileToDelete ) )
		{
			if ( !isBatch )
			{
				std::println( "'{}' is a folder, but you called massfs d with 'u' as a parameter, 'b' should be used for directories as they are by definition a batch operation.", stemName );
				std::println( "Proceed with the deletion? Y/N" );

				while ( true )
				{
					std::string response{};
					std::cin >> response;

					if ( response == "Y" || response == "y" )
					{
						std::println( "Proceeding with Batch Deletion of files contained by {}", stemName );
						break;
					}

					if ( response == "N" || response == "n" )
					{
						return false;
					}

					std::println( "Invalid Response! Please input Y or N." );
				}
			}

			const size_t count = helpers::get_directory_file_count( fileToDelete );
			remove_all( fileToDelete );
			std::println( "Deleted {} files inside of '{}'.", count, stemName );
			return true;
		}

		if ( !remove( fileToDelete ) )
		{
			std::println( "Could not delete {}.", fileToDelete.stem().string() );
			return false;
		}

		return true;
	}
}
