all: sample2D

sample2D: first_try.cpp glad.c
	g++ -g -o sample2D first_try.cpp glad.c -lGL -lglfw -ldl -lSOIL -lftgl -L/usr/local/lib -I/usr/include -I/usr/include/freetype2

clean:
	rm sample2D
