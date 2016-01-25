//credits to Kiril Kirov or writing the config class <3
#ifndef CONFIG_H
#define CONFIG_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
using namespace std;

namespace ConfigMan
{
	class Config
	{
	public:
		static Config& Instance(const char* config_file)
		{
			static Config config(config_file);
			return config;
		}
		template<typename T>
		T getValueAs(const string &var);

	private:
		map<string, string> file_data;

	private:
		Config(const char *config_file);
		Config(const Config&);
		Config& operator=(const Config&);
	};
}

#endif