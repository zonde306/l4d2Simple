#include "IBaseClientState.h"

bool NET_SetConVar::ReadFromBuffer(bf_read& buffer)
{
	int numvars = buffer.ReadByte();

	m_ConVars.RemoveAll();

	for (int i = 0; i < numvars; i++)
	{
		cvar_t cvar;
		buffer.ReadString(cvar.name, sizeof(cvar.name));
		buffer.ReadString(cvar.value, sizeof(cvar.value));
		m_ConVars.AddToTail(cvar);

	}
	return !buffer.IsOverflowed();
}

bool NET_SetConVar::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS, true);

	int numvars = m_ConVars.Count();

	// Note how many we're sending
	buffer.WriteByte(numvars);

	for (int i = 0; i < numvars; i++)
	{
		cvar_t* cvar = &m_ConVars[i];
		buffer.WriteString(cvar->name);
		buffer.WriteString(cvar->value);
	}

	return !buffer.IsOverflowed();
}

bool CLC_RespondCvarValue::ReadFromBuffer(bf_read& buffer)
{
	m_iCookie = buffer.ReadSBitLong(32);
	m_eStatusCode = (EQueryCvarValueStatus)buffer.ReadSBitLong(4);

	// Read the name.
	buffer.ReadString(m_szCvarNameBuffer, sizeof(m_szCvarNameBuffer));
	m_szCvarName = m_szCvarNameBuffer;

	// Read the value.
	buffer.ReadString(m_szCvarValueBuffer, sizeof(m_szCvarValueBuffer));
	m_szCvarValue = m_szCvarValueBuffer;

	return !buffer.IsOverflowed();
}

bool CLC_RespondCvarValue::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	buffer.WriteSBitLong(m_iCookie, 32);
	buffer.WriteSBitLong(m_eStatusCode, 4);

	buffer.WriteString(m_szCvarName);
	buffer.WriteString(m_szCvarValue);

	return !buffer.IsOverflowed();
}

/*
bool CLC_FileCRCCheck::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	// Reserved for future use.
	buffer.WriteOneBit(0);

	// Just write a couple bits for the path ID if it's one of the common ones.
	int iCode = FindCommonPathID(m_szPathID);
	if (iCode == -1)
	{
		buffer.WriteUBitLong(0, 2);
		buffer.WriteString(m_szPathID);
	}
	else
	{
		buffer.WriteUBitLong(iCode + 1, 2);
	}

	iCode = FindCommonPrefix(m_szFilename);
	if (iCode == -1)
	{
		buffer.WriteUBitLong(0, 3);
		buffer.WriteString(m_szFilename);
	}
	else
	{
		buffer.WriteUBitLong(iCode + 1, 3);
		buffer.WriteString(&m_szFilename[strlen(g_MostCommonPrefixes[iCode]) + 1]);
	}

	buffer.WriteUBitLong(m_CRC, 32);
	return !buffer.IsOverflowed();
}

bool CLC_FileCRCCheck::ReadFromBuffer(bf_read& buffer)
{
	// Reserved for future use.
	buffer.ReadOneBit();

	// Read the path ID.
	int iCode = buffer.ReadUBitLong(2);
	if (iCode == 0)
	{
		buffer.ReadString(m_szPathID, sizeof(m_szPathID));
	}
	else if ((iCode - 1) < ARRAYSIZE(g_MostCommonPathIDs))
	{
		V_strncpy(m_szPathID, g_MostCommonPathIDs[iCode - 1], sizeof(m_szPathID));
	}
	else
	{
		Assert(!"Invalid path ID code in CLC_FileCRCCheck");
		return false;
	}

	// Read the filename.
	iCode = buffer.ReadUBitLong(3);
	if (iCode == 0)
	{
		buffer.ReadString(m_szFilename, sizeof(m_szFilename));
	}
	else if ((iCode - 1) < ARRAYSIZE(g_MostCommonPrefixes))
	{
		char szTemp[MAX_PATH];
		buffer.ReadString(szTemp, sizeof(szTemp));
		V_snprintf(m_szFilename, sizeof(m_szFilename), "%s%c%s", g_MostCommonPrefixes[iCode - 1], CORRECT_PATH_SEPARATOR, szTemp);
	}
	else
	{
		Assert(!"Invalid prefix code in CLC_FileCRCCheck.");
		return false;
	}

	m_CRC = buffer.ReadUBitLong(32);

	return !buffer.IsOverflowed();
}
*/

bool CLC_ListenEvents::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	int count = MAX_EVENT_NUMBER / 32;
	for (int i = 0; i < count; ++i)
	{
		buffer.WriteUBitLong(m_EventArray.GetDWord(i), 32);
	}

	return !buffer.IsOverflowed();
}

bool CLC_ListenEvents::ReadFromBuffer(bf_read& buffer)
{
	int count = MAX_EVENT_NUMBER / 32;
	for (int i = 0; i < count; ++i)
	{
		m_EventArray.SetDWord(i, buffer.ReadUBitLong(32));
	}

	return !buffer.IsOverflowed();
}

bool CLC_ClientInfo::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	buffer.WriteLong(m_nServerCount);
	buffer.WriteLong(m_nSendTableCRC);
	buffer.WriteOneBit(m_bIsHLTV ? 1 : 0);
	buffer.WriteLong(m_nFriendsID);
	buffer.WriteString(m_FriendsName);

	for (int i = 0; i < MAX_CUSTOM_FILES; i++)
	{
		if (m_nCustomFiles[i] != 0)
		{
			buffer.WriteOneBit(1);
			buffer.WriteUBitLong(m_nCustomFiles[i], 32);
		}
		else
		{
			buffer.WriteOneBit(0);
		}
	}

	return !buffer.IsOverflowed();
}

bool CLC_ClientInfo::ReadFromBuffer(bf_read& buffer)
{
	m_nServerCount = buffer.ReadLong();
	m_nSendTableCRC = buffer.ReadLong();
	m_bIsHLTV = buffer.ReadOneBit() != 0;
	m_nFriendsID = buffer.ReadLong();
	buffer.ReadString(m_FriendsName, sizeof(m_FriendsName));

	for (int i = 0; i < MAX_CUSTOM_FILES; i++)
	{
		if (buffer.ReadOneBit() != 0)
		{
			m_nCustomFiles[i] = buffer.ReadUBitLong(32);
		}
		else
		{
			m_nCustomFiles[i] = 0;
		}
	}


	return !buffer.IsOverflowed();
}
