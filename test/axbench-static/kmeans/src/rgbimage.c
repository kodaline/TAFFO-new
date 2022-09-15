/*
 * rgbimage.c
 * 
 * Created on: Sep 9, 2013
 * 			Author: Amir Yazdanbakhsh <a.yazdanbakhsh@gatech.edu>
 */


#include "rgbimage.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void initRgbImage(RgbImage* image) {
	image->w = 0;
	image->h = 0;
	image->pixels = NULL;
	image->meta = NULL;
}

int readCell(FILE *fp, char* w) {
	int __attribute((annotate("scalar(disabled range(1,255))"))) c;
	int i;

	w[0] = 0;
	for (c = fgetc(fp), i = 0; c != EOF; c = fgetc(fp)) {
		if (c == ' ' || c == '\t') {
			if (w[0] != '\"')
				continue;
		}

		if (c == ',' || c == '\n') {
			if (w[0] != '\"')
				break;
			else if (c == '\"') {
				w[i] = c;
				i++;
				break;
			}
		}

		w[i] = c;
		i++;
	}
	w[i] = 0;

	return c;
}

int loadRgbImage(const char* fileName,
		 RgbImage* __attribute((annotate(ANNOTATION_RGBIMAGE))) image,
		 float __attribute((annotate("scalar()"))) scale) {
	int c;
	int i;
	int j;
	char w[256];
	//RgbPixel** __attribute((annotate(ANNOTATION_RGBPIXEL))) pixels;
	RgbPixel**& __attribute((annotate(ANNOTATION_RGBPIXEL))) pixels = image->pixels;
	FILE *fp;

	//printf("Loading %s ...\n", fileName);

	fp = fopen(fileName, "r");
	if (!fp) {
		printf("Warning: Oops! Cannot open %s!\n", fileName);
		return 0;
	}

	c = readCell(fp, w);
	image->w = atoi(w);
	c = readCell(fp, w);
	image->h = atoi(w);

	//printf("%d x %d\n", image->w, image->h);

	pixels = (RgbPixel**)malloc(image->h * sizeof(RgbPixel*));

	if (pixels == NULL) {
		printf("Warning: Oops! Cannot allocate memory for the pixels!\n");

		fclose(fp);

		return 0;
	}

	c = 0;
	for(i = 0; i < image->h; i++) {
		pixels[i] = (RgbPixel*)malloc(image->w * sizeof(RgbPixel));
		if (pixels[i] == NULL) {
			c = 1;
			break;
		}
	}

	if (c == 1) {
		printf("Warning: Oops! Cannot allocate memory for the pixels!\n");

		for (i--; i >= 0; i--)
			free(pixels[i]);
		free(pixels);

		fclose(fp);

		return 0;
	}

	for(i = 0; i < image->h; i++) {
		for(j = 0; j < image->w; j++) {
			c = readCell(fp, w);
			float __attribute((annotate("scalar(range(1,255))"))) r = atoi(w);
			pixels[i][j].r = r / scale;

			c = readCell(fp, w);
			float __attribute((annotate("scalar(range(1,255))"))) g = atoi(w);
			pixels[i][j].g = g / scale;

			c = readCell(fp, w);
			float __attribute((annotate("scalar(range(1,255))"))) b = atoi(w);
			pixels[i][j].b = b / scale;

			pixels[i][j].cluster = 0;
			pixels[i][j].distance = 0.;
		}
	}
	//image->pixels = pixels;

	c = readCell(fp, w);
	image->meta = (char*)malloc(strlen(w) * sizeof(char));
	if(image->meta == NULL) {
		printf("Warning: Oops! Cannot allocate memory for the pixels!\n");

		for (i = 0; i < image->h; i++)
			free(pixels[i]);
		free(pixels);

		fclose(fp);

		return 0;

	}
	strcpy(image->meta, w);

	//printf("%s\n", image->meta);

	//printf("w=%d x h=%d\n", image->w, image->h);

	return 1;
}

int saveRgbImage(RgbImage* __attribute((annotate(ANNOTATION_RGBIMAGE))) image,
		 const char* fileName,
		 float __attribute((annotate("scalar()"))) scale) {
	int i;
	int j;
	FILE *fp;

	//printf("Saving %s ...\n", fileName);

	fp = fopen(fileName, "w");
	if (!fp) {
		printf("Warning: Oops! Cannot open %s!\n", fileName);
		return 0;
	}

	fprintf(fp, "%d,%d\n", image->w, image->h);
	//printf("%d,%d\n", image->w, image->h);

	for(i = 0; i < image->h; i++) {
		for(j = 0; j < (image->w - 1); j++) {
			fprintf(fp, "%d,%d,%d,", int(image->pixels[i][j].r * scale), int(image->pixels[i][j].g * scale), int(image->pixels[i][j].b * scale));
		}
		fprintf(fp, "%d,%d,%d\n", int(image->pixels[i][j].r * scale), int(image->pixels[i][j].g * scale), int(image->pixels[i][j].b * scale));
	}

	fprintf(fp, "%s", image->meta);
	//printf("%s\n", image->meta);

	fclose(fp);

	return 1;
}

void freeRgbImage(RgbImage* image) {
	int i;

	if (image->meta != NULL)
		free(image->meta);

	if (image->pixels == NULL)
		return;

	for (i = 0; i < image->h; i++)
		if (image->pixels != NULL)
			free(image->pixels[i]);

	free(image->pixels);
}

void makeGrayscale(RgbImage* __attribute((annotate(ANNOTATION_RGBIMAGE))) image) {
	int i;
	int j;
	float __attribute((annotate("scalar()"))) luminance;

	float __attribute((annotate("scalar()"))) rC = 0.30;
	float __attribute((annotate("scalar()"))) gC = 0.59;
	float __attribute((annotate("scalar()"))) bC = 0.11;

	for(i = 0; i < image->h; i++) {
		for(j = 0; j < image->w; j++) {
			luminance =
				rC * image->pixels[i][j].r +
				gC * image->pixels[i][j].g +
				bC * image->pixels[i][j].b;

			image->pixels[i][j].r = (unsigned char)luminance;
			image->pixels[i][j].g = (unsigned char)luminance;
			image->pixels[i][j].b = (unsigned char)luminance;
		}
	}
}
