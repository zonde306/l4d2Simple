#include "config.h"
#include "utils.h"
#include "xorstr.h"
#include <regex>

std::unique_ptr<CProfile> g_pConfig;

CProfile::CProfile()
{
}

CProfile::CProfile(const std::string & path)
{
	OpenFile(path);
}

CProfile::~CProfile()
{
	if (m_File.is_open())
		CloseFile();
	
	m_KeyValue.clear();
}

bool CProfile::OpenFile(const std::string & path)
{
	if (m_File.is_open())
		CloseFile();
	
	m_File.open(path, std::ios::in|std::ios::out|std::ios::beg);
	if (m_File.bad() || !m_File.is_open())
		return false;

	m_KeyValue.clear();

	char buffer[255];
	std::string line, key, value, mainKey;

	// 块注释
	std::regex re_ignore = std::regex(XorStr("/\\*.*\\*/"));

	while (m_File.good() && !m_File.eof())
	{
		m_File.getline(buffer, 255);
		if (buffer[0] == '\0')
			continue;

		line = buffer;

		// 清除块注释
		line = std::regex_replace(line, re_ignore, "");

		size_t here = line.rfind(';');
		if (here != std::string::npos)
		{
			// 清除单行注释
			line = line.substr(0, here);
		}

		// 去除无效字符
		line = Utils::Trim(line, XorStr(" \t\r\n"));

		if (line.empty() || line[0] == ';')
			continue;

		if (line[0] == '/' && line[1] == '/')
			continue;

		if (line[0] == '[' && line[line.length() - 1] == ']')
		{
			mainKey = line.substr(1, line.length() - 2);
			continue;
		}

		if (mainKey.empty())
			continue;

		here = line.find('=');
		if (here == std::string::npos)
			continue;

		key = line.substr(0, here);
		value = line.substr(here + 1);
		key = Utils::Trim(key, XorStr(" \t\r\n"));
		value = Utils::Trim(value, XorStr(" \t\r\n"));

		// 去除两侧多余的双引号
		if (key[0] == '"' && key[key.length() - 1] == '"')
			key = key.substr(1, key.length() - 2);
		if (value[0] == '"' && value[value.length() - 1] == '"')
			value = value.substr(1, value.length() - 2);

		m_KeyValue[mainKey][key] = value;
	}

	return true;
}

bool CProfile::CloseFile()
{
	if (!m_File.is_open())
		return false;
	
	m_File.seekp(std::ios::beg);

	for (const auto& mk : m_KeyValue)
	{
		m_File << '[' << mk.first << ']' << std::endl;
		
		for (const auto& kv : mk.second)
		{
			m_File << kv.first << " = " << kv.second.m_sValue << std::endl;
		}

		m_File << std::endl;
	}

	m_File.close();
	return true;
}

bool CProfile::SaveToFile()
{
	if (!m_File.is_open())
		return false;
	
	m_File.flush();
	return true;
}

bool CProfile::HasMainKey(const std::string & mainKeys)
{
	return (m_KeyValue.find(mainKeys) != m_KeyValue.end());
}

bool CProfile::HasKey(const std::string & mainKeys, const std::string & keys)
{
	if (!HasMainKey(mainKeys))
		return false;

	return (m_KeyValue[mainKeys].find(keys) != m_KeyValue[mainKeys].end());
}

bool CProfile::EraseKey(const std::string & mainKeys, const std::string & keys)
{
	if(!HasKey(mainKeys, keys))
		return false;

	m_KeyValue[mainKeys].erase(keys);
	if (m_KeyValue[mainKeys].empty())
		m_KeyValue.erase(mainKeys);

	return true;
}

void CProfile::SetValue(const std::string & mainKeys, const std::string & keys, const std::string & value)
{
	if (HasKey(mainKeys, keys))
		m_KeyValue[mainKeys][keys].SetValue(value);
	else
		m_KeyValue[mainKeys][keys] = value;
}

void CProfile::SetValue(const std::string & mainKeys, const std::string & keys, int value)
{
	if (HasKey(mainKeys, keys))
		m_KeyValue[mainKeys][keys].SetValue(value);
	else
		m_KeyValue[mainKeys][keys] = value;
}

void CProfile::SetValue(const std::string & mainKeys, const std::string & keys, float value)
{
	if (HasKey(mainKeys, keys))
		m_KeyValue[mainKeys][keys].SetValue(value);
	else
		m_KeyValue[mainKeys][keys] = value;
}

std::string CProfile::GetString(const std::string & mainKeys, const std::string & keys, const std::string& def)
{
	if (!HasKey(mainKeys, keys))
	{
		m_KeyValue[mainKeys][keys] = def;
		return def;
	}

	return m_KeyValue[mainKeys][keys].m_sValue;
}

int CProfile::GetInteger(const std::string & mainKeys, const std::string & keys, int def)
{
	if (!HasKey(mainKeys, keys))
	{
		m_KeyValue[mainKeys][keys] = def;
		return def;
	}
	
	return m_KeyValue[mainKeys][keys].m_iValue;
}

float CProfile::GetFloat(const std::string & mainKeys, const std::string & keys, float def)
{
	if (!HasKey(mainKeys, keys))
	{
		m_KeyValue[mainKeys][keys] = def;
		return def;
	}
	
	return m_KeyValue[mainKeys][keys].m_fValue;
}

bool CProfile::GetBoolean(const std::string & mainKeys, const std::string & keys, bool def)
{
	if (!HasKey(mainKeys, keys))
	{
		m_KeyValue[mainKeys][keys] = def;
		return def;
	}
	
	return (m_KeyValue[mainKeys][keys].m_iValue > 0);
}

typename CProfile::iterator CProfile::begin()
{
	return m_KeyValue.begin();
}

typename CProfile::iterator CProfile::end()
{
	return m_KeyValue.end();
}

typename CProfile::const_iterator CProfile::begin() const
{
	return m_KeyValue.cbegin();
}

typename CProfile::const_iterator CProfile::end() const
{
	return m_KeyValue.cend();
}

typename CProfile::iterator2 CProfile::begin(const std::string & mainKeys)
{
	return m_KeyValue[mainKeys].begin();
}

typename CProfile::iterator2 CProfile::end(const std::string & mainKeys)
{
	return m_KeyValue[mainKeys].end();
}

typename CProfile::const_iterator2 CProfile::begin(const std::string & mainKeys) const
{
	return m_KeyValue.at(mainKeys).cbegin();
}

typename CProfile::const_iterator2 CProfile::end(const std::string & mainKeys) const
{
	return m_KeyValue.at(mainKeys).cend();
}

CProfile::_KeyValues::_KeyValues() : m_iValue(0), m_fValue(0.0f), m_sValue("")
{
}

CProfile::_KeyValues::_KeyValues(int value)
{
	SetValue(value);
}

CProfile::_KeyValues::_KeyValues(float value)
{
	SetValue(value);
}

CProfile::_KeyValues::_KeyValues(const std::string & value)
{
	SetValue(value);
}

void CProfile::_KeyValues::SetValue(int value)
{
	m_iValue = value;
	m_fValue = static_cast<float>(value);
	m_sValue = std::to_string(value);
}

void CProfile::_KeyValues::SetValue(float value)
{
	m_iValue = static_cast<int>(value);
	m_fValue = value;
	m_sValue = std::to_string(value);
}

void CProfile::_KeyValues::SetValue(const std::string & value)
{
	m_iValue = atoi(value.c_str());
	m_fValue = static_cast<float>(atof(value.c_str()));
	m_sValue = value;
}
