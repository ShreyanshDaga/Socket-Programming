EE450_Project_ShreyanshDaga:
	gcc -o auc auctionserver.c -lnsl -lsocket -lresolv
	gcc -o bidd1 bidder1.c -lnsl -lsocket -lresolv
	gcc -o bidd2 bidder2.c -lnsl -lsocket -lresolv
	gcc -o sell1 seller1.c -lnsl -lsocket -lresolv
	gcc -o sell2 seller2.c -lnsl -lsocket -lresolv	

clean:
	rm -f auc bidd1 bidd2 sell1 sell2
	
