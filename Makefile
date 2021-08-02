all: p enc chan
p: p.c
	gcc  -std=gnu11 -o p p.c -lpthread
enc: enc.c
	gcc  -std=gnu11 -o enc enc.c -lpthread -lcrypto
chan: chan.c
	gcc  -std=gnu11 -o chan chan.c -lpthread
clean:
	rm -rf chan enc p
#make 
#p1 ---> ./p -p 0.1   
#p2 ---> ./p
