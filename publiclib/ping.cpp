#include "stdafx.h"

#include "ping.h"   
#include "util.h"
#include "LogEx.h"


//第二种ping 方法 ，win7非管理员，失败
/*
#define DEF_BUF_SIZE 1024 
#define IP_HEADER_SIZE 20 
#define ICMP_HEADER_SIZE 12 


typedef struct _ICMP_HEADER {    
	BYTE bType;        //类型    
	BYTE bCode;        //代码    
	USHORT nCheckSum;  //校验各  
	USHORT nId;        //进程ID  
	USHORT nSequence;  //序号    
	UINT nTimeStamp;   //时间 
}ICMP_HEADER, *PICMP_HEADER; 

USHORT GetCheckSum(LPBYTE lpBuff, DWORD dwSize) 
{    
	DWORD dwCheckSum = 0;    
	USHORT* lpWord = (USHORT*)lpBuff;  
	while(dwSize > 1)    
	{       
		dwCheckSum += *lpWord++;  
		dwSize -= 2;
	}   
	if(dwSize ==1)      
		dwCheckSum += *((LPBYTE)lpBuff);  
	dwCheckSum = (dwCheckSum >> 16) + (dwCheckSum & 0XFFFF);   
	return (USHORT)(~dwCheckSum); 
} */

BOOL CPing::Ping(UINT nRetries,const char* pstrHost,int& nResponse)
{  

	//提权，失败
	/*HMODULE hDll = ::LoadLibrary(_T("ntdll.dll")); 
	typedef int (__stdcall * type_RtlAdjustPrivilege)(int, bool, bool, int*);
	type_RtlAdjustPrivilege RtlAdjustPrivilege = (type_RtlAdjustPrivilege)GetProcAddress(hDll, "RtlAdjustPrivilege");
	int nEn = 0;
	RtlAdjustPrivilege(0x14,TRUE,FALSE,&nEn);
	FreeLibrary(hDll);*/

	
	//第二种ping 方法 ，win7非管理员，失败
	/*SOCKADDR_IN DestSockAddr;     
	DestSockAddr.sin_family = AF_INET;    
	DestSockAddr.sin_addr.S_un.S_addr = inet_addr(pstrHost);    
	DestSockAddr.sin_port = htons(0);
	char ICMPPack[ICMP_HEADER_SIZE] = {0};  
	PICMP_HEADER pICMPHeader = (PICMP_HEADER)ICMPPack;   
	pICMPHeader->bType = 8;     
	pICMPHeader->bCode = 0;    
	pICMPHeader->nId = (USHORT)::GetCurrentProcessId();  
	pICMPHeader->nCheckSum = 0;
	pICMPHeader->nTimeStamp = 0;    
	WORD version = MAKEWORD(2, 2);  
	WSADATA wsaData;  
	if(WSAStartup(version, &wsaData) != 0)   
	{      
		Write_Log(LOG_INFO,"WSAStartup error\n");       
		return FALSE;  
	}   
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);    
	if(s== SOCKET_ERROR)   
	{      
		Write_Log(LOG_INFO,"init error\n");       
		return FALSE;  
	}   
	int nTime = 1000;  
	int ret = ::setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTime, sizeof(nTime));  
	char szRcvBuff[DEF_BUF_SIZE];  
	SOCKADDR_IN SourceSockAddr;  
	for(int i=0; i<4; i++)   
	{   
		pICMPHeader->nCheckSum = 0;      
		pICMPHeader->nSequence = i;  
		pICMPHeader->nTimeStamp = ::GetTickCount();   
		pICMPHeader->nCheckSum = GetCheckSum((LPBYTE)(ICMPPack), ICMP_HEADER_SIZE); 
		int nRet = ::sendto(s, ICMPPack, ICMP_HEADER_SIZE, 0, (SOCKADDR*)&DestSockAddr, sizeof(DestSockAddr));      
		if(nRet == SOCKET_ERROR)  
		{     
			Write_Log(LOG_INFO,"send error.\n");     
			return FALSE;    
		}      
		int nLen = sizeof(SOCKADDR_IN);        
		 
		nRet = ::recvfrom(s, szRcvBuff,DEF_BUF_SIZE,0,(SOCKADDR*)&SourceSockAddr,&nLen);  
		if(nRet == SOCKET_ERROR)      
		{     
			int nError = ::WSAGetLastError();       
			Write_Log(LOG_INFO,"Recv Error:%d.\n", nError);   
			return FALSE;      
		}   
		            
		PICMP_HEADER pRcvHeader = (PICMP_HEADER)(szRcvBuff + IP_HEADER_SIZE);     
		int nTime = ::GetTickCount() - pRcvHeader->nTimeStamp;    
		Write_Log(LOG_INFO,"从目标地址传回: %s bytes=%d time=%dms\n", inet_ntoa(SourceSockAddr.sin_addr), nRet, nTime);       
		::Sleep(1000);
	}    
	closesocket(s);   
	WSACleanup();  
	return 0;*/


    SOCKET    rawSocket;  
    LPHOSTENT lpHost;  
    UINT      nLoop;  
    int       nRet;  
    struct    sockaddr_in saDest;  
    //struct    sockaddr_in saSrc;  
    //DWORD     dwTimeSent;  
    //DWORD     dwElapsed;  
    //u_char    cTTL;  
  
    CStringA str;
  
	Write_Log(LOG_INFO,"try ping %s.",pstrHost);

    // Create a Raw socket   
    rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);  
	//rawSocket = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
	//rawSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP|IPPROTO_UDP|IPPROTO_ICMP);
	//rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_IP);  
    if (rawSocket == SOCKET_ERROR)   
    {  
		Write_Log(LOG_ERROR,"socket init fail.");
		nResponse = 66666666;
        return FALSE;  
    }  
      
    // Lookup host   
    lpHost = gethostbyname(pstrHost);  
    if (lpHost == NULL)  
    {  
        //str.Format("Host not found: %s", pstrHost); 
		Write_Log(LOG_ERROR,"Host not found: %s", pstrHost);
        return FALSE;  
    }  
      
    // Setup destination socket address   
    saDest.sin_addr.s_addr = *((u_long FAR *) (lpHost->h_addr));  
    saDest.sin_family = AF_INET;  
    saDest.sin_port = 0;  
    // Tell the user what we're doing   
    //str.Format("Pinging %s [%s] with %d bytes of data:",pstrHost,inet_ntoa(saDest.sin_addr),REQ_DATASIZE);
	Write_Log(LOG_INFO,"Pinging %s [%s] with %d bytes of data:",pstrHost,inet_ntoa(saDest.sin_addr),REQ_DATASIZE);


	int nStartTime;
	int nStopTime;
    // Ping multiple times   
    for (nLoop = 0; nLoop < nRetries; nLoop++)  
    {  
        // Send ICMP echo request   
		nStartTime = Ex_GetTickCount();
        SendEchoRequest(rawSocket, &saDest);  
        //Sleep(1000);
        nRet = WaitForEchoReply(rawSocket);
		nStopTime = Ex_GetTickCount();
		nResponse = nStopTime - nStartTime;
        if (nRet == SOCKET_ERROR)  
        {  
            //WSAError("select()");
			Write_Log(LOG_ERROR,"SOCKET_ERROR");
            return FALSE;  
        }  
        if (!nRet)  
        {  
            //str.Format("Request Timed Out");
			nResponse = 9000;
			Write_Log(LOG_ERROR,"Request Timed Out");
            return FALSE;  
        }  
        else  
        {  
            return TRUE;  
        }  
    }  
    nRet = closesocket(rawSocket);
	return false;
}  
  
  
int CPing::SendEchoRequest(SOCKET s,LPSOCKADDR_IN lpstToAddr)   
{  
    static ECHOREQUEST echoReq;  
    static int nId = 1;  
    static int nSeq = 1;  
    int nRet;  
  
    // Fill in echo request   
    echoReq.icmpHdr.Type        = ICMP_ECHOREQ;  
    echoReq.icmpHdr.Code        = 0;  
    echoReq.icmpHdr.Checksum    = 0;  
    echoReq.icmpHdr.ID          = nId++;  
    echoReq.icmpHdr.Seq         = nSeq++;  
  
    // Fill in some data to send   
    for (nRet = 0; nRet < REQ_DATASIZE; nRet++)  
        echoReq.cData[nRet] = ' '+nRet;  
  
    // Save tick count when sent   
    echoReq.dwTime              = GetTickCount();  
  
    // Put data in packet and compute checksum   
    echoReq.icmpHdr.Checksum = in_cksum((u_short *)&echoReq, sizeof(ECHOREQUEST));  
  
    // Send the echo request                                     
    nRet = sendto(s,                        /* socket */  
                 (LPSTR)&echoReq,           /* buffer */  
                 sizeof(ECHOREQUEST),  
                 0,                         /* flags */  
                 (LPSOCKADDR)lpstToAddr, /* destination */  
                 sizeof(SOCKADDR_IN));   /* address length */  
  
    if (nRet == SOCKET_ERROR)   
        WSAError("sendto()");  
    return (nRet);  
}  
  
  
DWORD CPing::RecvEchoReply(SOCKET s, LPSOCKADDR_IN lpsaFrom, u_char *pTTL)   
{  
    ECHOREPLY echoReply;  
    int nRet;  
    int nAddrLen = sizeof(struct sockaddr_in);  
  
    // Receive the echo reply      
    nRet = recvfrom(s,                  // socket   
                    (LPSTR)&echoReply,  // buffer   
                    sizeof(ECHOREPLY),  // size of buffer   
                    0,                  // flags   
                    (LPSOCKADDR)lpsaFrom,   // From address   
                    &nAddrLen);         // pointer to address len   
  
    // Check return value   
    if (nRet == SOCKET_ERROR)   
        WSAError("recvfrom()");  
  
    // return time sent and IP TTL   
    *pTTL = echoReply.ipHdr.TTL;  
  
    return(echoReply.echoRequest.dwTime);             
}  
  
  
int CPing::WaitForEchoReply(SOCKET s)  
{  
    struct timeval Timeout;  
    fd_set readfds;  
  
    readfds.fd_count = 1;  
    readfds.fd_array[0] = s;  
    Timeout.tv_sec = 1;  
    Timeout.tv_usec = 0;  
  
    return(select(1, &readfds, NULL, NULL, &Timeout));  
}  
  
void CPing::WSAError(LPCSTR lpMsg)  
{  
    CStringA strMsg;  
    strMsg.Format("%s - WSAError: %ld",lpMsg,WSAGetLastError());      
}  
  
  
  
  
//   
// Mike Muuss' in_cksum() function   
// and his comments from the original   
// ping program   
//   
// * Author -   
// *    Mike Muuss   
// *    U. S. Army Ballistic Research Laboratory   
// *    December, 1983   
  
/* 
 *          I N _ C K S U M 
 * 
 * Checksum routine for Internet Protocol family headers (C Version) 
 * 
 */  
u_short CPing::in_cksum(u_short *addr, int len)  
{  
    register int nleft = len;  
    register u_short *w = addr;  
    register u_short answer;  
    register int sum = 0;  
  
    /* 
     *  Our algorithm is simple, using a 32 bit accumulator (sum), 
     *  we add sequential 16 bit words to it, and at the end, fold 
     *  back all the carry bits from the top 16 bits into the lower 
     *  16 bits. 
     */  
    while( nleft > 1 )  {  
        sum += *w++;  
        nleft -= 2;  
    }  
  
    /* mop up an odd byte, if necessary */  
    if( nleft == 1 ) {  
        u_short u = 0;  
  
        *(u_char *)(&u) = *(u_char *)w ;  
        sum += u;  
    }  
  
    /* 
     * add back carry outs from top 16 bits to low 16 bits 
     */  
    sum = (sum >> 16) + (sum & 0xffff);   /* add hi 16 to low 16 */  
    sum += (sum >> 16);           /* add carry */  
    answer = ~sum;              /* truncate to 16 bits */  
    return (answer);  
}  
