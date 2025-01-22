main: kibic kierownik kontrola pracownik_techniczny
	rm *.o

kibic: kibic.o helpers.o
	gcc kibic.o helpers.o -o kibic -lpthread

kibic.o: kibic.c
	gcc -c kibic.c

helpers.o: helpers.c helpers.h
	gcc -c helpers.c

kierownik: kierownik.c
	gcc -o kierownik kierownik.c

kontrola: kontrola.o helpers.o
	gcc kontrola.o helpers.o -o kontrola

kontrola.o: kontrola.c
	gcc -c kontrola.c

pracownik_techniczny: pracownik_techniczny.o helpers.o
	gcc pracownik_techniczny.o helpers.o -o pracownik_techniczny

pracownik_techniczny.o: pracownik_techniczny.c helpers.c
	gcc -c pracownik_techniczny.c