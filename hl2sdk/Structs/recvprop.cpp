#include "recvprop.h"

inline DVariant::DVariant() 
{
	m_Type = DPT_Float; 
}

inline DVariant::DVariant(float val) 
{
	m_Type = DPT_Float;
	m_Float = val; 
}

inline const char* DVariant::ToString()
{
	static char text[128];

	switch (m_Type)
	{
	case DPT_Int:
		_snprintf(text, sizeof(text), "%i", (int)m_Int);
		break;
	case DPT_Float:
		_snprintf(text, sizeof(text), "%.3f", m_Float);
		break;
	case DPT_Vector:
		_snprintf(text, sizeof(text), "(%.3f,%.3f,%.3f)",
			m_Vector[0], m_Vector[1], m_Vector[2]);
		break;
	case DPT_VectorXY:
		_snprintf(text, sizeof(text), "(%.3f,%.3f)",
			m_Vector[0], m_Vector[1]);
		break;
#if 0
	case DPT_Quaternion:
		Q_snprintf(text, sizeof(text), "(%.3f,%.3f,%.3f %.3f)",
			m_Vector[0], m_Vector[1], m_Vector[2], m_Vector[3]);
		break;
#endif
	case DPT_String:
		if (m_pString)
			return m_pString;
		else
			return "NULL";
		break;
	case DPT_Array:
		_snprintf(text, sizeof(text), "Array");
		break;
	case DPT_DataTable:
		_snprintf(text, sizeof(text), "DataTable");
		break;
	case DPT_Int64:
		_snprintf(text, sizeof(text), "%lld", m_Int64);
		break;
	default:
		_snprintf(text, sizeof(text), "DVariant type %i unknown", m_Type);
		break;
	}

	return text;
}