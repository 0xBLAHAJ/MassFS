#include "Globals.h"

namespace path_manager
{
	static DWORD re_run_as_admin( const std::filesystem::path& currentExecutionPath )
	{
		const auto operation = L"runas";
		const auto program = L"powershell.exe";

		std::wstringstream wss;
		wss << L"-Command \"& '" << currentExecutionPath.wstring() << L"' 'path' 'ps'\"";

		const auto args = wss.str();

		SHELLEXECUTEINFO sei = {};
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

	static std::string get_string_error( const DWORD error )
	{
		LPSTR buffer = nullptr;
		const size_t size = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), reinterpret_cast<LPSTR>( &buffer ), 0, nullptr );

		std::string message( buffer, size );
		LocalFree( buffer );

		return message;
	}

	bool execute( const std::filesystem::path& currentExecutionPath, const bool isExecutedByPowershell )
	{
		if ( !isExecutedByPowershell )
		{
			const DWORD res = re_run_as_admin( currentExecutionPath );
			switch ( res )
			{
			case 0:
				return true;
			case ERROR_CANCELLED:
				std::print( "Please click 'Yes' on the UAC Request if you want to add MassFS to the System Path, this action requires Admin Privileges.\n" );
				return false;
			default:
				std::print( "Could not start a new PowerShell, Error was: {}", get_string_error( res ) );
				return false;
			}
		}

		const std::string stringPath = currentExecutionPath.parent_path().string();
		std::print( "Called with {}\n", stringPath );

		HKEY hKey;
		const auto subKey = R"(SYSTEM\CurrentControlSet\Control\Session Manager\Environment)";
		const auto valueName = "Path";
		DWORD size = 0;
		DWORD valueType = REG_EXPAND_SZ;

		std::print( "Opening HKEY_LOCAL_MACHINE/System/CurrentControlSet/Control/Session Manager/Environment/Path...\n" );
		if ( const LONG openRes = RegOpenKeyExA( HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ | KEY_WRITE, &hKey ); openRes == ERROR_SUCCESS )
		{
			std::print( "Querying Current Path...\n" );
			if ( LONG queryRes = RegQueryValueExA( hKey, valueName, nullptr, nullptr, nullptr, &size ); queryRes == ERROR_SUCCESS || queryRes == ERROR_MORE_DATA )
			{
				std::vector<char> value( size );
				queryRes = RegQueryValueExA( hKey, valueName, nullptr, &valueType, reinterpret_cast<LPBYTE>( value.data() ), &size );
				if ( queryRes == ERROR_SUCCESS )
				{
					auto newValue = std::string( value.begin(), value.end() );
					std::print( "Ensuring we don't already exist in Path...\n" );
					std::print( "\n{}\n\n", newValue );
					if ( newValue.find( stringPath ) == std::string::npos )
					{
						if ( !newValue.empty() )
						{
							switch ( newValue.back() )
							{
							case '\0': // Remove pre-existing null-terminator
							case ' ':  // Remove space (malformated Path)
								newValue.pop_back();
								break;
							case ';': // Don't add a double ;
								break;
							default:
								newValue += ';';
								break;
							}
						}

						newValue += stringPath + ";";

						std::print( "Setting the new Registry Value...\n" );
						if ( const LONG set_res = RegSetValueExA( hKey, valueName, 0, valueType, reinterpret_cast<const BYTE*>( newValue.c_str() ), newValue.size() + 1 ); set_res != ERROR_SUCCESS )
						{
							RegCloseKey( hKey );
							std::print( "Failed to set new value in Registry! Ensure you have the necessary permissions.\n" );
							return false;
						}
					}
					else
					{
						std::print( "{} is already in the Path variable!\n", stringPath );
						std::print( "\n{}\n\n", newValue );
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
		std::print( "Successfully added {} to the path!\n", stringPath );
		RegCloseKey( hKey );
		return true;
	}
}
