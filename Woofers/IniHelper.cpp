#include "IniHelper.h"

#include <stdlib.h>

namespace Woofers
{
	IniHelper* IniHelper::_instance = 0;
	bool IniHelper::_initialized = false;

	/////////////////////////
	//	Public Methods
	/////////////////////////

	IniHelper::IniHelper(const std::string iniFile, const bool cacheAll) : 
		_targetIniFile(iniFile),
		_isCached(cacheAll)
	{
		if (_isCached)
		{
			// Read the ini file once and store contents in memory
			ini_browse(&BrowseCallback, (void*)&_cachedData, _targetIniFile.c_str());
		}
	}

	IniHelper::~IniHelper()
	{
	}

	IniHelper& IniHelper::Instance()
	{
		if (!_initialized)
		{
			// Singleton class has not been initialized, do so now
			_instance = new IniHelper(WOOFER_INI_FILE, true);

			// Call CleanUp when the program exits
			atexit(&CleanUp);
			_initialized = true;
		}

		return *_instance;
	}

	const bool IniHelper::GetBool(const char* section, const char* key)
	{
		bool result = DEFAULT_BOOL;

		if (_isCached)
		{
			// ini in memory
			std::string temp = _cachedData[section][key];
			if (temp == "1" || temp == "T" || temp == "t" || temp == "Y" || temp == "y")
			{
				result = true;
			}
			else if (temp == "0" || temp == "F" || temp == "f" || temp == "N" || temp == "n")
			{
				result = false;
			}
		}
		else
		{
			result = ini_getbool(section, key, DEFAULT_BOOL, _targetIniFile.c_str());
		}

		return result;
	}

	const long IniHelper::GetLong(const char* section, const char* key)
	{
		long result;

		if (_isCached)
		{
			// ini in memory
			result = std::atol(_cachedData[section][key].c_str());
		}
		else
		{
			result = ini_getl(section, key, DEFAULT_LONG, _targetIniFile.c_str());
		}

		return result;
	}

	const std::string IniHelper::GetString(const char* section, const char* key)
	{
		std::string result;

		if (_isCached)
		{
			// ini in memory
			result = _cachedData[section][key];
		}
		else
		{
			char buffer[255];
			ini_gets(section, key, DEFAULT_STRING, buffer, 255, _targetIniFile.c_str());
			result = buffer;
		}

		return result;
	}

	const float IniHelper::GetFloat(const char* section, const char* key)
	{
		float result;

		if (_isCached)
		{
			// ini in memory
			result = std::atof(_cachedData[section][key].c_str());
		}
		else
		{
			result = ini_getf(section, key, DEFAULT_FLOAT, _targetIniFile.c_str());
		}

		return result;
	}

	/////////////////////////
	//	Private Methods
	/////////////////////////


	void IniHelper::CleanUp()
	{
		delete _instance;
		_instance = 0;
		_initialized = false;
	}

	int IniHelper::BrowseCallback(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *Value, const void *UserData)
	{
		// Store the section and key inside of our collection
		IniDictionary* cachedData = ((IniDictionary*)UserData);
		(*cachedData)[std::string(Section)][std::string(Key)] = std::string(Value);
		return 1;
	}
}