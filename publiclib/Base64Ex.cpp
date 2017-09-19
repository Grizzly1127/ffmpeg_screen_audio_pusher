/************************************
*comment:Base64编码解码
*auth:huangyubin
*date:2015.12.30
*************************************/

#include "stdafx.h"
#include "Base64Ex.h"
#include <string>

Base64Ex::Base64Ex(const char* pszSrc, const unsigned char modeType, int nSrcLen)
: m_pResult(NULL)
, m_byteModeType(modeType)
, m_nLen(0)
{
	if (!pszSrc)
	{
		return;
	}

	int nLenSrc = (0 == nSrcLen) ? strlen(pszSrc) : nSrcLen;
	if (nLenSrc<=0)
	{
		return;
	}
	//编码
	if (modeType == modeEncode)
	{
		int len = nLenSrc * 2 + 3;
		m_pResult = new char[len];
		memset(m_pResult, 0x00, len);
		m_nLen = EncodeBase64((unsigned char*)pszSrc, m_pResult, nLenSrc, nLenSrc * 2);
		if (m_nLen>0 && m_nLen < nLenSrc * 2)
		{
			m_pResult[m_nLen] = '\0';
		}
	}
	else if (modeType == modeDecode)
	{
		int len = nLenSrc * 2 + 3;
		m_pResult = new char[len];
		memset(m_pResult, 0x00, len);
		m_nLen = DecodeBase64(pszSrc, (unsigned char*)m_pResult, nLenSrc);

		if (m_nLen>0 && m_nLen < len)
		{
			m_pResult[m_nLen] = '\0';
		}
	}
}


Base64Ex::~Base64Ex()
{
	if (m_pResult)
	{
 		delete []m_pResult;
 		m_pResult = NULL;
	}
}

const char EnBase64Tab[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
int Base64Ex::EncodeBase64(const unsigned char* pSrc, char* pDst, int nSrcLen, int nMaxLineLen)
{
	unsigned char c1, c2, c3;    // 输入缓冲区读出3个字节
	int nDstLen = 0;             // 输出的字符计数
	int nLineLen = 0;            // 输出的行长度计数
	int nDiv = nSrcLen / 3;      // 输入数据长度除以3得到的倍数
	int nMod = nSrcLen % 3;      // 输入数据长度除以3得到的余数

	// 每次取3个字节，编码成4个字符
	for (int i = 0; i < nDiv; i++)
	{
		// 取3个字节
		c1 = *pSrc++;
		c2 = *pSrc++;
		c3 = *pSrc++;

		// 编码成4个字符
		*pDst++ = EnBase64Tab[c1 >> 2];
		*pDst++ = EnBase64Tab[((c1 << 4) | (c2 >> 4)) & 0x3f];
		*pDst++ = EnBase64Tab[((c2 << 2) | (c3 >> 6)) & 0x3f];
		*pDst++ = EnBase64Tab[c3 & 0x3f];
		nLineLen += 4;
		nDstLen += 4;

		// 输出换行？
		if (nLineLen > nMaxLineLen - 4)
		{
			*pDst++ = '\r';
			*pDst++ = '\n';
			nLineLen = 0;
			nDstLen += 2;
		}
	}

	// 编码余下的字节
	if (nMod == 1)
	{
		c1 = *pSrc++;
		*pDst++ = EnBase64Tab[(c1 & 0xfc) >> 2];
		*pDst++ = EnBase64Tab[((c1 & 0x03) << 4)];
		*pDst++ = '=';
		*pDst++ = '=';
		nLineLen += 4;
		nDstLen += 4;
	}
	else if (nMod == 2)
	{
		c1 = *pSrc++;
		c2 = *pSrc++;
		*pDst++ = EnBase64Tab[(c1 & 0xfc) >> 2];
		*pDst++ = EnBase64Tab[((c1 & 0x03) << 4) | ((c2 & 0xf0) >> 4)];
		*pDst++ = EnBase64Tab[((c2 & 0x0f) << 2)];
		*pDst++ = '=';
		nDstLen += 4;
	}

	// 输出加个结束符
	*pDst = '\0';

	return nDstLen;
}

/*
Base64解码方法中，最简单的也是查表法：将64个可打印字符的值作为索引，查表得到的值（范围为0-63）依次连起来，拼凑成字节形式输出，就得到解码结果。
*/

const char DeBase64Tab[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	62,        // '+'
	0, 0, 0,
	63,        // '/'
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61,        // '0'-'9'
	0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,        // 'A'-'Z'
	0, 0, 0, 0, 0, 0,
	26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,        // 'a'-'z'
};

int Base64Ex::DecodeBase64(const char* pSrc, unsigned char* pDst, int nSrcLen)
{
	int nDstLen;            // 输出的字符计数
	int nValue;             // 解码用到的长整数
	int i;

	i = 0;
	nDstLen = 0;

	// 取4个字符，解码到一个长整数，再经过移位得到3个字节
	while (i < nSrcLen)
	{
		if (*pSrc != '\r' && *pSrc != '\n')
		{
			nValue = DeBase64Tab[*pSrc++] << 18;
			nValue += DeBase64Tab[*pSrc++] << 12;
			*pDst++ = (nValue & 0x00ff0000) >> 16;
			nDstLen++;

			if (*pSrc != '=')
			{
				nValue += DeBase64Tab[*pSrc++] << 6;
				*pDst++ = (nValue & 0x0000ff00) >> 8;
				nDstLen++;

				if (*pSrc != '=')
				{
					nValue += DeBase64Tab[*pSrc++];
					*pDst++ = nValue & 0x000000ff;
					nDstLen++;
				}
			}

			i += 4;
		}
		else        // 回车换行，跳过
		{
			pSrc++;
			i++;
		}
	}

	// 输出加个结束符
	*pDst = '\0';

	return nDstLen;
}

const char* Base64Ex::GetResult()
{
	return m_pResult;
}

const int Base64Ex::GetLen()
{
	return m_nLen;
}

//const char g_pKey[] = {66, 56, 98, 54, 54, 56, 97, 99, 97, 52, 97, 53, 48, 50, 55, 51, 97};
//EnDecryptionEx::EnDecryptionEx(string strSrc, const unsigned char modeType)
//: m_strResult("")
//, m_byteModeType(modeType)
//{
//	if ("" == strSrc)
//	{
//		return;
//	}
//
//	int nLenSrc = strSrc.length();
//	if (nLenSrc <= 0)
//	{
//		return;
//	}
//
//	//编码
//	if (modeType == modeEncode)
//	{
//	}
//	else if (modeType == modeDecode)
//	{
//		char *pResult = new char[nLenSrc+2];
//		::memset(pResult, 0x00, nLenSrc+2);
//		if (Decryption(g_pKey, (unsigned char *)strSrc.c_str(), nLenSrc, pResult))
//		{
//			m_strResult = pResult;
//		}
//		else
//		{
//			m_strResult = "";
//		}
//
//		delete []pResult;
//		pResult = NULL;
//	}
//
//
//#if 0
//	wchar_t* pTest = L"faf\"aggsdfh在手拿小本本记在了sgsg";
//	string t = CW2A(pTest, CP_UTF8);
//	
//
//	char szBuf[1024] = { 0 };
//	strcpy(szBuf, t.c_str());
//	char* pText = szBuf;
//
//	int strLen2 = t.length();
//	int strLen3 = strlen(szBuf);
//	int strLen4 = strlen(pText);
//
//
//	int strLen = strlen(pText);
//	unsigned char *cDec = new unsigned char[strLen+2];
//	memset(cDec, 0x00, strLen+2);
//
//	bool bRes = Encryption(g_pKey, t.c_str(), cDec);
//
//	char* pDecrypt = new char[strLen+ 2];
//	memset(pDecrypt, 0x00, strLen  + 2);
//	bool bRes2 = Decryption(g_pKey, cDec, strLen  + 2, pDecrypt);
//
//	string tt = pDecrypt;
//	int lenlen = tt.length();
//
//
//	CString xxx = CA2W(pDecrypt, CP_UTF8);
//
//#endif
//}
//
//
//EnDecryptionEx::~EnDecryptionEx()
//{
//}

bool Encryption(const char *pKey, const char *pText, unsigned char *pResult)
{
	if (!pKey || !pText)
		return 0;
	if (strlen(pKey) <= 0 || strlen(pText) <= 0)
		return 0;

	int nTextLen = strlen(pText);
	int nKeyLen = strlen(pKey);
	unsigned char* pPos = pResult;
	srand((int)time(0));
	unsigned char uRand = rand() % 255;

	//第一位为校验位，需要通过第二位校验
	for (int j = 0; j < nKeyLen; j++)
	{
		*pPos += (unsigned char)(*(pKey + j));
	}
	*pPos += uRand;
	pPos++;

	//第二位为随机数
	*pPos = uRand;
	pPos++;

	for (int i = 0; i < nTextLen; i++)
	{
		*pPos = 0x00;
		*pPos++ = (unsigned char)(*(pText + i)) + (unsigned char)(*(pKey + (i%nKeyLen))) + uRand;
	}

	return true;
}

bool Decryption(const char* pKey, const unsigned char* pDate, int ndataLen, char *pResult)
{
	if (!pKey || !pDate || !pResult)
		return false;
	if (strlen(pKey) <= 0 || ndataLen <= 0)
		return false;
	int nKeyLen = strlen(pKey);
	char *pTemp = pResult;
	unsigned char uCheck = *pDate;//取出校验位
	unsigned char uRand = *(pDate + 1);//取出随机数
	unsigned char uCauculate = 0;//计算值
	for (int j = 0; j < nKeyLen; j++)
	{
		uCauculate += (unsigned char)(*(pKey + j)); //242
	}
	uCauculate += uRand;
	if (uCauculate != uCheck)
		return false;

	for (int i = 2; i < ndataLen; i++)
	{
		*pTemp = (unsigned char)(*(pDate + i)) - (unsigned char)(*(pKey + ((i - 2) % nKeyLen))) - uRand;
		pTemp++;
	}
	return true;
}

string eclassDecrypt(const char* pText, const char* pKey)
{
	if (!pText)
		return "";
	if (strlen(pText) <= 0)
		return "";

	string strResult = "";
	Base64Ex baseText(pText, Base64Ex::modeDecode);
	if (baseText.GetLen() <= 0)
		return "";
	char* pBuffer = new char[baseText.GetLen()];
	memset(pBuffer, 0x00, baseText.GetLen());
	Decryption(pKey, (unsigned char*)baseText.GetResult(), baseText.GetLen(), pBuffer);
	strResult = pBuffer;
	if (pBuffer)
	{
		delete[]pBuffer;
		pBuffer = NULL;
	}
	return strResult;
}

string eclassEncrypt(const char* pText, const char* pKey)
{
	if (!pText || !pKey)
		return "";
	if (strlen(pText) <= 0)
		return "";

	string strResult = "";
	int nResultLen = strlen(pText)+2;

	unsigned char* pBuffer = new unsigned char[nResultLen];
	memset(pBuffer, 0x00, nResultLen);
	Encryption(pKey, pText, pBuffer);

	Base64Ex baseText((const char *)pBuffer, Base64Ex::modeEncode, nResultLen);
	strResult = baseText.GetResult();

	if (pBuffer)
	{
		delete[]pBuffer;
		pBuffer = NULL;
	}

	CStringA strReplace = strResult.c_str();
	strReplace.Replace("+", "%2B");
	strReplace.Replace("=", "%3D");
	strResult = strReplace.GetBuffer(0);


	return strResult;
}
