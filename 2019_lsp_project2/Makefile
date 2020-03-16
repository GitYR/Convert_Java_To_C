RESULT_OBJECTS = main.o setting.o convert.o

ssu_converter : $(RESULT_OBJECTS)
	gcc -o ssu_converter $(RESULT_OBJECTS) -g
	make clean

main.o : convert.h main.c
	gcc -c -o main.o main.c

setting.o : convert.h setting.c
	gcc -c -o setting.o setting.c

convert.o : convert.h convert.c
	gcc -c -o convert.o convert.c

clean :
	rm $(RESULT_OBJECTS)
