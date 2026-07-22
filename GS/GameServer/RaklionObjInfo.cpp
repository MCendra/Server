// RaklionObjInfo.cpp: implementation of the CRaklionObjInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RaklionObjInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRaklionObjInfo::CRaklionObjInfo()
{
	this->Reset();
}

CRaklionObjInfo::~CRaklionObjInfo()
{

}

void CRaklionObjInfo::Reset()
{
	this->m_ObjCount = 0;

	for(int n=0;n < MAX_RAKLION_OBJ_INDEX;n++)
	{
		this->m_ObjIndex[n] = -1;
	}
}

bool CRaklionObjInfo::AddObj(int aIndex)
{
	if(this->m_ObjCount < 0 || this->m_ObjCount >= MAX_RAKLION_OBJ_INDEX)
	{
		return 0;
	}

	this->m_ObjIndex[this->m_ObjCount++] = aIndex;
	return 1;
}

int CRaklionObjInfo::GetCount()
{
	return this->m_ObjCount;
}
