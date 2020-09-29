#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#define PXSZ    1//4

struct bmap {
	uint16_t hdr;
	uint32_t sz;
	uint16_t rsv1;
	uint16_t rsv2;
	uint32_t offs;
	struct dib {
		uint32_t sz;
		uint32_t wd;
		uint32_t ht;
		uint16_t numclrplane;
		uint16_t bpp;
		uint32_t cmp;
		uint32_t imgsz;
		uint32_t hres;
		uint32_t vres;
		uint32_t numclrplt;
		uint32_t impclt;
	} dib;
	struct rgbquad {
		uint32_t rgb;
		/*uint8_t blue;
		uint8_t green;
		uint8_t red;
		uint8_t rsvd;*/
	} pm[256];
	uint8_t dat[];
} __attribute__((packed));

#define BI_RGB  0
#define BI_RLE8 1

void writebitmap(uint32_t x, uint32_t y, uint8_t *dat)
{
	struct bmap *b;
	uint32_t i, sz;

	sz = sizeof(*b) + PXSZ * x * y;
	b = malloc(sz);
	memset(b, 0, sizeof(*b));

	b->hdr = 0x4d42;
	b->sz = sz;
	b->offs = (uintptr_t)b->dat - (uintptr_t)b;

	b->dib.sz = sizeof(b->dib);
	b->dib.wd = x;
	b->dib.ht = y;
	b->dib.numclrplane = 1;
	b->dib.bpp = PXSZ * 8;
	b->dib.cmp = BI_RGB;
	//b->dib.cmp = BI_RLE8;

	if (b->dib.bpp == 8)
		for (i = 0; i < 256; i++)
			b->pm[i].rgb = i << 16 | i << 8 | i;

	memcpy(b->dat, dat, PXSZ * x * y);
	if (sz != write(1, b, sz))
		printf("write: %d %s\n", errno, strerror(errno));
}

struct cplx {
	double re;
	double im;
};

void cplx_sum(struct cplx *c1, struct cplx *c2)
{
	c1->re += c2->re;
	c1->im += c2->im;
}

void cplx_mul(struct cplx *c1, struct cplx *c2)
{
	double f = c1->re * c2->re - c1->im * c2->im;;

	c1->im = c1->re * c2->im + c1->im * c2->re;
	c1->re = f;
}

uint8_t test(struct cplx *c, int iters)
{
	struct cplx z = { 0.0f, 0.0f };
	uint8_t i;

	for (i = 0; i < iters; i++) {
		cplx_mul(&z, &z);
		cplx_sum(&z, c);

		if ((z.re * z.re + z.im * z.im) > 4.00f)
			return i;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	uint32_t i, x, y, ht, wd = 800;
	uint8_t *dat;
	char *e;
	float delta, x1, x2, y1, y2;
	struct cplx c;

	if (argc != 5) {
		printf("usage: %s x1 x2 y1 y2\n", argv[0]);
		return -1;
	}

	x1 = strtof(argv[1], NULL);
	x2 = strtof(argv[2], NULL);
	y1 = strtof(argv[3], NULL);
	y2 = strtof(argv[4], NULL);

	if (x1 > x2 || y1 > y2) {
		printf("usage: \n");
		return -1;
	}

	e = getenv("MNDL_WD");
	if (e)
		wd = strtol(e, NULL, 0);
	if (wd < 32 || wd > 5000) {
		printf("usage: 2\n");
		return -1;
	}

	delta = (x2 - x1) / (float)wd;
	ht = (uint32_t) (wd * (y2 - y1) / (x2 - x1));
	dat = malloc(PXSZ * wd * ht);

	fprintf(stderr, "%d x %d\n", wd, ht);

	for (y = 0; y < ht; y++) {
		for (x = 0; x < wd; x++) {
			i = PXSZ * x + y * wd * PXSZ;

			c.re = x1 + delta * (float)x;
			c.im = y1 + delta * (float)y;

			dat[i] = test(&c, 128) << 1;
			/*dat[i + 1] = dat[i];
			dat[i + 2] = dat[i] >> 1;
			dat[i + 3] = dat[i] >> 2;*/
		}
	}
	writebitmap(wd, ht, dat);

	return 0;
}
