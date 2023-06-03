todo: spinningcube_withlight

LDLIBS=-lGL -lGLEW -lglfw -lm 

spinningcube_withlight: spinningcube_withlight.o textfile.o

clean:
	rm -f *.o *~

cleanall: clean
	rm -f spinningcube_withlight
