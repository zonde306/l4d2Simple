#include "config.h"
#include "utils.h"
#include "xorstr.h"

#include <regex>

std::unique_ptr<CProfile> g_pConfig;

CProfile::CProfile() { }

CProfile::CProfile(const std::string& path)
{
	OpenFile(path);
	LoadFromFile();
}

CProfile::CProfile(const CProfile& other)
{
	m_sFileName = other.m_sFileName;
	m_KeyValue = other.m_KeyValue;
}

CProfile::~CProfile()
{
	if (m_File.is_open())
		CloseFile();

	m_KeyValue.clear();
}

CProfile& CProfile::operator=(const CProfile& other)
{
	m_KeyValue = other.m_KeyValue;
	return *this;
}

bool CProfile::OpenFile(const std::string& path, bool write)
{
	if (path.empty())
		return false;

	m_sFileName = path;

	if (m_File.is_open())
		CloseFile();

	if (write)
		m_File.open(path, std::ios::out | std::ios::beg | std::ios::trunc);
	else
		m_File.open(path, std::ios::in | std::ios::beg);

	if (m_File.bad() || !m_File.is_open())
	{
		m_File.open(path, std::ios::out | std::ios::in | std::ios::beg | std::ios::trunc);
		m_bFileMode = true;
		return false;
	}

	m_bFileMode = write;
	return true;
}

bool CProfile::CloseFile()
{
	if (!m_File.is_open())
		return false;

	m_File.close();
	return true;
}

bool CProfile::SaveToFile()
{
	if (!m_File.is_open() || !m_bFileMode)
	{
		CloseFile();
		OpenFile(m_sFileName, true);
	}

	m_File.seekp(std::ios::beg);
	bool success = Save(m_File);
	m_File.flush();

	return success;
}

bool CProfile::LoadFromFile()
{
	if (!m_File.is_open() || m_bFileMode)
	{
		CloseFile();
		OpenFile(m_sFileName, false);
	}

	m_KeyValue.clear();
	m_File.seekg(std::ios::beg);

	return Load(m_File);
}

bool CProfile::Save(std::ostream& stream) const
{
	for (const auto& mk : m_KeyValue)
	{
		if (mk.first.empty() || mk.second.empty())
			continue;

		stream << '[' << mk.first << ']' << std::endl;

		for (const auto& kv : mk.second)
		{
			if (kv.first.empty())
				continue;

			if (kv.second.m_sValue.empty())
				stream << kv.first << std::endl;
			else if (kv.first.find('=') != std::string::npos || kv.second.m_sValue.find('=') != std::string::npos)
				stream << '\"' << kv.first << "\" = \"" << kv.second.m_sValue << '\"' << std::endl;
			else
				stream << kv.first << " = " << kv.second.m_sValue << std::endl;
		}

		stream << std::endl;
	}

	return true;
}

bool CProfile::Load(std::istream& stream)
{
	char buffer[255];

	std::string line, key, value, mainKey;
	std::regex re_ignore = std::regex(XorStr("/\\*.*\\*/"));

	while (stream.good() && !stream.eof())
	{
		stream.getline(buffer, 255);

		if (buffer[0] == '\0')
			continue;

		line = buffer;
		line = std::regex_replace(line, re_ignore, "");

		size_t here = line.rfind(';');

		if (here != std::string::npos)
		{
			line = line.substr(0, here);
		}

		line = Utils::StringTrim(line, XorStr(" \t\r\n"));

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

		here = ParseKeyValue(line);

		if (here != std::string::npos)
		{
			key = line.substr(0, here);
			value = line.substr(here + 1);
		}
		else
		{
			key = line;
			value.clear();
		}

		key = Utils::StringTrim(key, XorStr(" \t\r\n"));
		value = Utils::StringTrim(value, XorStr(" \t\r\n"));

		if (key[0] == '"' && key[key.length() - 1] == '"')
			key = key.substr(1, key.length() - 2);
		if (value[0] == '"' && value[value.length() - 1] == '"')
			value = value.substr(1, value.length() - 2);

		m_KeyValue[mainKey][key] = value;
	}

	return !m_KeyValue.empty();
}

void CProfile::Clear()
{
	m_KeyValue.clear();
}

bool CProfile::HasMainKey(const std::string& mainKeys) const
{
	return (m_KeyValue.find(mainKeys) != m_KeyValue.end());
}

bool CProfile::HasKey(const std::string& mainKeys, const std::string& keys)
{
	if (!HasMainKey(mainKeys))
		return false;

	return (m_KeyValue[mainKeys].find(keys) != m_KeyValue[mainKeys].end());
}

bool CProfile::EraseKey(const std::string& mainKeys, const std::string& keys)
{
	if (!HasKey(mainKeys, keys))
		return false;

	m_KeyValue[mainKeys].erase(keys);

	if (m_KeyValue[mainKeys].empty())
		m_KeyValue.erase(mainKeys);

	return true;
}

void CProfile::SetValue(const std::string& mainKeys, const std::string& keys, const std::string& value)
{
	if (HasKey(mainKeys, keys))
		m_KeyValue[mainKeys][keys].SetValue(value);
	else
		m_KeyValue[mainKeys][keys] = value;
}

void CProfile::SetValue(const std::string& mainKeys, const std::string& keys, int value)
{
	if (HasKey(mainKeys, keys))
		m_KeyValue[mainKeys][keys].SetValue(value);
	else
		m_KeyValue[mainKeys][keys] = value;
}

void CProfile::SetValue(const std::string& mainKeys, const std::string& keys, float value)
{
	if (HasKey(mainKeys, keys))
		m_KeyValue[mainKeys][keys].SetValue(value);
	else
		m_KeyValue[mainKeys][keys] = value;
}

std::string CProfile::GetString(const std::string& mainKeys, const std::string& keys, const std::string& def)
{
	if (!HasKey(mainKeys, keys))
	{
		m_KeyValue[mainKeys][keys] = def;
		return def;
	}

	return m_KeyValue[mainKeys][keys].m_sValue;
}

int CProfile::GetInteger(const std::string& mainKeys, const std::string& keys, int def)
{
	if (!HasKey(mainKeys, keys))
	{
		m_KeyValue[mainKeys][keys] = def;
		return def;
	}

	return m_KeyValue[mainKeys][keys].m_iValue;
}

float CProfile::GetFloat(const std::string& mainKeys, const std::string& keys, float def)
{
	if (!HasKey(mainKeys, keys))
	{
		m_KeyValue[mainKeys][keys] = def;
		return def;
	}

	return m_KeyValue[mainKeys][keys].m_fValue;
}

bool CProfile::GetBoolean(const std::string& mainKeys, const std::string& keys, bool def)
{
	if (!HasKey(mainKeys, keys))
	{
		m_KeyValue[mainKeys][keys] = def;
		return def;
	}

	return (m_KeyValue[mainKeys][keys].m_iValue > 0);
}

typename CProfile::KeyValueType& CProfile::GetMainKey(const std::string& mainKeys)
{
	return m_KeyValue[mainKeys];
}

size_t CProfile::ParseKeyValue(const std::string& text)
{
	bool inGroup = false;

	for (size_t i = 0; i < text.length(); ++i)
	{
		if (text[i] == '"')
			inGroup = !inGroup;

		if (inGroup)
			continue;

		if (text[i] == '=')
			return i;
	}

	return std::string::npos;
}

std::pair<std::string, std::string> CProfile::ParseKeyValueEx(const std::string& text)
{
	size_t trim = ParseKeyValue(text);

	if (trim == std::string::npos)
		return std::make_pair(text, "");

	return std::make_pair(Utils::StringTrim(text.substr(0, trim)), Utils::StringTrim(text.substr(trim + 1)));
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

typename CProfile::iterator2 CProfile::begin(const std::string& mainKeys)
{
	return m_KeyValue[mainKeys].begin();
}

typename CProfile::iterator2 CProfile::end(const std::string& mainKeys)
{
	return m_KeyValue[mainKeys].end();
}

typename CProfile::const_iterator2 CProfile::begin(const std::string& mainKeys) const
{
	return m_KeyValue.at(mainKeys).cbegin();
}

typename CProfile::const_iterator2 CProfile::end(const std::string& mainKeys) const
{
	return m_KeyValue.at(mainKeys).cend();
}

CProfile::_KeyValues::_KeyValues() : m_iValue(0), m_fValue(0.0f), m_sValue("") { }

CProfile::_KeyValues::_KeyValues(int value)
{
	SetValue(value);
}

CProfile::_KeyValues::_KeyValues(float value)
{
	SetValue(value);
}

CProfile::_KeyValues::_KeyValues(const std::string& value)
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

void CProfile::_KeyValues::SetValue(const std::string& value)
{
	m_iValue = atoi(value.c_str());
	m_fValue = static_cast<float>(atof(value.c_str()));
	m_sValue = value;
}