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

#include"seller.h"

Seller seller2;
void ReadSeller(char *pszSellerFile);

void Phase_1();
void ServerResponse_Login(char *pszServRes);

void Phase_2();
void ReadItemList(char *pszFile, char *pszItemList);
void SendItemList(char *pszItemList);

void Phase_3();
void FinalResult();
void PrintResultList(char *buffRx);

int iPreAucPort;
char szPreAucIP[20];

int main() 
{
        //Read Seller Info
        ReadSeller("sellerPass2.txt");
        
        //Begin of Phase 1
        Phase_1();
        printf("\nEnd of Phase 1 for %s", seller2.szSellerName);
        //End of Phase 1
        
        //Begin of Phase 2                
        Phase_2();
        printf("\nEnd of Phase 1 for %s", seller2.szSellerName);
        //End of Phase 2
        
        //Begin of Phase 3
        Phase_3();
        printf("\nEnd of Phase 1 for %s", seller2.szSellerName);
        //End of Phase 3
        return 0;
}

void ReadSeller(char *pszSellerFile)
{
    FILE *fp;    
    
    fp = fopen(pszSellerFile,"r");
    
    if(fp == NULL)
    {
        printf("\nSeller 2 File reading error!");
        exit(1);        
    }
    
    int i = 0;
    
    while( i < 1 )
    {
        fscanf(fp,"%d %s %s %s", &seller2.iType, seller2.szSellerName, seller2.szSellerPass, seller2.szSellerAcc);                
        i++;
    }       
    
    fclose(fp);        
}

void Phase_1()
{
        int iCLSock, bytes_recieved;  
        char recv_data[1024];
        struct hostent *host;
        struct sockaddr_in addrAucServ, addrSell;          
        socklen_t addrLen = sizeof(addrSell);
        char szLogIn[100];
    
        host = gethostbyname("localhost");
        
        //Phase 1
        if ((iCLSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        {
            perror("Socket");
            exit(1);
        }

        memset(&addrAucServ, 0, sizeof(addrAucServ));
        addrAucServ.sin_family = AF_INET;     
        addrAucServ.sin_port = htons(1933);   
        addrAucServ.sin_addr = *((struct in_addr *)host->h_addr);

        memset(&addrSell, 0, sizeof(addrSell));
        addrSell.sin_family = AF_INET;
        addrSell.sin_port = htons(0);
        addrSell.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        if(bind(iCLSock, (struct sockaddr *) &addrSell, sizeof(addrSell)) == -1)
        {
            perror("Bind:");
            exit(1);
        }               
                
        if(getsockname(iCLSock, (struct sockaddr *) &addrSell, &addrLen) != 0)
        {
            perror("GetSockName:");
            exit(1);
        }
        
        printf("\nPhase 1: Seller2 has TCP Port %d  and IP address %s", ntohs(addrSell.sin_port), inet_ntoa(addrSell.sin_addr));
        fflush(stdout); 
                
        if (connect(iCLSock, (struct sockaddr *)&addrAucServ, sizeof(struct sockaddr)) == -1) 
        {
            perror("Connect");
            exit(1);
        }
        
        strcpy(szLogIn, "Login# ");
        strcat(szLogIn, "2 ");
        strcat(szLogIn, seller2.szSellerName);
        strcat(szLogIn, " ");
        strcat(szLogIn, seller2.szSellerPass);    
        strcat(szLogIn, " ");
        strcat(szLogIn, seller2.szSellerAcc);
        
        printf("\nPhase 1: Login request. User: %s password: %s Bank account: %s", seller2.szSellerName, seller2.szSellerPass, seller2.szSellerAcc);
        
        send(iCLSock, szLogIn, strlen(szLogIn), 0);               
        
        bytes_recieved = recv(iCLSock, recv_data,1024,0);
        recv_data[bytes_recieved] = '\0';        
        ServerResponse_Login(recv_data);        
        fflush(stdout);
        
        close(iCLSock);
}

void ServerResponse_Login(char *pszServRes)
{
    char s[2] = " ";
    char *pToken;
    
    pToken = strtok(pszServRes, s);
    
    if(strcmp(pToken, "Accepted#") == 0)
    {
        pToken = strtok(NULL, s);
        strcpy(szPreAucIP, pToken);        
        pToken = strtok(NULL, s);        
        iPreAucPort = atoi(pToken);
        printf("\nPhase 1: Login request reply: Accepted");
        printf("\nPhase 1: Auction Server has IP Address %s and PreAuction TCP Port  Number %d", szPreAucIP, iPreAucPort);
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

void Phase_2()
{        
        int iCLSock, bytes_recieved;  
        char send_data[1024],recv_data[1024];
        struct hostent *host;
        struct sockaddr_in addrAucServ, addrSell;          
        socklen_t addrLen = sizeof(addrSell);
        int iPreAucSock, iBuffTx;
        char buffTx[1024], buffRx[1024];
    
        host = gethostbyname("127.0.0.1");
                
        sleep(5);
        //Phase 1
        if ((iPreAucSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        {
            perror("Socket");
            exit(1);
        }

        memset(&addrAucServ, 0, sizeof(addrAucServ));
        addrAucServ.sin_family = AF_INET;
        addrAucServ.sin_port = htons(iPreAucPort);
        addrAucServ.sin_addr = *((struct in_addr *)host->h_addr);        
                
        if (connect(iPreAucSock, (struct sockaddr *)&addrAucServ, sizeof(struct sockaddr)) == -1) 
        {
            perror("Connect");
            exit(1);
        }        
        
        printf("\nPhase 2: Auction Server IP Address: %s PreAuction TCP Port Number %d", szPreAucIP, iPreAucPort);
        
        ReadItemList("itemList2.txt", buffTx);
        
        iBuffTx = send(iPreAucSock, buffTx, strlen(buffTx), 0);
        
        if(iBuffTx != strlen(buffTx))
        {
            perror("Incomplete Sending: ");
        }                
        
        recv(iPreAucSock, buffRx, 1024, 0);
        
        close(iPreAucSock);
        printf("\nEnd of Phase 2 for Seller2");
}

void ReadItemList(char *pszFile, char *pszItemList)
{
    FILE *fp;
    char szUser[20];
    char szItem[20];
    char szPrice[10];
    
    strcpy(pszItemList,"ItemList# ");
    fp = fopen(pszFile,"r");    
    
    fscanf(fp, "%s", szUser);
    strcat(pszItemList, szUser);
       
    printf("\nPhase 2: Seller2 send item lists.");
    printf("\nPhase 2: %s", szUser);
    
    while(feof(fp) == 0)
    {
        strcat(pszItemList ," ");
        fscanf(fp, "%s", szItem);
        if(strcmp(szItem, "\0") == 0)
        {
            break;
        }
        strcat(pszItemList, szItem);
        strcat(pszItemList ," ");
        fscanf(fp, "%s", szPrice);                
        strcat(pszItemList, szPrice);
        
        printf("\n%s %s", szItem, szPrice);
        
        strcpy(szItem,"");
        strcpy(szPrice,"");
    }    
    
    fclose(fp);
}

void Phase_3()
{
    FinalResult();
}

void FinalResult()
{
    int iSockFD, iOpt = 1, iNewSock;
    struct sockaddr_in addrSell, addrAucServ;
    int sin_size = sizeof(struct sockaddr_in);
    char buffRx[1024];
    int iBuffRx;
        
    memset(&addrSell, 0, sizeof(addrSell));
    addrSell.sin_family = AF_INET;
    addrSell.sin_port = htons(3033);
    addrSell.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    iSockFD = socket(AF_INET, SOCK_STREAM, 0);
    
    if (setsockopt(iSockFD, SOL_SOCKET,SO_REUSEADDR, &iOpt, sizeof(int)) == -1) 
    {
        perror("Setsockopt");
        exit(1);        
    }
    
    if(bind(iSockFD, (struct sockaddr *)&addrSell, sizeof(addrSell)) == -1)
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
