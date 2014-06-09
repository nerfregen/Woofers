#ifndef INIHELPER_H
#define INIHELPER_H

#include <map>
#include <string>

#include "minini\minIni.h"

namespace Woofers
{
	const std::string WOOFER_INI_FILE = "Woofers.ini";

	/// <summary>
	/// Wrapper class for the minIni library. The singleton instance of this class can be retrieved with the Instance method.
	/// </summary>
	class IniHelper
	{
	private:
		typedef std::map<std::string, std::map<std::string, std::string>> IniDictionary;
		static IniHelper* _instance;
		static bool _initialized;
		IniDictionary _cachedData;
		std::string _targetIniFile;
		bool _isCached;

		static void CleanUp();
		static int BrowseCallback(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *Value, const void *UserData);

		// Not copyable
		IniHelper(IniHelper const&);
		IniHelper& operator=(IniHelper const &);
	public:
		// Default values when the section/key is not found
		const char* DEFAULT_STRING = "Error loading string";
		const bool DEFAULT_BOOL = false;
		const long DEFAULT_LONG = 0;
		const float DEFAULT_FLOAT = 0.0f;

		/// <summary>
		/// Get the default singleton instance.
		/// </summary>
		static IniHelper& Instance();

		/// <summary>
		/// Create an instance that reads the specified INI file.
		/// </summary>
		/// <param name="iniFile">The INI file to read.</param>
		/// <param name="cacheAll">Stores contents of INI file in memory if true, false otherwise.</param>
		IniHelper(const std::string iniFile, const bool cacheAll);
		~IniHelper();

		const bool GetBool(const char* section, const char* key);
		const long GetLong(const char* section, const char* key);
		const std::string GetString(const char* section, const char* key);
		const float GetFloat(const char* section, const char* key);
	};

}
#endif