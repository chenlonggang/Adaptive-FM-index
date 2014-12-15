CC=gcc 
CFLAGS=  -O3 -Wall  -DHAVE_CONFIG_H 
libdivsufsort.a:divsufsort.o sssort.o trsort.o utils.o
	ar rc libdivsufsort.a divsufsort.o sssort.o trsort.o utils.o

%.o:%.cpp *.h
	$(CC) -c $(CFLAGS) $< -o $@
clean:
	rm *.a *.o
