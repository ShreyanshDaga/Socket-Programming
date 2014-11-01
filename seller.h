/* 
 * File:   seller.h
 * Author: pushkar
 *
 * Created on April 21, 2014, 11:24 PM
 */

#ifndef SELLER_H
#define	SELLER_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct Seller
    {
        char szServerIP[20];
        char szPortPhase3[10];
        char szPortPhase2[10];
        char szSellerName[20];
        char szSellerPass[20];
        char szSellerAcc[20];
        int iType;
    } Seller;        


#ifdef	__cplusplus
}
#endif

#endif	/* SELLER_H */

