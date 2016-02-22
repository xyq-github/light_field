all:		rmanpp lidquery

rmanpp:		rmanpp.cpp
		g++ -o rmanpp rmanpp.cpp

lidquery:  	lidquery.o lidquery_main.o
		cc -o lidquery lidquery.o lidquery_main.o

lidquery.o: 	lidquery.c lidquery.h
		cc -c lidquery.c

lidquery_main.o:	lidquery_main.c lidquery.h
		cc -c lidquery_main.c

clean:  
		\rm -f lidquery lidquery.o lidquery_main.o
		\rm -f rmanpp
