OUTPUTS=build bin
FOLDERS=bin build

all: WTF	WTFServer

WTF: WTF.c
	gcc -g WTF.c -o WTF -lcrypto

WTFServer: WTFServer.c
	gcc -g -o WTFServer -pthread WTFServer.c

test: WTFTest.c
	gcc -g -o WTFTest WTFTest.c
	./WTFTest

clean:
	rm -rf clientTest
	rm -rf serverTest
	rm -rf WTF
	rm -rf WTFServer
	rm -rf WTFTest
