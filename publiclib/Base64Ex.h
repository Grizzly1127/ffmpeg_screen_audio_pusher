/************************************
*comment:Base64编码解码
*auth:huangyubin
*date:2015.12.30
*************************************/

#ifndef __BASE64_EX_H__
#define __BASE64_EX_H__

#include "def.h"

class EXPORT_FUNCTION Base64Ex
{
public:
	Base64Ex(const char* pszSrc, const unsigned char modeType, int nSrcLen = 0);
	~Base64Ex();

	const static unsigned char modeEncode = '1';
	const static unsigned char modeDecode = '2';
	const char* GetResult();
	const int GetLen();

private:
	char* m_pResult;
	unsigned char m_byteModeType;
	int m_nLen;

	int EncodeBase64(const unsigned char* pSrc, char* pDst, int nSrcLen, int nMaxLineLen);
	int DecodeBase64(const char* pSrc, unsigned char* pDst, int nSrcLen);
};


//s1加解密
//class EXPORT_FUNCTION EnDecryptionEx
//{
//public:
//	EnDecryptionEx(string strSrc, const unsigned char modeType);
//	~EnDecryptionEx();
//
//	const static unsigned char modeEncode = '1';
//	const static unsigned char modeDecode = '2';
//	string GetResult();
//
//private:
//	string m_strResult;
//	unsigned char m_byteModeType;
//
//	bool Encryption(const char *pKey, const char *pText, unsigned char *pResult);
//	bool Decryption(const char *pKey, const  unsigned char* pDate, int ndataLen, char *pResult);
//};


//客户端
#define S1_KEY_ECLASS_CLIENT "DAOJFML27MZ6CJVWAFP3"
//后台管理系统
#define S1_KEY_CMS  "VQEO1RU34QLZ1A3QL06"
//eclass协议
#define S1_KEY_ECLASS_PROTOCOL  "f8b668aca4a50273a"

const char g_pKey[] = "f8b668aca4a50273a";
string EXPORT_FUNCTION eclassDecrypt(const char* pText, const char* pKey);
string EXPORT_FUNCTION eclassEncrypt(const char* pText, const char* pKey);
bool Encryption(const char *pKey, const char *pText, unsigned char *pResult);
bool Decryption(const char *pKey, const  unsigned char* pDate, int ndataLen, char *pResult);

#endif