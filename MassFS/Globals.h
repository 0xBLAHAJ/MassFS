#pragma once
#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <windows.h>
#include <print>

inline uintmax_t g_currentBytes = 0;
inline uintmax_t g_maxBytes = 0;
inline long long g_previousPercentage = -1;
inline int g_baseLine = 0;
inline std::chrono::high_resolution_clock::time_point g_actionStart;

inline const char* VERSION = "0.01";
constexpr long long BUFFER_SIZE = 4096;

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

namespace helpers
{
	void print_percentage( long long percent );
	std::size_t get_directory_file_count( const std::filesystem::path& path );
	size_t get_directory_size( const std::filesystem::path& path );
	std::string format_bytes( size_t bytes );
}

namespace hard_delete
{
	bool execute( const std::filesystem::path& fileToDelete, const char* modifier );
}

namespace soft_delete
{
	bool execute( const std::filesystem::path& fileToDelete, const char* modifier );
}

namespace path_manager
{
	bool execute( const std::filesystem::path& currentExecutionPath, bool isExecutedByPowershell );
}

namespace help
{
	void execute();
}
