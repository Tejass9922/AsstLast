all: WTF WTFServer


WTF: WTF.c
	gcc -g WTF.c -o WTF -lcrypto 

WTFServer: WTFServer.c
	gcc -g -o WTFServer -pthread WTFServer.c 

clean:
	rm -f WTF
	rm -f WTFServer
    rm -f *.o WTF WTFServer