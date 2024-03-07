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
			if ( !isBatch && !helpers::query_user_mismatch( stemName, "d" ) )
			{
				return false;
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
