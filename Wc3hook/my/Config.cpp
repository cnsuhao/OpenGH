#include "stdafx.h"
#include "Config.h"

using namespace std;
using namespace ConfigMan;

Config::Config(const char *config_file)
{
	ifstream inputFile(config_file);
	if(!inputFile.is_open())
		cout << "Error opening config file with name: "
			 << config_file
			 << " !";

	string line,
		   var_name, 
		   var_value;
	size_t help_pos;

	while(!inputFile.eof())
	{
		getline(inputFile, line);
		
		if(line[0] == '#' || line == "")
			continue;
		
		help_pos = line.find('=');

		copy(line.begin(), line.begin() + help_pos, inserter(var_name, var_name.begin()));
		var_name.erase(remove_if(var_name.begin(), var_name.end(), isspace), var_name.end());

		if(line[++help_pos]==' ')
			help_pos = line.find_first_not_of(' ', help_pos);
		
		if(line[help_pos] == '\"')
		{
			++help_pos;
			copy(line.begin() + help_pos, 
					line.begin() + line.find_last_of('\"'), 
					inserter(var_value, var_value.begin()));
		}
		else
		{
			copy(line.begin() + help_pos, 
					line.begin() + line.find_last_not_of(' ') + 1,
					inserter(var_value, var_value.begin()));
		}

		file_data.insert(make_pair(var_name, var_value));
		var_name = var_value = "";
	}
}


template<>
bool Config::getValueAs(const string &var)
{
	if(_stricmp(file_data[var].c_str(),"true")==0)
		return true;
	else 
		return false;
}

template<>
string Config::getValueAs(const string &var)
{
	return file_data[var];
}

template<>
int Config::getValueAs(const string &var)
{
	return atoi(file_data[var].c_str());
}

template<>
double Config::getValueAs(const string &var)
{
	return atof(file_data[var].c_str());
}


