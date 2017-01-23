all: sample2D

sample2D: Sample_GL3_2D.cpp glad.c
	g++ -o sample2D Sample_GL3_2D.cpp glad.c -lGL -lglfw -ldl -I/usr/local/include -I/usr/local/include/FTGL -I/usr/local/include/freetype2 -I/usr/include/libpng12 -L/usr/local/lib -lftgl

clean:
	rm sample2D
