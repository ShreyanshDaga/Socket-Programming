// AuctionServer.c

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

#include"auctionserver.h"

MyUser lstUsers[6];
ItemList lstItem[2];
BiddingList bidd1List, bidd2List;
BroadcastList blist;
SellList sellList[10];
int iBcount = 0;

void Phase_1();
int ReadRegistrationFile(char *pszFileName);
int AuthenticateUser(int iSock, char *pszIP);

void Phase_2();
void ReadItemList(int iSock);

void Phase_3();
void ReadBroadcastList(char *pszFile, char *buffTx);
void RecieveBiddingList(char *buffRx, BiddingList *pBiddList);
void Auction(BroadcastList blist, BiddingList bidd1, BiddingList bidd2);
int FindMatchInBidder(BiddingList bidd, char *pszSeller, char *pszItem, char *pszPrice, int *pBiddPrice);
void PostResult_Bidd1();
void PostResult_Bidd2();
void PostResult_Sell1();
void PostResult_Sell2();
void PrintFinalResult();

char szAucServIP[20];

int main()
{                     
        //Begin of Phase 1
        Phase_1();
        printf("\nEnd of Phase 1 for Auction Server");
        //End of Phase 1
        
        //Begin of Phase 2
        Phase_2();
        printf("\nEnd of Phase 2 for Auction Server");
        //End of Phase 2
        
        //Begin of Phase 3
        Phase_3();
        printf("\nEnd of Phase 3 for Auction Server");
        //End of Phase 3
}

void Phase_1()
{
        int iWelSock, iNewSock, bytes_recieved , true = 1;  
        char send_data [1024] , recv_data[1024];

        struct sockaddr_in addAucServer, addClient;    
        int sin_size;
        
        int iFlagAddInfo;
        struct addrinfo Hints, *Serverinfo;
        int iSockOpt = 1;
        
        int iAucPort = 1933;                        
        
        //Read Registration File and populate the Users with their data
        ReadRegistrationFile("Registration.txt");
        
        memset(&Hints, 0, sizeof(Hints));
        Hints.ai_family = AF_INET;
        Hints.ai_socktype = SOCK_STREAM;        
        
        if((iFlagAddInfo = getaddrinfo("localhost", "1933", &Hints, &Serverinfo)) != 0)
        {
            perror("getaddrinfo ");            
        }
        
        void *pAddr;
        struct sockaddr_in *pIP = (struct sockaddr_in *) Serverinfo->ai_addr;
        
        pAddr = &pIP->sin_addr;
        inet_ntop(Serverinfo->ai_family, pAddr, szAucServIP, sizeof(szAucServIP));                        
        
        //Create Welcoming Socket
        if ((iWelSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        {
            perror("Socket");
            exit(1);
        }             
        
        if (setsockopt(iWelSock, SOL_SOCKET,SO_REUSEADDR, &iSockOpt, sizeof(int)) == -1) 
        {
            perror("Setsockopt");
            exit(1);        
        }
        struct hostent *host;
        host = gethostbyname("localhost");
        
        // Configuring the Welcoming Socket
        memset(&addAucServer, 0 ,sizeof(addAucServer));
        addAucServer.sin_family = AF_INET;         
        addAucServer.sin_port = htons(iAucPort);     
        addAucServer.sin_addr = *((struct in_addr *)host->h_addr);                    

        if (bind(iWelSock, (struct sockaddr *)&addAucServer, sizeof(struct sockaddr)) == -1) 
        {
            perror("Unable to bind");
            exit(1);
        }

        if (listen(iWelSock, 10) == -1) 
        {
            perror("Listen");
            exit(1);
        }
		
        printf("\nPhase 1: Auction Server has TCP port number %d and IP address %s" , iAucPort, szAucServIP);
        fflush(stdout);
        int i = 0;
        
        for(i = 0;i<4;i++)
        {  
                sin_size = sizeof(struct sockaddr_in);

                iNewSock = accept(iWelSock, (struct sockaddr *)&addClient, &sin_size);
                
                //if(!fork())
                //{
                        //close(iWelSock);
                        AuthenticateUser(iNewSock, inet_ntoa(addClient.sin_addr));                        
                        close(iNewSock);
                //}
               // close(iNewSock);
                fflush(stdout);
        }
                
        close(iWelSock);
}

int ReadRegistrationFile(char *pszFileName)
{
    FILE *fp;
    char szName[20], szPassword[20], szAccount[20];    
    
    fp = fopen(pszFileName,"r");
    
    if(fp == NULL)
    {
        printf("\nRegistration File reading error!");
        exit(1);        
    }
    
    int i = 0;
    
    while( i < 6 )
    {
        fscanf(fp,"%s %s %s", szName, szPassword, szAccount);
        
        lstUsers[i].iType = 0;
        strcpy(lstUsers[i].szUserName, szName);
        strcpy(lstUsers[i].szUserPass, szPassword);
        strcpy(lstUsers[i].szUserAcc, szAccount);       
        
        i++;
    }
    
    //printf("\nRegistration file reading complete.");
    
    fclose(fp);
    
    return 0;
}

int AuthenticateUser(int iSock, char *pszIP)
{
        char buffRx[1024], buffTx[1024];
        int iBuffRx, iBuffTx;
        char szLogin[10];
        char szUserName[20];
        char szUserPass[20];
        char szUserAcc[20];
        int iType, i = 0;        
    
        iBuffRx = recv(iSock, buffRx, 1024, 0);
        buffRx[iBuffRx] = '\0';
        
        sscanf(buffRx,"%s %d %s %s %s", szLogin, &iType, szUserName, szUserPass, szUserAcc);
    
        if(strcmp(szLogin,"Login#") == 0)
        {
                printf("\nPhase 1: Authentication request. ");
                printf("User#: %s. Password: %s. Bank Account: %s. User IP Addr: %s.", szUserName, szUserPass, szUserAcc, pszIP);
                
                for(i = 0;i<6;i++)
                {
                        if(strcmp(szUserName, lstUsers[i].szUserName) == 0)
                        {
                                if(strcmp(szUserPass, lstUsers[i].szUserPass) == 0 && strcmp(szUserAcc, lstUsers[i].szUserAcc) == 0)
                                {
                                        lstUsers[i].iType = iType;
                                        strcpy(lstUsers[i].szUserIP, pszIP);
                    
                                        strcpy(buffTx,"Accepted#");                                                                            
                                        
                                        printf(" Authorized: Yes");
                                        
                                        if(iType == 2)
                                        {
                                            char szPreAucPort[10] = "2033";                                                                                    
                                            
                                            strcat(buffTx, " ");
                                            strcat(buffTx, szAucServIP);
                                            strcat(buffTx, " ");
                                            strcat(buffTx, szPreAucPort);                                                                                                                                                                             
                                        }
                                        
                                        iBuffTx = send(iSock, buffTx, strlen(buffTx), 0);
                                        fflush(stdout);
                                }
                                else
                                {
                                        strcpy(buffTx,"Rejected#");
                                        iBuffTx = send(iSock, buffTx, strlen(buffTx), 0);
                                        printf(" Authorized: No");                                        
                                }
                        }
                }                
        }
        else
        {
                strcpy(buffTx,"Bad#");
                iBuffTx = send(iSock, buffTx, strlen(buffTx), 0);
        }
        
        return 0;
}

void Phase_2()
{
        int iWelSock, iNewSock, bytes_recieved , true = 1;  
        char send_data [1024] , recv_data[1024], szServIP[16];

        struct sockaddr_in addAucServer, addClient;    
        int sin_size;
        
        int iFlagAddInfo;
        struct addrinfo Hints, *Serverinfo;
        int iSockOpt = 1;
        
        int iAucPort = 2033;
        
        memset(&Hints, 0, sizeof(Hints));
        Hints.ai_family = AF_INET;
        Hints.ai_socktype = SOCK_STREAM;
        
        if((iFlagAddInfo = getaddrinfo("127.0.0.1", "2033", &Hints, &Serverinfo)) != 0)
        {
            perror("\ngetaddrinfo: ");
            exit(1);
        }
        
        void *pAddr;
        struct sockaddr_in *pIP = (struct sockaddr_in *) Serverinfo->ai_addr;
        
        pAddr = &pIP->sin_addr;
        inet_ntop(Serverinfo->ai_family, pAddr, szServIP, sizeof(szServIP));
        
        
        //Create Welcoming Socket
         if ((iWelSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        {
            perror("\nSocket");
            exit(1);
        }             
        
        if (setsockopt(iWelSock, SOL_SOCKET,SO_REUSEADDR, &iSockOpt, sizeof(int)) == -1) 
        {
            perror("\nSet socket Options");
            exit(1);        
        }
        
        // Configuring the Welcoming Socket
        memset(&addAucServer, 0, sizeof(addAucServer));
        addAucServer.sin_family = AF_INET;         
        addAucServer.sin_port = htons(iAucPort);     
        addAucServer.sin_addr.s_addr = inet_addr("127.0.0.1");                

        if (bind(iWelSock, (struct sockaddr *)&addAucServer, sizeof(struct sockaddr)) == -1) 
        {
            perror("\nUnable to bind");
            exit(1);
        }

        if (listen(iWelSock, 10) == -1) 
        {
            perror("\nListen");
            exit(1);
        }
        
        printf("\nPhase 2: Auction Server IP address %s, PreAuction TCP port number %d." , szServIP, iAucPort);
        fflush(stdout);
        
        int i = 0;
        for(i = 0;i<2;i++)
        {
                sin_size = sizeof(struct sockaddr_in);
                                
                iNewSock = accept(iWelSock, (struct sockaddr *)&addClient, &sin_size);                
                                
                ReadItemList(iNewSock);
                close(iNewSock);
                fflush(stdout);
        }
        
        close(iWelSock);
}

void ReadItemList(int iSock)
{
    char buffRx[1024];
    int iBuffRx;
    char szUser[20], szItem[20], szPrice[20];
    char ch = 'a';
    int i = 0;
    char *pToken;
    char s[2] = " ";
    int iCount = 0;
    int user = 0;
    
    iBuffRx = recv(iSock, buffRx, 1024, 0);        
    
    buffRx[iBuffRx] = '\0';
    
    pToken = strtok(buffRx, s);
    
    if(strcmp(pToken, "ItemList#") == 0)
    {
        pToken = strtok(NULL, s);
        strcpy(szUser, pToken);                
        
        if(strcmp(szUser, "Taylor") != 0)                    
            user = 1;
        
        strcpy(lstItem[user].szUser, szUser);
        
        while(pToken != NULL)
        {
                pToken = strtok(NULL, s);
                if(pToken == NULL)
                {
                    break;
                }
                strcpy(szItem, pToken);
                pToken = strtok(NULL, s);
                strcpy(szPrice, pToken);
                
                strcpy(lstItem[user].szItem[iCount], szItem);
                strcpy(lstItem[user].szPrice[iCount], szPrice);
                iCount++;
        }
        
        printf("\nPhase 2: Seller %s send item list.", szUser);
        printf("\nPhase 2:");
        int k = 0;
        for(k = 0;k < iCount;k++)
        {
                printf("\n%s %s", lstItem[user].szItem[k], lstItem[user].szPrice[k]);
        }
    }
    else
    {
        printf("\nPhase 2: Bad Command from User");
    }
    
    send(iSock, "ok", strlen("ok"), 0);
}

void Phase_3()
{
    int iUDPBidd1 = 4933, iUDPBidd2 = 5033;
    int iTCPBidd1 = 3933, iTCPBidd2 = 4033;
    int iTCPSell1 =2933 , iTCPSell2 = 3033;
    
    int iUDPSock1, iUDPSock2;
    int iTCPSockB1, iTCPSockB2, iTCPSockS1, iTCPSockS2;
    
    struct sockaddr_in addrServ, addrBidd1, addrBidd2, addrSell1, addrSell2;
    socklen_t addlen = sizeof(addrBidd2);
    
    char buffTx[1024], buffRx1[1024], buffRx2[1024];        
    
    iUDPSock1 = socket(AF_INET, SOCK_DGRAM, 0);
    iUDPSock2 = socket(AF_INET, SOCK_DGRAM, 0);
    
    memset(&addrServ, 0, sizeof(addrServ));
    addrServ.sin_family = AF_INET;
    addrServ.sin_port = htons(0);
    addrServ.sin_addr.s_addr = inet_addr("127.0.0.1");
    int i;
    
    if(bind(iUDPSock1, (struct sockaddr *) &addrServ, sizeof(addrServ)) != 0)
    {
        perror("\nPhase 3: Bind Error ");
    }
    
    if(bind(iUDPSock2, (struct sockaddr *) &addrServ, sizeof(addrServ)) != 0)
    {
        perror("\nPhase 3: Bind Error ");
    }
    
    char szAucIP[20];
    int iCh;
    int iBuffRx, iBuffTx;
    
    void *pAddr;
    struct sockaddr_in *pIP = (struct sockaddr_in *) &addrServ;
        
    pAddr = &pIP->sin_addr;
    inet_ntop(addrServ.sin_family, pAddr, szAucIP, sizeof(szAucIP));
    
    iCh = getsockname(iUDPSock1, (struct sockaddr *) &addrServ, &addlen);
    printf("\nPhase 3: Auction Server has IP address: %s  Auction UDP Port Number: %d", szAucIP, ntohs(addrServ.sin_port));
    iCh = getsockname(iUDPSock2, (struct sockaddr *) &addrServ, &addlen);
    printf("\nPhase 3: Auction Server has IP address: %s  Auction UDP Port Number: %d", szAucIP, ntohs(addrServ.sin_port));
    
    memset(&addrBidd1, 0, sizeof(addrBidd1));
    addrBidd1.sin_family = AF_INET;
    addrBidd1.sin_port = htons(iUDPBidd1);    
    addrBidd1.sin_addr.s_addr = INADDR_ANY;
    
    memset(&addrBidd2, 0, sizeof(addrBidd2));
    addrBidd2.sin_family = AF_INET;
    addrBidd2.sin_port = htons(iUDPBidd2);
    addrBidd2.sin_addr.s_addr = inet_addr("127.0.0.1");//INADDR_ANY;        
    
    memset(&blist, 0, sizeof(blist));
    
    ReadBroadcastList("broadcastList.txt", buffTx);
    
    //Send Broadcast List
    iBuffTx = sendto(iUDPSock1, buffTx, sizeof(buffTx), 0, (struct sockaddr *) &addrBidd1, addlen);
    iBuffTx = sendto(iUDPSock2, buffTx, sizeof(buffTx), 0, (struct sockaddr *) &addrBidd2, addlen);    
    
    memset(&bidd1List, 0 ,sizeof(bidd1List));
    memset(&bidd2List, 0 ,sizeof(bidd2List));
    
    //if(fork() == 0)
    {
        iBuffRx = recvfrom(iUDPSock1, buffRx1, 1024, 0, (struct sockaddr *) &addrBidd1, &addlen);        
        buffRx1[iBuffRx] = '\0';
        RecieveBiddingList(buffRx1, &bidd1List);    
    }    
        
    //if(fork() == 0)
    {
        iBuffRx = recvfrom(iUDPSock2, buffRx2, 1024, 0, (struct sockaddr *) &addrBidd2, &addlen);        
        buffRx2[iBuffRx] = '\0';
        RecieveBiddingList(buffRx2, &bidd2List);        
    }    
                
    close(iUDPSock1);
    close(iUDPSock2);
    
    //Perform Auctioning
    Auction(blist, bidd1List, bidd2List);        
    
    sleep(5);
    
    PrintFinalResult();
    
    //For Bidder 1
    PostResult_Bidd1();
    //For Bidder 2
    PostResult_Bidd2();
    //For Seller 1
    PostResult_Sell1();
    //For Seller 2
    PostResult_Sell2();
}

void ReadBroadcastList(char *pszFile, char *buffTx)
{
    FILE *fp;
    char szSeller[20], szItem[20], szPrice[20];
    
    fp = fopen(pszFile, "r");
    
    strcpy(buffTx,"BroadcastList#");

    printf("\nPhase 3: ");
    
    int i = 0;
    while(feof(fp) == 0)
    {
        fscanf(fp,"%s %s %s", szSeller, szItem, szPrice);
        
        if(strcmp(szSeller,"\0") == 0)
        {
            break;
        }
        
        if(strcmp(szItem,"\0") == 0)
        {
            break;
        }
        
        if(strcmp(szPrice,"\0") == 0)
        {
            break;
        }
        
        strcat(buffTx, " ");
        strcat(buffTx,szSeller);
        strcat(buffTx, " ");
        strcat(buffTx, szItem);
        strcat(buffTx, " ");
        strcat(buffTx, szPrice);
        
        printf("\n%s %s %s", szSeller, szItem, szPrice);
        
        strcpy(blist.szSeller[i], szSeller);
        strcpy(blist.szItem[i], szItem);
        strcpy(blist.szPrice[i], szPrice);
        
        szSeller[0] = '\0';
        szItem[0] = '\0';
        szPrice[0] = '\0';
        i++;
    }
    
    iBcount = i;
}

void RecieveBiddingList(char *buffRx, BiddingList *pBiddList)
{    
    char s[2] = " ";
    
    char *pToken;
    
    pToken = strtok(buffRx, s);
    
    if(strcmp(pToken,"BiddingInfo#") == 0)
    {
        pToken = strtok(NULL, s);
        
        printf("\nPhase 3: Auction Server received a bidding from %s", pToken);
        strcpy(pBiddList->szBidder, pToken);
        printf("\nPhase 3: ");
        int i = 0;
        while(pToken != NULL)
        {
            pToken = strtok(NULL, s);
            if(pToken == NULL)
            {
                break;
            }
            
            strcpy(pBiddList->szSellers[i], pToken);
            pToken = strtok(NULL, s);
            strcpy(pBiddList->szItems[i], pToken);
            pToken = strtok(NULL, s);
            strcpy(pBiddList->szPrice[i], pToken);            
            
            printf("\n%s %s %s ", pBiddList->szSellers[i], pBiddList->szItems[i], pBiddList->szPrice[i]);
            i++;
        }                
    }
    else
    {
        printf("\nBad Command From Bidder");
    }
}

void Auction(BroadcastList blist, BiddingList bidd1, BiddingList bidd2)
{
    int i = 0;
    char szSeller[20];
    char szBidder[20];
    char szItem[20];
    char szPrice[10];
    int iItemsSold = 0;
    
    
    int iPrice, iBiddPrice1, iBiddPrice2;
    
    for(i = 0;i<iBcount;i++)
    {
        strcpy(szSeller, blist.szSeller[i]);
        strcpy(szItem, blist.szItem[i]);
        strcpy(szPrice, blist.szPrice[i]);
        
        if(FindMatchInBidder(bidd1, szSeller, szItem, szPrice, &iBiddPrice1))
        {
            if(FindMatchInBidder(bidd2, szSeller, szItem, szPrice, &iBiddPrice2))
            {
                if(iBiddPrice1 >= iBiddPrice2)
                {
                    //Sell to Bidd1
                    strcpy(sellList[iItemsSold].szSeller, szSeller);
                    strcpy(sellList[iItemsSold].szBidder, bidd1.szBidder);
                    strcpy(sellList[iItemsSold].szItem,szItem);
                    sellList[iItemsSold].iPrice = atoi(szPrice);
                    sellList[iItemsSold].iProfit = iBiddPrice1 - atoi(szPrice);
                    iItemsSold++;
                }
                else
                {
                    //Sell to Bidd2
                    strcpy(sellList[iItemsSold].szSeller, szSeller);
                    strcpy(sellList[iItemsSold].szBidder, bidd2.szBidder);
                    strcpy(sellList[iItemsSold].szItem,szItem);
                    sellList[iItemsSold].iPrice = atoi(szPrice);
                    sellList[iItemsSold].iProfit = iBiddPrice2 - atoi(szPrice);                    
                    iItemsSold++;
                }
            }
            else
            {
                //Sell to Bidd1
                strcpy(sellList[iItemsSold].szSeller, szSeller);
                strcpy(sellList[iItemsSold].szBidder, bidd1.szBidder);
                strcpy(sellList[iItemsSold].szItem,szItem);
                sellList[iItemsSold].iPrice = atoi(szPrice);
                sellList[iItemsSold].iProfit = iBiddPrice1 - atoi(szPrice);                
                iItemsSold++;
            }
        }
        else
        {
            if(FindMatchInBidder(bidd2, szSeller, szItem, szPrice, &iBiddPrice2))
            {
                //Sell to Bidd2
                strcpy(sellList[iItemsSold].szSeller, szSeller);
                strcpy(sellList[iItemsSold].szBidder, bidd2.szBidder);
                strcpy(sellList[iItemsSold].szItem,szItem);
                sellList[iItemsSold].iPrice = atoi(szPrice);
                sellList[iItemsSold].iProfit = iBiddPrice2 - atoi(szPrice);   
                iItemsSold++;
            }
            else
            {
                //Dont sell
            }
        }        
    }
}

int FindMatchInBidder(BiddingList bidd, char *pszSeller, char *pszItem, char *pszPrice, int *pBiddPrice)
{
    char szSeller[20], szItem[20], szPrice[20];
    int i = 0;
    int iMatch = 0;    
    int iOld = atoi(pszPrice);
    
    while(1)
    {
        strcpy(szSeller, bidd.szSellers[i]);
        strcpy(szItem, bidd.szItems[i]);
        strcpy(szPrice, bidd.szPrice[i]);
        
        if(strcmp(szSeller, "") == 0 || strcmp(szItem, "") == 0 || strcmp(szPrice, "") == 0)
        {
            break;
        }
        
        if(strcmp(szSeller, pszSeller) == 0 && strcmp(szItem, pszItem) == 0)
        {
            iMatch = 1;
            *pBiddPrice = atoi(szPrice);
            
            if(*pBiddPrice - iOld >= 0)
            {
                iMatch = 1;
            }            
            break;
        }
        i++;
    }
    
    
    return iMatch;
}

void PrintFinalResult()
{
    char szItem[20], szPrice[20];
    
    int i = 0;
    while(1)
    {
        strcpy(szItem, sellList[i].szItem);
        if(strcmp(szItem, "") == 0)
            break;
        
        sprintf(szPrice, "%d", sellList[i].iPrice + sellList[i].iProfit);
        
        printf("\nPhase 3: Item %s was sold at price %s", szItem, szPrice);
        i++;
    }
}

void PostResult_Bidd1()
{
    int iSock, iPortBidd = 3933;
    struct sockaddr_in addrBidd1;
    char buffTx[1024];
    char szUser[20];
    char szKimmat[20]; //Hindi term for price
    
    iSock = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&addrBidd1, 0 , sizeof(addrBidd1));
    addrBidd1.sin_family = AF_INET;
    addrBidd1.sin_port = htons(iPortBidd);
    addrBidd1.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(connect(iSock, (struct sockaddr*)&addrBidd1, sizeof(addrBidd1)) == -1)
    {
        perror("Connect:");
    }
    
    strcpy(szUser, bidd1List.szBidder);        
    
    strcpy(buffTx, "SoldList#");
    
    int i = 0;
    while(1)
    {
        if(strcmp(sellList[i].szBidder, "") == 0 || strcmp(sellList[i].szSeller, "") == 0)
        {
            break;
        }
        
        if(strcmp(sellList[i].szBidder, szUser) == 0)
        {
            strcat(buffTx, " ");
            strcat(buffTx, sellList[i].szItem);
            strcat(buffTx, " ");
            sprintf(szKimmat, "%d", sellList[i].iProfit + sellList[i].iPrice);
            strcat(buffTx, szKimmat);
        }        
        i++;
    }
    
    send(iSock, buffTx, strlen(buffTx), 0);        
}
 
void PostResult_Bidd2()
{
    int iSock, iPortBidd = 4033;
    struct sockaddr_in addrBidd2;
    char buffTx[1024];
    char szUser[20];
    char szKimmat[20]; //Hindi term for price
    
    iSock = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&addrBidd2, 0 , sizeof(addrBidd2));
    addrBidd2.sin_family = AF_INET;
    addrBidd2.sin_port = htons(iPortBidd);
    addrBidd2.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(connect(iSock, (struct sockaddr*)&addrBidd2, sizeof(addrBidd2)) == -1)
    {
        perror("Connect:");
    }
    
    strcpy(szUser, bidd2List.szBidder);        
    
    strcpy(buffTx, "SoldList#");
    
    int i = 0;
    while(1)
    {
        if(strcmp(sellList[i].szBidder, "") == 0 || strcmp(sellList[i].szSeller, "") == 0)
        {
            break;
        }
        
        if(strcmp(sellList[i].szBidder, szUser) == 0)
        {
            strcat(buffTx, " ");
            strcat(buffTx, sellList[i].szItem);
            strcat(buffTx, " ");
            sprintf(szKimmat, "%d", sellList[i].iProfit + sellList[i].iPrice);
            strcat(buffTx, szKimmat);
        }        
        i++;
    }
    
    send(iSock, buffTx, strlen(buffTx), 0);  
}

void PostResult_Sell1()
{
    int iSock, iPortBidd = 2933;
    struct sockaddr_in addrSell1;
    char buffTx[1024];
    char szUser[20];
    char szKimmat[20]; //Hindi term for price
    
    iSock = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&addrSell1, 0 , sizeof(addrSell1));
    addrSell1.sin_family = AF_INET;
    addrSell1.sin_port = htons(iPortBidd);
    addrSell1.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(connect(iSock, (struct sockaddr*)&addrSell1, sizeof(addrSell1)) == -1)
    {
        perror("Connect:");
    }
    
    strcpy(szUser, lstItem[0].szUser);        
    
    strcpy(buffTx, "SoldList#");
    
    int i = 0;
    while(1)
    {
        if(strcmp(sellList[i].szBidder,"") == 0 || strcmp(sellList[i].szSeller, "") == 0)
        {
            break;
        }
        
        if(strcmp(sellList[i].szSeller, szUser) == 0)
        {
            strcat(buffTx, " ");
            strcat(buffTx, sellList[i].szItem);
            strcat(buffTx, " ");
            sprintf(szKimmat, "%d", sellList[i].iProfit + sellList[i].iPrice);   
            strcat(buffTx, szKimmat);
        }        
        i++;
    }
    
    send(iSock, buffTx, strlen(buffTx), 0);   
}

void PostResult_Sell2()
{
    int iSock, iPortBidd = 3033;
    struct sockaddr_in addrSell2;
    char buffTx[1024];
    char szUser[20];
    char szKimmat[20]; //Hindi term for price
    
    iSock = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&addrSell2, 0 , sizeof(addrSell2));
    addrSell2.sin_family = AF_INET;
    addrSell2.sin_port = htons(iPortBidd);
    addrSell2.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(connect(iSock, (struct sockaddr*)&addrSell2, sizeof(addrSell2)) == -1)
    {
        perror("Connect:");
    }
    
    strcpy(szUser, lstItem[1].szUser);        
    
    strcpy(buffTx, "SoldList#");
    
    int i = 0;
    while(1)
    {
        if(strcmp(sellList[i].szBidder,"") == 0 || strcmp(sellList[i].szSeller, "") == 0)
        {
            break;
        }
        
        if(strcmp(sellList[i].szSeller, szUser) == 0)
        {
            strcat(buffTx, " ");
            strcat(buffTx, sellList[i].szItem);
            strcat(buffTx, " ");
            sprintf(szKimmat, "%d", sellList[i].iProfit + sellList[i].iPrice);    
            strcat(buffTx, szKimmat);
        }        
        i++;
    }
    
    send(iSock, buffTx, strlen(buffTx), 0);   
}
