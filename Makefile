CFLAGS += -Wall -Werror
CFLAGS += -ggdb

LDLIBS += -lm

mandelbrot_bmp: mandelbrot
	./mandelbrot -2 1 -1.3 1.3 > t.bmp
	eog t.bmp &

clean:
	@rm -f mandelbrot *.o 2&>/dev/null
