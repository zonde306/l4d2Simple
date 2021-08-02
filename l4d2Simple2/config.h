#pragma once
#include <string>
#include <fstream>
#include <map>
#include <memory>
#include <iostream>

class CProfile
{
public:
	CProfile();
	CProfile(const std::string& path);
	CProfile(const CProfile& other);
	~CProfile();

	CProfile& operator=(const CProfile& other);

	// 配置文件
	bool OpenFile(const std::string& path, bool write = false);
	bool CloseFile();
	bool SaveToFile();
	bool LoadFromFile();
	bool Save(std::ostream& stream) const;
	bool Load(std::istream& stream);
	void Clear();

	// 寻找主键
	bool HasMainKey(const std::string& mainKeys) const;
	
	// 寻找键
	bool HasKey(const std::string& mainKeys, const std::string& keys);

	// 删除键
	bool EraseKey(const std::string& mainKeys, const std::string& keys);

	// 获取/设置值
	void SetValue(const std::string& mainKeys, const std::string& keys, const std::string& value);
	void SetValue(const std::string& mainKeys, const std::string& keys, int value);
	void SetValue(const std::string& mainKeys, const std::string& keys, float value);
	std::string GetString(const std::string& mainKeys, const std::string& keys, const std::string& def = "");
	int GetInteger(const std::string& mainKeys, const std::string& keys, int def = 0);
	float GetFloat(const std::string& mainKeys, const std::string& keys, float def = 0.0f);
	bool GetBoolean(const std::string& mainKeys, const std::string& keys, bool def = false);

	struct _KeyValues
	{
	public:
		int m_iValue;
		float m_fValue;
		std::string m_sValue;

		_KeyValues();
		_KeyValues(int value);
		_KeyValues(float value);
		_KeyValues(const std::string& value);

		void SetValue(int value);
		void SetValue(float value);
		void SetValue(const std::string& value);

		/*
		inline operator int&() { return m_iValue; };
		inline operator float&() { return m_fValue; };
		inline operator std::string&() { return m_sValue; };
		*/
	};

	using KeyValueType = std::map<std::string, _KeyValues>;
	using MainKeyValueType = std::map<std::string, KeyValueType>;

	KeyValueType& GetMainKey(const std::string& mainKeys);
	static size_t ParseKeyValue(const std::string& text);
	static std::pair<std::string, std::string> ParseKeyValueEx(const std::string& text);

public:
	using iterator = MainKeyValueType::iterator;
	using const_iterator = MainKeyValueType::const_iterator;

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;

	using iterator2 = KeyValueType::iterator;
	using const_iterator2 = KeyValueType::const_iterator;

	iterator2 begin(const std::string& mainKeys);
	iterator2 end(const std::string& mainKeys);
	const_iterator2 begin(const std::string& mainKeys) const;
	const_iterator2 end(const std::string& mainKeys) const;

private:
	bool m_bFileMode;
	std::fstream m_File;
	MainKeyValueType m_KeyValue;
	std::string m_sFileName;
};

extern std::unique_ptr<CProfile> g_pConfig;
