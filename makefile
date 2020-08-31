target1=Encryption
target2=Decryption
src1=Encryption.o base64.o
src2=Decryption.o base64.o
src=$(wildcard ./*.c)
obj=$(patsubst ./%.c, ./%.o, $(src))
CC=gcc
all:$(target1) $(target2)
$(target1):$(src1)
	$(CC) $(src1) -lcrypto -o $(target1)
$(target2):$(src2)
	$(CC) $(src2) -lcrypto -o $(target2)
Encryption.o:Encryption.c
	$(CC) -lcrypto -c Encryption.c
Decryption.o:Decryption.c
	$(CC) -lcrypto -c Decryption.c 
base64.o:base64.c base64.h
	$(CC) -c base64.c  
.PHONY:clean
clean:
	rm -f $(obj) $(target1) $(target2)
