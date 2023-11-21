/*
	Projeto desenhado pelo Professor João Manuel Rodrigues (jmr@ua.pt).

	Alunos: Olha Buts (112920) & André Correia (87818)

	Data: 24.11.2023

	Nota: Para ver a definição de cada função (descrição, pré-condições, pós-condições, etc.), passar o cursor por cima do nome da respetiva função.
*/


// Libraries:

#include "image8bit.h"
#include "instrumentation.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


// Directives & Macros:

#define PIXMEM InstrCount[0]										// Número de Acessos a Pixels.
#define VALCOMP InstrCount[1]										// Número de Comparações entre Pixels.
#define FORCNT InstrCount[2]										// Número de Ciclos de um For.
#define COMP_EXTREMO_INICIAL(a,b) ( a < b ? b : a )					// Macro Auxiliar ao Blur Otimizado Para Determinar o Valor do Kernel Aplicável ao Lado Esquerdo e Lado Superior.
#define COMP_EXTREMO_FINAL(a,b) ( a >= b ? b - 1 : a )				// Macro Auxiliar ao Blur Otimizado Para Determinar o Valor do Kernel Aplicável ao Lado Direito e Lado Inferior.


// Declarações & Instanciações [Globais]:

struct image { int width; int height; int maxval; uint8* pixel;};	// Estrutura Base do Programa (Armazena Graymap de 8 Bits).
const uint8 PixMax = 255; 											// Valor Máximo do Pixel.
static int errsave = 0; 											// Variável para Preservar o "errno" temporariamente.
static char* errCause;												// Variável para Registar a Causa do Erro.












// Funções Para Gestão de Erros:

char* ImageErrMsg() { return errCause; }

static int check(int condition, const char* failmsg)
{
	errCause = (char*)(condition ? "" : failmsg);
	return condition;
} 












// Função Para Calibração dos Intrumentos de Medição de Complexidade Computacional:

void ImageInit(void)
{
	InstrCalibrate();
	InstrName[0] = "pixmem";		// Número de Acessos a Pixels.
	InstrName[1] = "valcomp";		// Número de Comparações entre Pixels.
	InstrName[2] = "forcnt";		// Número de Ciclos de um For.
}












// Funções Construtoras & Destrutoras da Estrutura:

Image ImageCreate(int width, int height, uint8 maxval)
{
	// Precondições:
	assert (width >= 0);
	assert (height >= 0);
	assert (0 < maxval && maxval <= PixMax);

	// Declarações & Instanciações:
	Image myImg = (Image)malloc(sizeof(struct image));
	if (myImg == NULL) {errCause ="The System Cannot Allocate Memory for the new Black Image."; errno = 12; return NULL;}

	// Atribuição de Valores aos Membros da Estrutura:
	myImg->height = height;
	myImg->width = width;
	myImg->maxval = maxval;

	// Alocação de Memória para o Vetor 1D que Armazena a Imagem (2D):
	myImg->pixel = (uint8*)malloc(sizeof(uint8)*width*height);
	if (myImg->pixel == NULL)
	{
		errCause ="The System Cannot Allocate Memory for a Raster Scan in a new Blank Image."; 
		errsave = errno;
		free(myImg); // Liberta a Memória Alocada Anteriormente.
		errno = errsave;
		return NULL;
	}

	
	
	// Return:
	return myImg;
}

void ImageDestroy(Image* imgp)
{
	// Precondições:
	assert (imgp != NULL);
	//assert (*imgp != NULL);

	//Preserva errno/errCausa global
	int success = check(*imgp != NULL, "Invalid Pointer!" );
	if (success) {
		// Libertação da Memória do Array 1D da Imagem:
		free((*imgp)->pixel);

		// Libertação da Memória da Estrutura:
		free((*imgp));
		*imgp = NULL;	
	}
	else
	{
		errno = 14;
	}
}












// Funções para Leitura e Escrita de Imagens do Formato .PGM (Tipo P2):

static int skipComments(FILE* f) // Função Auxiliar by JMR.
{
	char c;
	int i = 0;
	while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {i++;}
	return i;
} 

Image ImageLoad(const char* filename) /* by JMR */
{
	// Declarações & Instanciações:
	int w, h;
	int maxval;
	char c;
	FILE* f = NULL;
	Image img = NULL;

	// Processamento:
	int success = 
	check( (f = fopen(filename, "rb")) != NULL, "Open Failed!" ) &&
	// Parse PGM header
	check( fscanf(f, "P%c ", &c) == 1 && c == '5' , "Invalid File Format!" ) &&
	skipComments(f) >= 0 &&
	check( fscanf(f, "%d ", &w) == 1 && w >= 0 , "Invalid Width!" ) &&
	skipComments(f) >= 0 &&
	check( fscanf(f, "%d ", &h) == 1 && h >= 0 , "Invalid Height!" ) &&
	skipComments(f) >= 0 &&
	check( fscanf(f, "%d", &maxval) == 1 && 0 < maxval && maxval <= (int)PixMax , "Invalid maxval!" ) &&
	check( fscanf(f, "%c", &c) == 1 && isspace(c) , "Whitespace Expected!" ) &&
	// Allocate image
	(img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
	// Read pixels
	check( fread(img->pixel, sizeof(uint8), w*h, f) == w*h , "Reading Pixels!" );
	PIXMEM += (unsigned long)(w*h);	// count pixel memory accesses

	// Cleanup:
	if (!success) {
		errsave = errno;
		ImageDestroy(&img);
		errno = errsave;
	}
	if (f != NULL) fclose(f);

	// Return:
	return img;
}

int ImageSave(Image img, const char* filename) /* by JMR */
{
	// Declarações & Instanciações:
	assert (img != NULL);
	int w = img->width;
	int h = img->height;
	uint8 maxval = img->maxval;
	FILE* f = NULL;

	// Processamento:
	int success =
	check( (f = fopen(filename, "wb")) != NULL, "Open Failed!" ) &&
	check( fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0, "Writing Header Failed!" ) &&
	check( fwrite(img->pixel, sizeof(uint8), w*h, f) == w*h, "Writing Pixels Failed!" ); 
	PIXMEM += (unsigned long)(w*h);	// count pixel memory accesses

	// Cleanup:
	if (f != NULL) fclose(f);

	// Return:
	return success;
}












// Getters & Setters da Estrutura:

int ImageWidth(Image img)
{
	// Precondições:
	assert (img != NULL);

	// Return:
	return img->width;
}

int ImageHeight(Image img)
{
	// Precondições:
	assert (img != NULL);

	// Return:
	return img->height;
}

int ImageMaxval(Image img)
{
	// Precondições:
	assert (img != NULL);

	// Return:
	return img->maxval;
}

void ImageStats(Image img, uint8* min, uint8* max)
{
	// Precondições:
	assert (img != NULL);
	
	// Declarações & Instanciações:
	*max = (img->maxval);
	*min = *max;

	// Processamento:
	for (int i = 0; i < (img->height)*(img->width); i++)
	{
		if (img->pixel[i] < *min) {*min = img->pixel[i];}
	}
}












// Funções Auxiliares aos Algoritmos de Processamento de Imagem Posteriores:

int ImageValidPos(Image img, int x, int y)
{
	// Precondições:
	assert (img != NULL);

	// Return:
	return (0 <= x && x < img->width) && (0 <= y && y < img->height);
}

int ImageValidRect(Image img, int x, int y, int w, int h)
{
	// Precondições:
	assert (img != NULL);

	// Verificação de ImageValidPos():
	int validPos = ImageValidPos(img, x, y);
	if (validPos == 0) {return 0;}

	// Verifica se a Área do Retângulo Está Dentro da img:
	int largura = x + w;
	int altura = y + h;
	if (!ImageValidPos(img, largura, altura)) {return 0;}

	// Return:
	return 1;
}

static inline int G(Image img, int x, int y)
{
	int index;
	index = y * img->width + x;
	assert (0 <= index && index < img->width*img->height);
	return index;
}

uint8 ImageGetPixel(Image img, int x, int y)
{
	// Precondições:
	assert (img != NULL);
	assert (ImageValidPos(img, x, y));

	// For Performance Metrics:
	PIXMEM += 1;

	// Return:
	return img->pixel[G(img, x, y)];
}

void ImageSetPixel(Image img, int x, int y, uint8 level)
{
	// Precondições:
	assert (img != NULL);
	assert (ImageValidPos(img, x, y));

	// For Performance Metrics:
	PIXMEM += 1;

	// Return:
	img->pixel[G(img, x, y)] = level;
} 












// Algoritmos de Processamento de Imagens ao Nível do Pixel:

void ImageNegative(Image img)
{
	// Precondições:
	assert (img != NULL);
	
	// Processamento:
	for (int i = 0; i < img->width * img->height; i++)
	{
		img->pixel[i] = PixMax - img->pixel[i];
	}
}

void ImageThreshold(Image img, uint8 thr)
{
	// Precondições:
	assert (img != NULL);

	// Processamento:
	for (int i = 0; i < img->width * img->height; i++)
	{
		if (img->pixel[i] < thr) { img->pixel[i] = 0; }
		else { img->pixel[i] = PixMax; }
	}
}

void ImageBrighten(Image img, double factor)
{
	// Precondições:
	assert (img != NULL);
	assert (factor >= 0.0);

	// Declarações & Instanciações:
	int n;

	// Processamento:
	for (int i = 0; i < img->width * img->height; i++)
	{
		n = (int) img->pixel[i];
		n = n*factor + 0.5;
		if (n > (int)PixMax) {img->pixel[i] = PixMax;}
		else {img->pixel[i] = (uint8)n;}
	}
}

void ImageSetMaxValue(Image img)
{
	// Declarações & Instanciações:
	int maxValue = 0;

	// Processamento:
	for(int i = 0; i < img->width * img->height; i++)
	{
		if (img->pixel[i] > maxValue) {maxValue = img->pixel[i];}
	}

	// Resultado:
	img->maxval = maxValue;
}












// Algoritmos de Processamento de Imagens ao Nível Geométrico:

Image ImageRotate(Image img)
{
	// Precondições:
	assert (img != NULL);

	// Declarações & Instanciações:
	Image newImg = ImageCreate(img->height, img->width, img->maxval);
	int success = check(newImg != NULL, "Failed to Allocate Memory for Rotated Image!");
	if (!success){errno = 12; return NULL;}
	int heightNew = 0;
	uint8 pixelValue = 0;

	// Processamento:
	for (int widthOriginal = img->width - 1; widthOriginal >= 0; widthOriginal--)
	{
		for (int i = 0; i <= (img->height - 1); i++)
		{
			pixelValue = ImageGetPixel(img, widthOriginal, i);
			ImageSetPixel(newImg, i, heightNew, pixelValue);
		}
		heightNew++;
	}

	// Return:
	return newImg;
}

Image ImageMirror(Image img)
{
	// Precondições:
	assert (img != NULL);

	// Declarações & Instanciações:
	Image newImg = ImageCreate(img->width, img->height, img->maxval);
	 if (!check(newImg != NULL, "Failed to Allocate Memory for Mirrored Image!"))
	{
		errno = 12;
		return NULL;
	}
	uint8 pixelValue = 0;

	// Processamento:
	for (int heightOriginal = img->height - 1; heightOriginal >= 0; heightOriginal--)
	{
		for (int i = 0; i <= (img->width - 1); i++)
		{
			pixelValue = ImageGetPixel(img, i, heightOriginal);
			ImageSetPixel(newImg, img->width-1 - i, heightOriginal, pixelValue);
		}
	}

	// Return:
	return newImg;
}

Image ImageCrop(Image img, int x, int y, int w, int h)
{
	// Precondições:
	assert (img != NULL);
	assert (ImageValidRect(img, x, y, w, h));

	// Declarações & Instanciações:
	Image newImg = ImageCreate(w, h, 1);
	if (!check(newImg != NULL, "Failed to Allocate Memory for Cropped Image!"))
	{
		errno = 12;
		return NULL;
	}
	uint8_t pixelValue = 0;
	int index = 0;

	// Processamento:
	for (int numLinhas = y; numLinhas < y+h; numLinhas++)
	{
		for (int i = 0; i < w; i++)
		{
			pixelValue = ImageGetPixel(img, x+i, numLinhas);
			newImg->pixel[index] = pixelValue;
			index++;
		}
	}
	ImageSetMaxValue(newImg);

	// Return:
	return newImg;
}












// Algoritmos de Processamento de Duas Imagens (uma sobre a outra):

void ImagePaste(Image img1, int x, int y, Image img2)
{
	// Precondições:
	assert (img1 != NULL);
	assert (img2 != NULL);
	assert (ImageValidRect(img1, x, y, img2->width, img2->height));

	// Declarações & Instanciações:
	uint8_t img2PixelValue = 0;

	// Processamento:
	for (int img1NumLinhas = y, img2NumLinhas = 0; img1NumLinhas < y+img2->height; img1NumLinhas++, img2NumLinhas++)
	{
		for (int i = 0; i < img2->width; i++)
		{
			img2PixelValue = ImageGetPixel(img2, i, img2NumLinhas);
			ImageSetPixel(img1, x+i, img1NumLinhas, img2PixelValue);
		}
	}
	ImageSetMaxValue(img1);
}

void ImageBlend(Image img1, int x, int y, Image img2, double alpha)
{
	// Precondições:
	assert (img1 != NULL);
	assert (img2 != NULL);
	assert (ImageValidRect(img1, x, y, img2->width, img2->height));

	// Declarações & Instanciações:
	double blend;
	uint8 img1PixelValue = 0;
	uint8 img2PixelValue = 0;

	// Processamento:
	for (int img1NumLinhas = y, img2NumLinhas = 0; img1NumLinhas < y+img2->height; img1NumLinhas++, img2NumLinhas++)
	{
		for (int i = 0; i < img2->width; i++)
		{
			img1PixelValue = ImageGetPixel(img1, x+i, img1NumLinhas);
			img2PixelValue = ImageGetPixel(img2, i, img2NumLinhas);
	
			blend = ((((1-alpha)*img1PixelValue) + (alpha*img2PixelValue)) + 0.5);

			if (blend > (double) PixMax) {ImageSetPixel(img1, x+i, img1NumLinhas, PixMax);}
			else if (blend < 0.0) {ImageSetPixel(img1, x+i, img1NumLinhas, (uint8)0);}
			else {ImageSetPixel(img1, x+i, img1NumLinhas, (uint8) blend);}
		}
	}
	ImageSetMaxValue(img1);
}

int ImageMatchSubImage(Image img1, int x, int y, Image img2)
{
	// Precondições:
	assert (img1 != NULL);
	assert (img2 != NULL);
	assert (ImageValidPos(img1, x, y));
	if (!ImageValidRect(img1, x, y, img2->width, img2->height)) { return 0; }
	
	// Processamento:
	for (int yRaster = 0; yRaster < img2->height; yRaster++)
	{
		for (int xRaster = 0; xRaster < img2->width; xRaster++)
		{
			// For Performance Metrics:
			VALCOMP += 1; 
			FORCNT += 1;

			if (ImageGetPixel(img1, xRaster + x, yRaster + y) != ImageGetPixel(img2, xRaster, yRaster)) { return 0; }
		}
		// For Performance Metrics:
		FORCNT += 1;
	}

	// Return:
	return 1;
}

int ImageLocateSubImage(Image img1, int* px, int* py, Image img2)
{
	// Precondições:
	assert (img1 != NULL);
	assert (img2 != NULL);
	
	// Processamento:
	for (int y = 0; img2->height < (img1->height - y); y++)
	{
		for (int x = 0; img2->width < (img1->width - x); x++)
		{
			FORCNT += 1; // For Performance Metrics.
			if (ImageMatchSubImage(img1, x, y, img2) == 1) { return 1; };
		}

		// For Performance Metrics:
		FORCNT += 1;
	}

	// Return:
	return 0;
}

void ImageBlur_NotOptimized(Image img, int dx, int dy)
{
	// Precondições:
	assert(img != NULL);
	assert(dx > 0);
	assert(dy > 0);

	// Declarações & Instanciações:
	Image imgTemp = ImageCreate(img->width, img->height, img->maxval);
		if (imgTemp == NULL) {errCause ="The System Cannot Allocate Memory for the Temporary Image."; errno = 12;}
			for (int i = 0; i < img->width * img->height; i++) {imgTemp->pixel[i] = img->pixel[i];} // Cópia do img->pixel para o imgTemp->pixel.
	double med = 0;
	double sum = 0;
	double cnt = 0;
	
	// Processamento:
	for (int yRaster = 0; yRaster < img->height; yRaster++)
	{
		for (int xRaster = 0; xRaster < img->width; xRaster++)
		{
			// Sum Dos Pixels:
			for (int yPixel = -dy; yPixel <= dy; yPixel++)
			{
				FORCNT += 1; // For Performance Metrics.
				for (int xPixel = -dx; xPixel <= dx; xPixel++)
				{
					FORCNT += 1; // For Performance Metrics.
					if (ImageValidPos(imgTemp, xRaster+xPixel, yRaster+yPixel)) { sum += (double) ImageGetPixel(imgTemp, xRaster+xPixel, yRaster+yPixel); cnt++;}
				}
			}

			// Média:
			med = (sum / cnt) + 0.5;

			// Substituição do Pixel:
			ImageSetPixel(img, xRaster, yRaster, (uint8) med);

			// Reset dos Valores:
			med = 0;
			sum = 0;
			cnt = 0;

			// For Performance Metrics:
			FORCNT += 1;
		}
		
		// For Performance Metrics:
		FORCNT += 1;
	}

	// Garbage Collector:
	ImageDestroy(&imgTemp);
}

void ImageBlur(Image img, int dx, int dy)
{
	// Precondições:
	assert(img != NULL);
	assert(dx > 0);
	assert(dy > 0);
	
	// Declarações & Instanciações:
	int* kernel = (int *) malloc(sizeof(int)*img->width*img->height); // Criação de um Vetor (1D) para Armazenar as Consecutivas Médias dos Sub-Quadrados da Imagem.
		if (kernel == NULL) {errCause = "The System Cannot Allocate Memory for Image Kernel."; errno = 12;}
			for (int i = 0; i < img->width * img->height; i++) { kernel[i] = img->pixel[i]; } // Cópia do img->pixel para o kernel*.
	double soma = 0;
	double area = 0;
	double media = 0;

	// Processamento Auxiliar que Armazena as Consecutivas Médias dos Sub-Quadrados da Imagem:
	for (int y = 0; y < img->height; y++)
	{
		for (int x = 0; x < img->width; x++)
		{
			FORCNT += 1; // For Performance Metrics.
			if (x > 0) { kernel[G(img, x, y)] += kernel[G(img, x - 1, y)]; }				// Adiciona o Elemento da Horizontal Anterior.
			if (y > 0) { kernel[G(img, x, y)] += kernel[G(img, x, y - 1)]; }				// Adiciona o Elemento da Vertical Anterior.
			if (x > 0 && y > 0) { kernel[G(img, x, y)] -= kernel[G(img, x - 1, y - 1)]; } 	// Retira o Elemento da Obliqua Anterior.
		}
		FORCNT += 1; // For Performance Metrics.
	}

	// Processamento Principal: (Aplicar o Blur na Imagem)
	for (int y = 0; y < img->height; y++)
	{
		FORCNT += 1; // For Performance Metrics.
		for (int x = 0; x < img->width; x++)
		{
			// Sum dos Pixels:
			soma = kernel[ G(img, COMP_EXTREMO_FINAL(x + dx, img->width), COMP_EXTREMO_FINAL(y + dy, img->height)) ] - 
				   kernel[ G(img, COMP_EXTREMO_FINAL(x + dx, img->width), COMP_EXTREMO_INICIAL(y - dy - 1, 0))	   ] - 
				   kernel[ G(img, COMP_EXTREMO_INICIAL(x - dx - 1, 0),    COMP_EXTREMO_FINAL(y + dy, img->height)) ] + 
				   kernel[ G(img, COMP_EXTREMO_INICIAL(x - dx - 1, 0),    COMP_EXTREMO_INICIAL(y - dy - 1, 0))     ];

			// Cálculo da Área do Pixel:
			area = (COMP_EXTREMO_FINAL(x + dx, img->width) - COMP_EXTREMO_INICIAL(x - dx - 1, 0)) *
				   (COMP_EXTREMO_FINAL(y + dy, img->height)- COMP_EXTREMO_INICIAL(y - dy - 1, 0));

			// Determinação da Média:
			media = (soma / area) + 0.5;

			// Aplicação do Valor de Blur ao Pixel:
			ImageSetPixel(img, x, y, (uint8) media);
		}
		FORCNT += 1; // For Performance Metrics.
	}

	// Garbage Collector:
	free(kernel);
}










// TestBench da Equipa: (deve estar comentada)
// Nota: Também foram usadas como testbenches as entradas do 'imageTest' e do 'imageTool'.
/*
void main()
{
	//Image myImg = ImageLoad("pgm/small/tools_2_765x460.pgm");
	//Image myImg2 = ImageLoad("test/small.pgm");
	//Image myImg2 = ImageLoad("test/blend.pgm");
	//ImageSave(myImg, "deleteMe3.pgm");

	//uint8_t min;
	//uint8_t max;
	//ImageStats(myImg, &min, &max);
	//printf_s("ImageValidPos = %d\n", ImageValidPos(myImg, 0, 0));
	//printf_s("ImageValidRect = %d\n", ImageValidRect(myImg, 85, 85, 10, 10));

	//int gLoc = G(myImg, 37, 34);
	//printf_s("Static Inline G: %d\n", gLoc);
	//printf_s("Color of G: %d\n", myImg->pixel[gLoc]);

	//ImageNegative(myImg);
	//ImageSave(myImg, "myTests\\negativeImage.pgm");

	//ImageThreshold(myImg, 155);
	//ImageSave(myImg, "myTests\\thresholdImage.pgm");

	//ImageBrighten(myImg, 1.5);
	//ImageSave(myImg, "myTests\\brightenImage.pgm");

	//Image newImg1 = ImageRotate(myImg);
	//ImageSave(newImg1, "myTests\\rotateImage.pgm");

	//Image newImg2 = ImageMirror(myImg);
	//ImageSave(newImg2, "myTests\\mirrorImage.pgm");

	//Image newImg3 = ImageCrop(myImg, 1, 1, 1400, 1000);
	//ImageSave(newImg3, "myTests\\cropImage.pgm");

	Image myImg = ImageLoad("ComplexityTests/ImageLocateTests/Sierra.pgm");
	Image myImg2 = ImageLoad("test/small.pgm");
	ImagePaste(myImg, 50, 50, myImg2);
	ImageSave(myImg, "ComplexityTests/ImageLocateTests/VeryLarge/Sierra.pgm");

	// cl image8bit.c instrumentation.c

	//Image myImg2 = ImageLoad("test/small.pgm");
	//ImageBlend(myImg, 100, 100, myImg2, 0.33);
	//ImageSave(myImg, "blendedImage.pgm");

	//Image myImg2 = ImageLoad("blur.pgm");
	//ImageBlur(myImg,7,7);
	//Image myImg2 = ImageLoad("test/blur.pgm");
	//int pixel = 0;
	//for (int i = 0; i < sizeof(uint8)*myImg->width*myImg->height; i++)
	//{
	//	if (myImg->pixel[i] != myImg2->pixel[i])
	//	{
	//		printf("Index = %d\nValueOfImg1 = %d\nValueOfImg2 = %d\n\n", i, myImg->pixel[i], myImg2->pixel[i]);
	//		pixel = i;
	//	}
	//}
	
	//for (int x = 0; x < 300; x++)
	//{
	//	for (int y = 0; y < 300; y++)
	//	{
	//		if (G(myImg2, x, y) == pixel) {printf_s("Coordinates: (%d,%d)\n", x, y);}
	//	}
	//}

	//Image myImg2 = ImageLoad("test/small.pgm");
	//printf_s("Result = %d\n", ImageMatchSubImage(myImg,100,100,myImg2));

	//Image myImg2 = ImageLoad("test/small.pgm");
	//int pxTemp = 0;
	//int pyTemp = 0;
	//int* px = &pxTemp;
	//int* py = &pyTemp;
	//printf_s("Result: %d\t(%d,%d)", ImageLocateSubImage(myImg, px, py, myImg2), *px, *py);
}*/