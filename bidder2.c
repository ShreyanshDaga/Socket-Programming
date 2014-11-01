/* 
 * File:   main.c
 * Author: Shreyansh
 *
 * Created on April 21, 2014, 3:38 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include"bidder.h"

Bidder bidd2;

void GetIP(struct sockaddr *pSockAddr, char *pszIP);
void ReadBidder(char *pszBidderFile);

void Phase_1();
void ServerResponse_Login(char *pszServRes);

void Phase_3();
void ReadBiddingInfo(char *pszFile, char *buffTx);
void RecieveItemList(char *buffRx);

void FinalResult();
void PrintResultList(char *buffRx);

int main() 
{
    //Read Bidder        
    ReadBidder("bidderPass2.txt");
    
    //Phase 1
    Phase_1();
    
    //Phase 3
    Phase_3();
    printf("\nEnd of Phase 3 for Bidder");
    //End of Program
}

void GetIP(struct sockaddr *pSockAddr, char *pszIP)
{
    void *pAddr;
    struct sockaddr_in *pIP = (struct sockaddr_in *) pSockAddr;
        
    pAddr = &pIP->sin_addr;
    inet_ntop(pSockAddr->sa_family, pAddr, pszIP, sizeof(pszIP));
}

void ReadBidder(char *pszBidderFile)
{
    FILE *fp;    
    
    fp = fopen(pszBidderFile,"r");
    
    if(fp == NULL)
    {
        printf("\nBidder2 File reading error!");
        exit(1);        
    }
    
    int i = 0;
    
    while( i < 1 )
    {
        fscanf(fp,"%d %s %s %s", &bidd2.iType, bidd2.szBidderName, bidd2.szBidderPass, bidd2.szBidderAcc);                
        i++;
    }       
    
    fclose(fp);
}

void Phase_1()
{
        int iCLSock, bytes_recieved;  
        char recv_data[1024];
        struct hostent *host;
        struct sockaddr_in addrAucServ, addrBidd;          
        socklen_t addrLen = sizeof(addrBidd);
        
        char szLogIn[100];         
        
        host = gethostbyname("127.0.0.1");

        if ((iCLSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Socket");
            exit(1);
        }

        memset(&addrAucServ, 0, sizeof(addrAucServ));
        addrAucServ.sin_family = AF_INET;     
        addrAucServ.sin_port = htons(1933);   
        addrAucServ.sin_addr = *((struct in_addr *)host->h_addr);    
        
        memset(&addrBidd, 0, sizeof(addrBidd));
        addrBidd.sin_family = AF_INET;
        addrBidd.sin_port = htons(0);
        addrBidd.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        if(bind(iCLSock, (struct sockaddr *) &addrBidd, sizeof(addrBidd)) == -1)
        {
            perror("Bind:");
            exit(1);
        }               
                
        if(getsockname(iCLSock, (struct sockaddr *) &addrBidd, &addrLen) != 0)
        {
            perror("GetSockName:");
            exit(1);
        }
        
        printf("\nPhase 1: Bidder1 %s has TCP Port %d  and IP address %s", bidd2.szBidderName, ntohs(addrBidd.sin_port), inet_ntoa(addrBidd.sin_addr));
        fflush(stdout); 

        if (connect(iCLSock, (struct sockaddr *)&addrAucServ, sizeof(struct sockaddr)) == -1) 
        {
            perror("Connect");
            exit(1);
        }
        
        strcpy(szLogIn, "Login# ");
        strcat(szLogIn, "1 ");
        strcat(szLogIn, bidd2.szBidderName);
        strcat(szLogIn, " ");
        strcat(szLogIn, bidd2.szBidderPass);    
        strcat(szLogIn, " ");
        strcat(szLogIn, bidd2.szBidderAcc);
        
        printf("\nPhase 1: Login request. User: %s password: %s Bank account: %s", bidd2.szBidderName, bidd2.szBidderPass, bidd2.szBidderAcc);
        
        send(iCLSock, szLogIn, strlen(szLogIn), 0);
        
        bytes_recieved = recv(iCLSock, recv_data,1024,0);
        recv_data[bytes_recieved] = '\0';
        
        ServerResponse_Login(recv_data);                    
}

void ServerResponse_Login(char *pszServRes)
{
    char s[2] = " ";
    char *pToken;
    
    pToken = strtok(pszServRes, s);
    
    if(strcmp(pToken, "Accepted#") == 0)
    {        
        printf("\nPhase 1: Login request reply: Accepted");        
    }
    else if(strcmp(pToken, "Rejected#") == 0)
    {
        printf("\nPhase 1: Login request reply: Rejected");
    }
    else
    {
        printf("\nBad Command from Auction Server");
    }  
}

void Phase_3()
{
    int iSockId, iPort = 5033, iBuffRx;    
    char buffRx[1024], buffTx[1024], szIP[20];
    struct sockaddr_in addrBidd2, addrAucServ;
    socklen_t ServLen = sizeof(addrAucServ);
    
    memset(&addrBidd2, 0, sizeof(addrBidd2));
    addrBidd2.sin_family = AF_INET;
    addrBidd2.sin_port = htons(iPort);
    addrBidd2.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    iSockId = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(bind(iSockId, (struct sockaddr *) &addrBidd2, sizeof(addrBidd2)) < 0)
    {
        perror("Bind Client:");
        return;
    }
            
    printf("\nPhase 3: Bidder2 %s has UDP port %d and IP address %s", bidd2.szBidderName, iPort, inet_ntoa(addrBidd2.sin_addr));
    fflush(stdout);
    
    iBuffRx = recvfrom(iSockId, buffRx, 1024, 0, (struct sockaddr* ) &addrAucServ, &ServLen);
    buffRx[iBuffRx] = '\0';
    
    RecieveItemList(buffRx);
    
    ReadBiddingInfo("bidding2.txt", buffTx);
    
    sendto(iSockId, buffTx, sizeof(buffTx), 0, (struct sockaddr*) &addrAucServ, ServLen);
    
    close(iSockId);
    
    FinalResult();
}

void ReadBiddingInfo(char *pszFile, char *buffTx)
{
    FILE *fp = fopen(pszFile,"r");
    char szSeller[20], szItem[20], szPrice[20];
    
    strcpy(buffTx, "BiddingInfo# ");
    strcat(buffTx, bidd2.szBidderName);
    
    while(feof(fp) == 0)
    {
        fscanf(fp, "%s %s %s", szSeller, szItem, szPrice);
        if(strcmp(szSeller,"") == 0)
        {
            break;
        }
        
        strcat(buffTx, " ");
        strcat(buffTx, szSeller);
        strcat(buffTx, " ");
        strcat(buffTx, szItem);
        strcat(buffTx, " ");
        strcat(buffTx, szPrice);
        
        szSeller[0] = '\0';
        szItem[0] = '\0';
        szPrice[0] = '\0';
    }
}

void RecieveItemList(char *buffRx)
{
    char s[2] = " ";
    char *pToken;
    
    pToken = strtok(buffRx, s);
    
    printf("\nPhase 3: ");
    
    if(strcmp(pToken, "BroadcastList#") == 0)
    {
        while(pToken != NULL)
        {
                pToken = strtok(NULL, s);
                if(pToken == NULL)
                    break;
                
                printf("\n%s ", pToken);
                pToken = strtok(NULL, s);
                printf("%s ", pToken);
                pToken = strtok(NULL, s);
                printf("%s", pToken);                
        }        
    }
    else
    {
        printf("\nPhase 3: Bad Command from Auction Server.");
    }
}

void FinalResult()
{
    int iSockFD, iOpt = 1, iNewSock;
    struct sockaddr_in addrBidd, addrAucServ;
    int sin_size = sizeof(struct sockaddr_in);
    char buffRx[1024];
    int iBuffRx;
        
    memset(&addrBidd, 0, sizeof(addrBidd));
    addrBidd.sin_family = AF_INET;
    addrBidd.sin_port = htons(4033);
    addrBidd.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    iSockFD = socket(AF_INET, SOCK_STREAM, 0);
    
    if (setsockopt(iSockFD, SOL_SOCKET,SO_REUSEADDR, &iOpt, sizeof(int)) == -1) 
    {
        perror("Setsockopt");
        exit(1);        
    }
    
    if(bind(iSockFD, (struct sockaddr *)&addrBidd, sizeof(addrBidd)) == -1)
    {
        perror("Bind:");
        exit(1);
    }
    
    listen(iSockFD, 5);     
    
    iNewSock = accept(iSockFD, (struct sockaddr *)&addrAucServ, &sin_size);
    
    iBuffRx = recv(iNewSock, buffRx, 1024, 0);
    buffRx[iBuffRx] = '\0';
    
    PrintResultList(buffRx);
    
    close(iNewSock);
    close(iSockFD);        
}

void PrintResultList(char *buffRx)
{
    char s[2] = " ";
    char *pToken;
    char szItem[20], szPrice[20];
    
    pToken = strtok(buffRx, s);
    
    if(strcmp(pToken, "SoldList#") == 0)
    {
        while(pToken != NULL)
        {
            pToken = strtok(NULL, s);
            if(pToken == NULL)
            {
                break;
            }
            
            printf("\nPhase 3: Item %s was sold at price ", pToken);
            pToken = strtok(NULL, s);
            printf("%s", pToken);
        }                                       
    }
    else
    {
        printf("\nBad Command from Auction Server");
    }
}
