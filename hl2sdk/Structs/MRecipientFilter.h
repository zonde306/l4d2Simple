#ifndef _MRECIPIENT_FILTER_H
#define _MRECIPIENT_FILTER_H
#include "../Interfaces/IEngineSound.h"
#include "../Utils/utlvector.h"

class MRecipientFilter : public IRecipientFilter
{
public:
	MRecipientFilter(void);
	~MRecipientFilter(void);

	virtual bool IsReliable( void ) const;
	virtual bool IsInitMessage( void ) const;

	virtual int GetRecipientCount( void ) const;
	virtual int GetRecipientIndex( int slot ) const;
	void AddAllPlayers( int maxClients );
	void AddRecipient (int iPlayer );
	void RemoveRecipient(int iPlayer);
	void RemoveAllRecipients();

private:
	bool m_bReliable;
	bool m_bInitMessage;
	CUtlVector< int > m_Recipients;
};

#endif