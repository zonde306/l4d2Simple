#include "MRecipientFilter.h"
#include "../interfaces.h"

MRecipientFilter::MRecipientFilter(void)
{
}

MRecipientFilter::~MRecipientFilter(void)
{
}

int MRecipientFilter::GetRecipientCount() const
{
	return m_Recipients.Size();
}

int MRecipientFilter::GetRecipientIndex(int slot) const
{
	if ( slot < 0 || slot >= GetRecipientCount() )
		return -1;

	return m_Recipients[ slot ];
}

bool MRecipientFilter::IsInitMessage() const
{
	return false;
}

bool MRecipientFilter::IsReliable() const
{
	return false;
}

void MRecipientFilter::AddAllPlayers(int maxClients)
{
	m_Recipients.RemoveAll();
	for ( int i = 1; i <= maxClients; i++ )
	{
		CBasePlayer* pPlayer = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(i));
		if (!pPlayer)
			continue;

		//AddRecipient( pPlayer );
		m_Recipients.AddToTail(i);
	}
} 
void MRecipientFilter::AddRecipient( int iPlayer )
{
	// Already in list
	if ( m_Recipients.Find( iPlayer ) != m_Recipients.InvalidIndex() )
		return;

	m_Recipients.AddToTail( iPlayer );
}

void MRecipientFilter::RemoveRecipient(int iPlayer)
{
	m_Recipients.FindAndRemove(iPlayer);
}

void MRecipientFilter::RemoveAllRecipients()
{
	for(int i = 1; i < m_Recipients.Count(); i++)
	{
		m_Recipients.Remove(i);
	}
}