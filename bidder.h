/* 
 * File:   bidder1.h
 * Author: pushkar
 *
 * Created on April 21, 2014, 3:51 PM
 */

#ifndef BIDDER1_H
#define	BIDDER1_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct Bidder
    {
        char szBidderName[20];
        char szBidderPass[20];
        char szBidderAcc[20];
        int iType;
    } Bidder;



#ifdef	__cplusplus
}
#endif

#endif	/* BIDDER1_H */

