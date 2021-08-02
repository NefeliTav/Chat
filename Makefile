all: p enc
p: p.c
	gcc  -std=gnu11 -o p p.c -lpthread
enc: enc.c
	gcc  -std=gnu11 -o enc enc.c -lpthread -lcrypto
clean:
	rm -rf enc p