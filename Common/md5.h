// MD5.h
#ifndef MD5_H
#define MD5_H
#pragma once
#include <string>
using namespace std;


union _tagMd5;



class CMD5  
{
// Construction
public:
	CMD5();
	virtual ~CMD5();
	
// Attributes
public:
	unsigned long	m_nCount[2];
	unsigned long	m_nState[4];
	unsigned char	m_nBuffer[64];

// Operations
public:
	void			Reset();
	void			Add( const void* pData, unsigned long nLength);
	void			Finish();
	void			GetHash(_tagMd5* pHash);

public:
	static string	HashToString(const _tagMd5* pHash );
	static bool		HashFromString(const char* pszHash, _tagMd5* pMD5);
	static bool		HashFromURN(const char* pszHash, _tagMd5* pMD5);

// Implementation
protected:
	void			Transform(unsigned char* pBlock);
	void			Encode(unsigned char* pOutput, unsigned long* pInput, unsigned long nLength);
	void			Decode(unsigned long* pOutput, unsigned char* pInput, unsigned long nLength);
};


typedef union _tagMd5
{
	unsigned char	n[16];
	unsigned char	b[16];
	unsigned long	w[4];
	_tagMd5& operator=(const _tagMd5& _Right)
	{
		for(int i = 0 ; i < 4 ; i++ )
		{
			w[i] = _Right.w[i];
		}
		return *this;
	}
	void Zero()
	{
		memset( w , 0, sizeof(w) );
	}
	bool bIsValidMd5()
	{
		for(int i = 0 ; i < 4 ; i++ )
		{
			if( 0 != w[i] )
				return true;
		}
		return false;
	}
	_tagMd5& operator=( const char* pMdString )
	{
		if( false == CMD5::HashFromString( pMdString ,this ) )
		{
			for(int i = 0 ; i < 4 ; i++ )
			{
				w[i] = 0;
			}
		}
		return *this;
	}
	bool operator==(const _tagMd5& _Right)
	{
		for(int i = 0 ; i < 4 ; i++ )
		{
			if( w[i] != _Right.w[i] ) 
				return false;
		}
		return true;
	}
	bool operator<(const _tagMd5& _Right)
	{
		for(int i = 0 ; i < 4 ; i++ )
		{
			if( w[i] > _Right.w[i] ) 
				return false;
			if( w[i] == _Right.w[i] )
				continue;

			return true;
		}
		return false;
	}

} MD4, MD5;

inline bool operator < (const _tagMd5& _Left ,const _tagMd5& _Right)
{
	for(int i = 0 ; i < 4 ; i++ )
	{
		if( _Left.w[i] > _Right.w[i] ) 
			return false;
		if( _Left.w[i] == _Right.w[i] )
			continue;

		return true;
	}
	return false;
}

#endif