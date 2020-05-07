all: menu_map

menu_map: menu_map.o
	gcc  -g -m32 -o menu_map menu_map.o
	rm menu_map.o

menu_map.o: menu_map.c
	gcc  -g -m32 -c -o menu_map.o menu_map.c

.PHONY: clean
clean:
	rm -rf ./*.o menu_map
