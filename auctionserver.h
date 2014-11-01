/* 
 * File:   auctionserver.h
 * Author: Shreyansh Daga
 *
 * Created on April 20, 2014, 6:16 PM
 */

#ifndef AUCTIONSERVER_H
#define	AUCTIONSERVER_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct MyUser
    {
        int iType;
        char szUserName[20];
        char szUserPass[20];
        char szUserAcc[20];
        char szUserIP[16];
        
    } MyUser;
    
    typedef struct ItemList
    {
        char szUser[20];
        char szItem[6][20];
        char szPrice[6][20];
    } ItemList;
    
    typedef struct BiddingList
    {
        char szBidder[20];
        char szItems[10][20];
        char szSellers[10][20];
        char szPrice[10][20];
    } BiddingList;
    
    typedef struct BroadcastList
    {
        char szSeller[15][20];
        char szItem[15][20];
        char szPrice[15][20];
    } BroadcastList;
    
    typedef struct SellList
    {
        char szBidder[20];
        char szSeller[20];
        char szItem[20];
        int iPrice;
        int iProfit;
    } SellList;

#ifdef	__cplusplus
}
#endif

#endif	/* AUCTIONSERVER_H */

