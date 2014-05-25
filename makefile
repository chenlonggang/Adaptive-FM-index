CiC=g++
CFLAGS=  -O3  -Wall -std=c++11
my_fm:ds_ssortr main.o fm.a
	g++ main.o fm.a   -o my_fm

fm.a:ABS_WT.o Balance_WT.o Huffman_WT.o Hutacker_WT.o FM.o WT_Node.o InArray.o loadkit.o savekit.o ds_ssort/ds.o ds_ssort/globals.o ds_ssort/helped.o ds_ssort/shallow.o ds_ssort/ds.o ds_ssort/globals.o ds_ssort/helped.o ds_ssort/shallow.o ds_ssort/deep2.o ds_ssort/blind2.o
	ar rc fm.a ABS_WT.o Balance_WT.o Huffman_WT.o Hutacker_WT.o FM.o WT_Node.o  InArray.o loadkit.o savekit.o ds_ssort/ds.o ds_ssort/globals.o ds_ssort/helped.o ds_ssort/shallow.o ds_ssort/ds.o ds_ssort/globals.o ds_ssort/helped.o ds_ssort/shallow.o ds_ssort/deep2.o ds_ssort/blind2.o

%.o:%.cpp *.h
	$(CC) -c  $(CFLAGS) $< -o $@

main.o:main.cpp  FM.h
	g++ -c  main.cpp

clean:
	rm *.a  *.o ds_ssort/*.a ds_ssort/*.o ds_ssort/ds ds_ssort/bwt ds_ssort/unbwt ds_ssort/testlcp my_fm;

ds_ssortr:
	make -C ./ds_ssort/; cp ./ds_ssort/ds_ssort.a .
