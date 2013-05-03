// http://www.purple.dti.ne.jp/~t-ogura/animeEffect の
// LiveHatching リアルタイム手描き風エフェクトプログラム　openCV版
// を基にAviUtlプラグイン化しました

#include "AnimEffect.h"

#include "opencv/cv.h"
#include "opencv/highgui.h"

#include <stdio.h>		// printf() / scanf()
#define _USE_MATH_DEFINES
#include <math.h>

#include <string>
#include <vector>
#include <map>

#define YC2R(Y,Cb,Cr) (Y + 1.402*Cr)
#define YC2G(Y,Cb,Cr) (Y - 0.344*Cb - 0.714*Cr)
#define YC2B(Y,Cb,Cr) (Y + 1.772*Cb)
#define RGB2Y(R,G,B) (0.299*R + 0.587*G + 0.114*B)
#define RGB2CB(R,G,B) (-0.169*R - 0.331*G + 0.500*B)
#define RGB2CR(R,G,B) (0.500*R - 0.419*G - 0.081*B)

// [min, max] の範囲に飽和
template <class T>
T Saturate(T v, T min, T max) {
	if (v < min) {
		return min;
	} else if (v > max) {
		return max;
	}
	return v;
}

template <>
int Saturate(int v, int min, int max) {
    v -= min;
    v &= ~(v >> 31);
    v += min;

    v -= max;
    v &=  (v >> 31);
    v += max;
    
    return v;
}


/* After Effects -> OpenCV */
static void CopyPFToCvImg(PF_Pixel *input_p, int width, int height, int row,
						  IplImage *cvImg)
{
	/* assumes input and img has the same dimensions */
	PF_Pixel *p = input_p;
	int x, y, xbytes;
	for (y = 0; y < height; y++) {
	    for (x = xbytes = 0; x < width; x++, p++, xbytes += cvImg->nChannels) {
			CV_IMAGE_ELEM(cvImg, unsigned char, y, xbytes + 0) = p->blue;
			CV_IMAGE_ELEM(cvImg, unsigned char, y, xbytes + 1) = p->green;
			CV_IMAGE_ELEM(cvImg, unsigned char, y, xbytes + 2) = p->red;
	    }
		p += row - width;
	}
}

/* OpenCV -> After Effects */
static void CopyCvImgToPF(IplImage *cvImg, int width, int height, int row,
						  PF_Pixel *output_p)
{
	/* assumes input and img has the same dimensions */
	PF_Pixel *p = output_p;
	int x, y, xbytes;

	for (y = 0; y < height; y++) {
		for (x = xbytes = 0; x < width; x++, p++, xbytes += cvImg->nChannels) {
			p->blue  = CV_IMAGE_ELEM(cvImg, unsigned char, y, xbytes + 0);
			p->green = CV_IMAGE_ELEM(cvImg, unsigned char, y, xbytes + 1);
			p->red   = CV_IMAGE_ELEM(cvImg, unsigned char, y, xbytes + 2);
			p->alpha = 255;
	    }
		p += row - width;
	}
}

static inline int is_neg(int v) {
    return (((unsigned int)v) >> (sizeof(v)*8 - 1));
}

static inline int modulo(int a, int b) {
	int m = a % b;
//	return m + b * is_neg(m);

	if (m >= 0) {
		return m;
	} else {
		return m + b;
	}
}

namespace bicubic {
	static const double a =	 -1.0;

	static inline double cubic1(double x) {
		return (((a+2.0)*x - (a+3.0))*x)*x + 1.0;
	}
	static inline double cubic2(double x) {
		return ((a * x - 5.0*a) * x + 8.0*a) * x - 4.0*a;
	}

	static inline double cubic_sub(uchar *img,
							int x0, int x1, int x2, int x3,
							double w0, double w1, double w2, double w3) {
		double v0 = (double)img[x0];
		double v1 = (double)img[x1];
		double v2 = (double)img[x2];
		double v3 = (double)img[x3];
		return v0 * w0 + v1 * w1 + v2 * w2 + v3 * w3;
	}

	static inline double v_bicubic(IplImage *img, double x, double y) {
		int zx = (int)x;
		int zy = (int)y;
		double rx = x - (double)zx;
		double ry = y - (double)zy;

		if (rx < 0.0) {
			zx--;
			rx += 1.0;
		}
		if (ry < 0.0) {
			zy--;
			ry += 1.0;
		}

		double wx0 = cubic2(1.0 + rx);
		double wx1 = cubic1(      rx);
		double wx2 = cubic1(1.0 - rx);
		double wx3 = cubic2(2.0 - rx);

		double wy0 = cubic2(1.0 + ry);
		double wy1 = cubic1(      ry);
		double wy2 = cubic1(1.0 - ry);
		double wy3 = cubic2(2.0 - ry);

		int zx0 = modulo(zx-1, img->width);
		int zx1 = (zx0 + 1) % img->width;
		int zx2 = (zx0 + 2) % img->width;
		int zx3 = (zx0 + 3) % img->width;

		int zy0 = modulo(zy-1, img->height);
		int zy1 = (zy0 + 1) % img->height;
		int zy2 = (zy0 + 2) % img->height;
		int zy3 = (zy0 + 3) % img->height;

		uchar *dat = (uchar*)img->imageData;
		int widthStep = img->widthStep;

		double vy0 = cubic_sub(dat + zy0 * widthStep, zx0,zx1,zx2,zx3, wx0,wx1,wx2,wx3);
		double vy1 = cubic_sub(dat + zy1 * widthStep, zx0,zx1,zx2,zx3, wx0,wx1,wx2,wx3);
		double vy2 = cubic_sub(dat + zy2 * widthStep, zx0,zx1,zx2,zx3, wx0,wx1,wx2,wx3);
		double vy3 = cubic_sub(dat + zy3 * widthStep, zx0,zx1,zx2,zx3, wx0,wx1,wx2,wx3);

		return vy0*wy0 + vy1*wy1 + vy2*wy2 + vy3*wy3;
	}
}
using namespace bicubic;


static inline double v_bilinear(IplImage *img, double x, double y) {
	int zx = (int)x;
	int zy = (int)y;
	double rx = x - (double)zx;
	double ry = y - (double)zy;

	if (rx < 0.0) {
		zx--;
		rx += 1.0;
	}
	if (ry < 0.0) {
		zy--;
		ry += 1.0;
	}

	int zx0 = modulo(zx, img->width);
	int zy0 = modulo(zy, img->height);

	int zx1 = (zx0 + 1) % img->width;
	int zy1 = (zy0 + 1) % img->height;

	int widthStep = img->widthStep;

	double v00 = (double)(uchar)img->imageData[zy0 * widthStep + zx0];
	double v01 = (double)(uchar)img->imageData[zy0 * widthStep + zx1];
	double v10 = (double)(uchar)img->imageData[zy1 * widthStep + zx0];
	double v11 = (double)(uchar)img->imageData[zy1 * widthStep + zx1];

	return ((v00 * (1.0-rx) + v01 * rx) * (1.0-ry)) + ((v10 * (1.0-rx) + v11 * rx) * ry);
}


namespace param{
	static int X = 5;			// LineStep 大きいとガクガクに
	static int Y = 54;			// 抽出感覚　大きいと等高線は疎に
	static int Z = 0;			// LineLength	
	static int lineWidth = 1;	// 線の太さ
	static int lineNoise = 15;	// 近接ピクセルとの比較する閾値。0以上にすると線が途切れる
	static double DARK = 1.0;	// エッジの色の輝度

	static int VFlag = 0;		// ガンマ補正みたいな？
	static int hatchingType = 0;     // 0..normal 1..inverse 2..white-base
	static const int nPattern = 7;   // ハッチングパターン数
	static int drawMode = 3;
	static int drawFps = 0;
	static int patternOffset = 0;
	static IplImage *hatching_org[nPattern];
	static IplImage *hatching[nPattern];

	static int background = 0;		//背景輝度
	static bool patInv = false;	//パターン反転有無

	static double inclination = -1.0;	//傾き
	static double incRange = 1.0;		//傾き変動幅
	static int	xRange  = 10000;	//x変動幅
	static int	yRange  = 10000;	//y変動幅
	static double hDenseMax  = 0;		//ハッチング濃度上限

	static double mDARK = 1.0;	// エッジの色の輝度係数
	static double addDARK = 0.0;

	static bool hatchingDiffuse = false; //ハッチング拡散有無

	static bool hatchingIntrp = false; //ハッチングパターン参照時の補間有無

	enum INTR_TYPE{
		INTR_NONE,
		INTR_BILINEAR,
		INTR_BICUBIC
	};
	enum INTR_TYPE hatchingIntrType = INTR_NONE; //ハッチングパターン参照時の補間タイプ

	static int hatchingPtNo = -100;	//ハッチングパターンの種類

	static double hatchingMagn = 1.0;  //ハッチングパターンの拡大率
	static double hatchingRad  = 1.0;  //ハッチングパターンの回転角度(ラジアン)

	struct hatchingTransform { //ハッチングパターン変換用係数
		double a00;
		double a01;
		double a02;
		double a10;
		double a11;
		double a12;
	};
	static hatchingTransform hatchingTf;

	static double s_coef = 2.0;				// 彩度係数
	static double v_coef = 1.0;				// 明度係数

	static bool makeMask = false;			// マスク作成モードか

	static CvRNG rng_state;

	enum LINE_TYPE{
		LINE_NORMAL,
		LINE_CANNY_GRAY,
		LINE_CANNY_RGB,
		LINE_CANNY_GRAY_RGB
	};
	static int lineType = LINE_NORMAL;
	static int cannyThr1 = 2000;
	static int cannyThr2 = 3000;
}
using namespace param;

static IplImage *img0 = NULL;
static IplImage *img1 = NULL;
static IplImage *hsv = NULL;  
static IplImage *grayImage = NULL;
static IplImage *binaryImage = NULL;
static IplImage *imgB = NULL;
static IplImage *imgG = NULL;
static IplImage *imgR = NULL;
static IplImage *binaryImageWk = NULL;

typedef struct PointTable{
	CvPoint p;
	CvScalar color;
} POINTTABLE;

static const int MAXPOINT = 100000;
static POINTTABLE pointTable[MAXPOINT];

// RGB差の合計
double colorLen(const CvScalar &x, const CvScalar &y)
{
	double sum = 0;
	for(int i=0;i<3;i++){
		sum += abs(x.val[i] - y.val[i]);
	}
	return sum;
}

// imageの座標位置カラー
CvScalar getColor(IplImage* image, int x, int y)
{
	int index = y * image->widthStep + x * image->nChannels;
	uchar *data =  (uchar *)image->imageData;
	return CV_RGB(data[index+2],data[index+1],data[index+0]);
}

// 座標がイメージ外にある可能性のある場合用
CvScalar getColorCheck(IplImage* image, int x, int y)
{
	if (x<0) x = 0;
	if (y<0) y = 0;
	if (x>=image->width) x = image->width-1;
	if (y>=image->height) y = image->height-1;
	return getColor(image, x, y);
}

void divScaler(CvScalar &s, double d)
{
	s.val[0] /= d;
	s.val[1] /= d;
	s.val[2] /= d;
	s.val[3] /= d;
}

void mulScaler(CvScalar &s, double d)
{
	s.val[0] *= d;
	s.val[1] *= d;
	s.val[2] *= d;
	s.val[3] *= d;
}

void muladdScaler(CvScalar &s, double d, double a)
{
	s.val[0] *= d;
	s.val[1] *= d;
	s.val[2] *= d;
	s.val[3] *= d;
	s.val[0] += a;
	s.val[1] += a;
	s.val[2] += a;
	s.val[3] += a;
}


//近接ピクセルとの色の差の最大値と閾値を比較

int checkPoint(IplImage *image, int x, int y, double th)
{
	static struct { int x,y; } map[4] = { { 1,0 }, {-1,0}, { 0,1}, { 0,-1} };
	CvScalar color = getColor(image, x,y);
	for(int i = 0;i<4; i++){
		if (colorLen(color, getColorCheck(image, x+map[i].x, y + map[i].y)) > th) return 1;
	}
	return 0;
}
int setPoint(int index, CvPoint *point, CvScalar &color)
{
	if (index >= MAXPOINT) return 0;
	pointTable[index].p = *point;
	pointTable[index].color.val[0] = color.val[0];
	pointTable[index].color.val[1] = color.val[1];
	pointTable[index].color.val[2] = color.val[2];
	pointTable[index].color.val[3] = color.val[3];
	return 1;		
}
double clip(double x, double min, double max)
{
	if (x>max) return max;
	if (x<min) return min;
	return x;
}

void drawNarrowingLine(IplImage* dst, CvPoint pt1, CvPoint pt2, CvScalar col, int width) {
	CvPoint pts[4];
	int bit = 3;
	int m = 1 << bit;

	double rad = atan2((double)(pt2.y-pt1.y), (double)(pt2.x-pt1.x)) + M_PI_2;
	double x0 = pt1.x + cos(rad) * width * 0.5;
	double y0 = pt1.y + sin(rad) * width * 0.5;
	double x1 = pt1.x - cos(rad) * width * 0.5;
	double y1 = pt1.y - sin(rad) * width * 0.5;
	double ewm = 0.2;
	double x2 = pt2.x - cos(rad) * width * 0.5 * ewm;
	double y2 = pt2.y - sin(rad) * width * 0.5 * ewm;
	double x3 = pt2.x + cos(rad) * width * 0.5 * ewm;
	double y3 = pt2.y + sin(rad) * width * 0.5 * ewm;

	pts[0].x = cvRound(x0 * m);
	pts[0].y = cvRound(y0 * m);
	pts[1].x = cvRound(x1 * m);
	pts[1].y = cvRound(y1 * m);
	pts[2].x = cvRound(x2 * m);
	pts[2].y = cvRound(y2 * m);
	pts[3].x = cvRound(x3 * m);
	pts[3].y = cvRound(y3 * m);

	cvFillConvexPoly(dst, pts, 4, col, CV_AA, bit);



	/*
	for(int i=0; i<width; i++) {
		CvPoint sp = cvPoint(pt1.x + cvRound((pt2.x-pt1.x)* i / (double)width), 
							 pt1.y + cvRound((pt2.y-pt1.y)* i / (double)width)); 
		CvPoint ep = cvPoint(pt1.x + cvRound((pt2.x-pt1.x)* (i+1) / (double)width), 
							 pt1.y + cvRound((pt2.y-pt1.y)* (i+1) / (double)width));
		cvLine(dst, sp, ep, col, i, CV_AA);
	}
	*/
}

// inputImageから線画を作り、baseに合成する plane 0,1,2:H,S,V 3:GrayScaleを線画の種に
void drawEdge(IplImage* image,IplImage* base,int plane = 2)
{
	static CvMemStorage *memStorage0 =cvCreateMemStorage(0); 
	CvSeq *contourSeq0 = NULL;

	int height    = image->height;
	int width     = image->width;
	int step      = image->widthStep;
	int channels  = image->nChannels;
	uchar *data      = (uchar *)image->imageData;


	if (plane <3){
		cvCvtColor(image,hsv,CV_BGR2HSV);				// HSVのplaneを線画生成の元にする
		for(int i=0;i<height*width;++i) grayImage->imageData[i] = hsv->imageData[i*3+plane];
	}else{
		cvCvtColor (image, grayImage, CV_BGR2GRAY);		// グレーイメージを作り線画生成の元にする
	}

	IplImage *target=base;					// 書き込むターゲットイメージ


	for(int x=20;x<240;x+=Y){
		switch (lineType) {
			case LINE_NORMAL:
				cvThreshold( grayImage, binaryImage, x, 255, CV_THRESH_BINARY );	// x の値を境に２値化し輪郭を抽出
				break;
			
			case LINE_CANNY_GRAY:
				cvCanny(grayImage, binaryImage, cannyThr1, cannyThr2, 5);	//Cannyアルゴリズムで輪郭抽出
				break;

			case LINE_CANNY_RGB:
				cvSplit(image, imgB, imgG, imgR, NULL);
			
				cvCanny(imgB, binaryImage,   cannyThr1, cannyThr2, 5);	//Cannyアルゴリズムで輪郭抽出
				cvCanny(imgG, binaryImageWk, cannyThr1, cannyThr2, 5);	//Cannyアルゴリズムで輪郭抽出
				cvOr(binaryImage, binaryImageWk, binaryImage);
				cvCanny(imgR, binaryImageWk, cannyThr1, cannyThr2, 5);	//Cannyアルゴリズムで輪郭抽出
				cvOr(binaryImage, binaryImageWk, binaryImage);
				break;

			case LINE_CANNY_GRAY_RGB:
				cvSplit(image, imgB, imgG, imgR, NULL);
			
				cvCanny(imgB, binaryImage,   cannyThr1, cannyThr2, 5);	//Cannyアルゴリズムで輪郭抽出
				cvCanny(imgG, binaryImageWk, cannyThr1, cannyThr2, 5);	//Cannyアルゴリズムで輪郭抽出
				cvOr(binaryImage, binaryImageWk, binaryImage);
				cvCanny(imgR, binaryImageWk, cannyThr1, cannyThr2, 5);	//Cannyアルゴリズムで輪郭抽出
				cvOr(binaryImage, binaryImageWk, binaryImage);
				cvCanny(grayImage, binaryImageWk, cannyThr1, cannyThr2, 5);	//Cannyアルゴリズムで輪郭抽出
				cvOr(binaryImage, binaryImageWk, binaryImage);
				break;
		}
		//cvNamedWindow("binaryImage");
		//cvShowImage("binaryImage", binaryImage);

		contourSeq0 = 0;
		cvFindContours(binaryImage, memStorage0, &contourSeq0,sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cvPoint(0,0)); // 輪郭線探索
		
		if (lineNoise>0){ // 不連続ラインの場合
			for(; contourSeq0 != 0; contourSeq0 = contourSeq0->h_next) 
			{
				CvPoint *p;
				if (contourSeq0->total<X*5) continue;		// 5角形以下の細かいのは排除
				int index = 0;
				for( int i=0; i<contourSeq0->total; i+=X )
				{
					p = CV_GET_SEQ_ELEM(CvPoint, contourSeq0, i);				// 点の場所と色を登録
					CvScalar color = getColor(image,p->x, p->y);
					muladdScaler(color,DARK,addDARK);										// 輝度を修正
					color.val[3] = checkPoint(image, p->x, p->y, lineNoise);	// 有効点かどうかを近接ピクセルから判断して[3]へ格納
					setPoint(index,p,color);									// pointTableへ保存
					index++;
					if (index > MAXPOINT){
						printf("INDEX ERROR\n"); 
						index = 0;
					}
				}
				// ５連続以下の有効点は無効 （Pending:高速化）
				for(int i=0;i<index;i++){
					int p1 = i;
					int p2,p3,p4,p0;
					if (pointTable[p1].color.val[3]){
						p2 = (p1+1)%index;
						p3 = (p1+2)%index;
						p4 = (p1+3)%index;
						p0 = (p1-1+index)%index;
						if (pointTable[p0].color.val[3]) continue;
						if (!pointTable[p2].color.val[3] ||
							!pointTable[p3].color.val[3] ||
							!pointTable[p4].color.val[3] ){						
							pointTable[p1].color.val[3] = 0;
						}
					}
				}
				// 接続された有効点を描く
				for(int i=0;i<index;i++){
					int p1 = i;
					int p2 = (i+1)%index;// if (p2==index) p2 = 0;
					if (pointTable[p1].color.val[3] && pointTable[p2].color.val[3])
					{
						CvScalar c;
						if (!makeMask) {
							c = pointTable[p1].color;
							muladdScaler(c,DARK,addDARK);
						} else {
							c.val[0] = c.val[1] = c.val[2] = 255;
						}
						if (lineWidth >= 0) {
							cvLine(target,pointTable[p1].p,pointTable[p2].p,c,lineWidth,CV_AA );
						} else {
							drawNarrowingLine(target,pointTable[p1].p,pointTable[p2].p,c, -lineWidth);
						}
					}
				}
			}
		}else{
			// 全部描く場合
			for(; contourSeq0 != 0; contourSeq0 = contourSeq0->h_next) 
			{

				CvPoint *p1 = 0;
				CvPoint *p2;

				if (contourSeq0->total<X*5) continue;		

				for(int i=0; i<contourSeq0->total; i+=X ){
					p1 = CV_GET_SEQ_ELEM(CvPoint, contourSeq0, (i)%contourSeq0->total);//始点
					p2 = CV_GET_SEQ_ELEM(CvPoint, contourSeq0, (i+X+Z)%contourSeq0->total);// 終点
					CvScalar color;
					if (!makeMask) {
						color = getColor(image,p1->x, p1->y);
						mulScaler(color,DARK);
					} else {
						color.val[0] = color.val[1] = color.val[2] = 255;
					}

					if (lineWidth >= 0) {
						cvLine(target,*p1,*p2,color,lineWidth,CV_AA );
					} else {
						drawNarrowingLine(target,*p1,*p2, color, -lineWidth);
					}
				}
			}
		}

		if (lineType != LINE_NORMAL) {	//Cannyの場合は１回のみ実行
			break;
		}
	}
	cvClearMemStorage(memStorage0);
}

// Hatchingエフェクト
int drawHatching(IplImage *frame, IplImage *base)
{
	int height    = frame->height;
	int width     = frame->width;
	int fnChannels = frame->nChannels;
	int fwidthStep = frame->widthStep;

	cvCvtColor(frame,hsv,CV_BGR2HSV);

	if (hatchingType == 5) {
		for(int y=0;y<height;++y){
			int index_y = y * fwidthStep;

			for(int x=0;x<width;++x){
				int index = index_y + x*fnChannels;
				hsv->imageData[index+2] = (uchar)(Saturate((uchar)hsv->imageData[index+2] * v_coef, 0.0, 255.0)) ;
			}
		}
		cvCvtColor(hsv, base, CV_HSV2BGR);	
		cvCvtColor(base, grayImage, CV_BGR2GRAY);	
	}

	double rx = cvRandReal(&rng_state) * xRange;// texture offset
	double ry = cvRandReal(&rng_state) * yRange;
	double d =  (cvRandReal(&rng_state)-0.5)*incRange + inclination;//傾き


	double a00 = hatchingTf.a00;
	double a01 = hatchingTf.a01;
	double a02 = hatchingTf.a02;
	double a10 = hatchingTf.a10;
	double a11 = hatchingTf.a11;
	double a12 = hatchingTf.a12;


	int vtbl[256];
	for (int i=0; i<256; i++) {
		double V = (VFlag >= 0 ? (double)i : (255+VFlag)*i/255.0) / 255.0;
		V = pow(V, (10-VFlag)/10.0)*255.0;
		vtbl[i] = (int)V;
	}


	for(int y=0;y<height;++y){
		int index_y = y * fwidthStep;

		for(int x=0;x<width;++x){
			int index = index_y + x*fnChannels;
			uchar v = hsv->imageData[index+2];
			v = vtbl[v];


			double X = x + rx;
			double Y = y + ry + (x*d);

			if (hatchingDiffuse) {
				X += cvRandReal(&rng_state)*2.0 - 1.0;
				Y += cvRandReal(&rng_state)*2.0 - 1.0;
			}

			double typeV;

			int maxV = nPattern-1+patternOffset;
			
			typeV = (255-v) / (256/maxV)-patternOffset;
			typeV = maxV-typeV ;

			if (typeV<0) typeV = 0;
			if (typeV>nPattern-1) typeV = nPattern-1;

			double vf = 0.0;
			int a = (int)typeV;
			int b = a+1;
			double w = typeV-a; // weight

			if (patInv){ // Inverse
				a = nPattern-1-a;
				b = nPattern-1-b;
			}

			double dx = a00 * X + a01 * Y + a02;
			double dy = a10 * X + a11 * Y + a12;

			switch (hatchingIntrType) {
				case INTR_NONE: {
					if (dx < 0.0) dx--;
					if (dy < 0.0) dy--;

					int aX = modulo((int)(dx+0.5), hatching[a]->width);
					int aY = modulo((int)(dy+0.5), hatching[a]->height);
					vf += ((uchar)hatching[a]->imageData[aX + aY * hatching[a]->widthStep]) * (1.0-w);
					if (b >=0 && b<=6){
						int bX = modulo((int)(dx+0.5), hatching[b]->width);
						int bY = modulo((int)(dy+0.5), hatching[b]->height);
						vf += ((uchar)hatching[b]->imageData[bX + bY * hatching[b]->widthStep]) * w;

					}
					break;
				}

				case INTR_BILINEAR:
					vf += v_bilinear(hatching[a], dx, dy)  * (1.0-w);
					if (b >=0 && b<=6){
						vf += v_bilinear(hatching[b], dx, dy) * w;
					}
					break;

				case INTR_BICUBIC:
					vf += v_bicubic(hatching[a], dx, dy) * (1.0-w);
					if (b >=0 && b<=6){
						vf += v_bicubic(hatching[b], dx, dy) * w;
					}
					vf = Saturate(vf, 0.0, 255.0);
					break;
			}

			double sf = s_coef;

			double vv = clip((uchar)hsv->imageData[index+2] * v_coef, 0.0, 255.0);
			if (hatchingType == 2){
				//int V = vv;V*=2;if (V>255)V=255;vv = V;// ベースを明るくしてみたり
				//vv = (uchar)clip(vv,0.0,255.0);
				vf=(vf*0.7+255.0*0.3); // 線も黒すぎるのでちょっと明るく
			}
			else if (hatchingType == 3){ // 白黒
				vv = 255.0;
				sf = 0.0;
				//vf=vf*0.7+255.0*0.3; // 線も黒すぎるのでちょっと明るく
			}
			
			uchar newv;
			double vf2;

			switch (hatchingType) {
				case 0:
				case 1:
					vf2 = 1.0 - clip(vf*(1.0/255.0), 0.0, hDenseMax);
					newv = (uchar)(vv * vf2) ;
					break;

				case 4:
					vf2 = clip(vf*(1.0/255.0), 1.0-hDenseMax, 1.0);
					sf *= (1-vf2);
					newv = (uchar)clip(vv + (255.0-vv)*vf2, 0.0, 255.0);
					break;

				case 2:
				case 3:
					vf2 = clip(vf*(1.0/255.0), 1.0-hDenseMax, 1.0);
					newv = (uchar)(vv * vf2) ;
					break;
				case 5:
					vf2 = clip(vf*(1.0/255.0), 0.0, 1.0);
					break;
			}

			if (!makeMask) {
				switch (hatchingType) {
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
						base->imageData[index+0] = hsv->imageData[index+0];
						base->imageData[index+1] = (uchar)(Saturate((uchar)hsv->imageData[index+1] * sf, 0.0, 255.0));
						base->imageData[index+2] = newv ;
						break;
					case 5:
						double hf = (1.0 - vf2) * hDenseMax;
						base->imageData[index+0] = (uchar)(((int)(uchar)hsv->imageData[index+0] + (int)(hf*180) + 360) % 180);
						base->imageData[index+1] = (uchar)(Saturate((uchar)hsv->imageData[index+1] * s_coef, 0.0, 255.0));
						base->imageData[index+2] = (uchar)(Saturate((uchar)hsv->imageData[index+2] * v_coef, 0.0, 255.0)) ;

						uchar cy = CV_IMAGE_ELEM(grayImage, uchar, y, x);
						CV_IMAGE_ELEM(grayImage, uchar, y, x) = (uchar)(Saturate(cy-8*(1.0-vf2), 0.0, 255.0));
						break;
				}
			} else {
				base->imageData[index+0] = 0;
				base->imageData[index+1] = 0;
				if (hatchingType == 2 || hatchingType == 3 || hatchingType == 4) {
					base->imageData[index+2] = 255 - (uchar)(255.0 * vf2);
				} else {
					base->imageData[index+2] = (uchar)(255.0 * vf2);
				}
			}
		}
	}
	cvCvtColor(base,base,CV_HSV2BGR);

	if (hatchingType == 5) {
		IplImage *ycc = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);
		cvCvtColor(base, ycc, CV_BGR2YCrCb);

		for(int y=0;y<height;++y){
			int index_y = y * fwidthStep;

			for(int x=0;x<width;++x){
				int index = index_y + x*fnChannels;
				ycc->imageData[index+0] = CV_IMAGE_ELEM(grayImage, uchar, y, x);
			}
		}

		cvCvtColor(ycc, base, CV_YCrCb2BGR);

		cvReleaseImage(&ycc);
	}

	return 1;
}


static
void CopyImgWithMargin(
		IplImage *src, IplImage *dst, int margin
){
	int srcw = src->width;
	int srch = src->height;
	int srcws = src->widthStep;

	int dstw = dst->width;
	int dsth = dst->height;
	int dstws = dst->widthStep;

	/* byte per pixel */
	int bpp = (src->depth / 8) * src->nChannels;

	for (int dsty = 0; dsty < dsth; dsty++) {
		int srcy = modulo(dsty - margin, srch);
		char *psrcy = src->imageData + (srcws * srcy);
		char *pdsty = dst->imageData + (dstws * dsty);

		for (int dstx = 0; dstx < dstw;) {
			if (margin <= dstx && dstx < srcw + margin) {
				char *psrc = psrcy;
				char *pdst = pdsty + margin * bpp;
				memcpy(pdst, psrc, bpp*srcw);
				dstx += srcw;
			} else {
				int srcx = modulo(dstx - margin, srcw);
				char *psrc = psrcy + srcx * bpp;
				char *pdst = pdsty + dstx * bpp;
				memcpy(pdst, psrc, bpp);
				dstx++;
			}
		}
	}
}

static
void AffineTransform(
		IplImage *src, IplImage *dst, CvMat *mat
){
	int srcw = src->width;
	int srch = src->height;
	int srcws = src->widthStep;

	int dstw = dst->width;
	int dsth = dst->height;
	int dstws = dst->widthStep;

	/* byte per pixel */
	int bpp = (src->depth / 8) * src->nChannels;

	double a00 = cvGetReal2D(mat, 0, 0);
	double a01 = cvGetReal2D(mat, 0, 1);
	double a02 = cvGetReal2D(mat, 0, 2);
	double a10 = cvGetReal2D(mat, 1, 0);
	double a11 = cvGetReal2D(mat, 1, 1);
	double a12 = cvGetReal2D(mat, 1, 2);

	for (int dsty = 0; dsty < dsth; dsty++) {
		char *pdsty = dst->imageData + (dstws * dsty);

		for (int dstx = 0; dstx < dstw; dstx++) {
			double x = a00 * dstx + a01 * dsty + a02;
			double y = a10 * dstx + a11 * dsty + a12;

			int xx = modulo((int)x, srcw);
			int yy = modulo((int)y, srch);

			char *psrc = src->imageData + (srcws * yy + bpp * xx);
			char *pdst = pdsty + dstx * bpp;
			memcpy(pdst, psrc, bpp);
		}
	}
}



static char g_resource_path[PATH_MAX];
#if 0 /* AviUtil */
static char *g_dllpath = NULL;
static std::map<std::string, int> g_trackMap;
static std::map<std::string, int> g_checkMap;
static std::vector<int> g_bef_check;
#endif /* AviUtil */

static void ReleaseImages() {
	cvReleaseImage(&img0);
	cvReleaseImage(&img1);
	cvReleaseImage(&hsv);
	cvReleaseImage(&grayImage);
	cvReleaseImage(&binaryImage);

	cvReleaseImage(&imgB);
	cvReleaseImage(&imgG);
	cvReleaseImage(&imgR);
	cvReleaseImage(&binaryImageWk);	
}

static void CreateImages(CvSize size) {
	img0		= cvCreateImage(size, IPL_DEPTH_8U, 3);
	img1		= cvCreateImage(size, IPL_DEPTH_8U, 3);
	hsv			= cvCreateImage(size, IPL_DEPTH_8U, 3);
	grayImage	= cvCreateImage(size, IPL_DEPTH_8U, 1);
	binaryImage	= cvCreateImage(size, IPL_DEPTH_8U, 1);

	imgB = cvCreateImage(size, IPL_DEPTH_8U, 1);
	imgG = cvCreateImage(size, IPL_DEPTH_8U, 1);
	imgR = cvCreateImage(size, IPL_DEPTH_8U, 1);
	binaryImageWk = cvCreateImage(size, IPL_DEPTH_8U, 1);
}

static void ReleaseHatchingPattern() {
	for(int i=0;i<nPattern;i++){
		if (hatching_org[i] != NULL) {
			cvReleaseImage(hatching_org + i);
			hatching_org[i] = NULL;
		}
		if (hatching[i] != NULL) {
			cvReleaseImage(hatching + i);
			hatching[i] = NULL;
		}
	}
}

static bool LoadHatchingPattern(char *errmsg) {
	char dir[PATH_MAX];

	snprintf(dir, sizeof(dir), "%s/images", g_resource_path);

	for(int i=0;i<nPattern;i++){
		char buf[PATH_MAX];
		if (hatchingPtNo == 0) {
			snprintf(buf, sizeof(buf), "%s/img/h%d.bmp", dir, i);
		} else {
			snprintf(buf, sizeof(buf), "%s/img%d/h%d.bmp", dir, hatchingPtNo,  i);
		}

		if (hatching_org[i] != NULL) {
			cvReleaseImage(hatching_org + i);
		}

		hatching_org[i] = cvLoadImage(buf, CV_LOAD_IMAGE_GRAYSCALE);

		if (hatching_org[i] == NULL) {
			ReleaseHatchingPattern();
			snprintf(errmsg, 8192, "%s の読み込みに失敗しました", buf);
			return false;
		}
	}
	return true;
}

static void InitHatchingPattern(PF_EffectWorld *input) {
	double w = input->width;
	double h = input->height;

	double hw = hatching_org[0]->width;
	double hh = hatching_org[0]->height;

	double par1[] = {
		1.0, 0.0, -hw/2.0,
		0.0, 1.0, -hh/2.0,
		0.0, 0.0, 1.0
	};

	double par2[] = {
		1.0, 0.0, w/2.0,
		0.0, 1.0, h/2.0,
		0.0, 0.0, 1.0
	};

	double rot[] = {
		cos(hatchingRad), -sin(hatchingRad), 0.0,
		sin(hatchingRad),  cos(hatchingRad), 0.0,
		0.0, 0.0, 1.0
	};


	double exp[] = {
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0
	};
	if (hatchingMagn >= 1.0) {
		exp[0] = hatchingMagn;
		exp[4] = hatchingMagn;
	}

	double wk1[9];
	double wk2[9];
	double res[9];
	double inv[9];

	CvMat mpar1,mpar2,mrot,mexp;
	CvMat mwk1,mwk2,mres,minv;

	cvInitMatHeader(&mpar1, 3, 3, CV_64FC1, par1);
	cvInitMatHeader(&mpar2, 3, 3, CV_64FC1, par2);
	cvInitMatHeader(&mrot, 3, 3, CV_64FC1, rot);
	cvInitMatHeader(&mexp, 3, 3, CV_64FC1, exp);

	cvInitMatHeader(&mwk1, 3, 3, CV_64FC1, wk1);
	cvInitMatHeader(&mwk2, 3, 3, CV_64FC1, wk2);
	cvInitMatHeader(&mres, 3, 3, CV_64FC1, res);
	cvInitMatHeader(&minv, 3, 3, CV_64FC1, inv);


	cvMatMul(&mrot, &mpar1, &mwk1);
	cvMatMul(&mexp, &mwk1, &mwk2);
	cvMatMul(&mpar2, &mwk2, &mres);
	cvInvert(&mres, &minv, CV_LU);

	hatchingTf.a00 = inv[0];
	hatchingTf.a01 = inv[1];
	hatchingTf.a02 = inv[2];
	hatchingTf.a10 = inv[3];
	hatchingTf.a11 = inv[4];
	hatchingTf.a12 = inv[5];

	for(int i=0;i<nPattern;i++){
		if (hatching[i] != NULL) {
			cvReleaseImage(hatching + i);
		}

		if (hatchingMagn >= 1.0) {
			CvSize size = cvSize(hatching_org[i]->width, hatching_org[i]->height);
			hatching[i] = cvCreateImage(size, hatching_org[i]->depth, hatching_org[i]->nChannels);
			cvCopy(hatching_org[i], hatching[i]);

		} else {	
			CvSize size = cvSize(Saturate((int)(hatching_org[i]->width * hatchingMagn), 1, 2048), 
								 Saturate((int)(hatching_org[i]->height * hatchingMagn), 1, 2048));
			hatching[i] = cvCreateImage(size, hatching_org[i]->depth, hatching_org[i]->nChannels);
			cvResize(hatching_org[i], hatching[i], CV_INTER_AREA);
		}
	}
}

PF_Err
UpdateHatchingPattern (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output)
{
	PF_Err				err		= PF_Err_NONE;
	PF_EffectWorld *input = &params[0]->u.ld;

	bool no_changed = false;
#if 0 /* TODO: AviUtil params */
	int no = 0;
	double magn = 1.0;
	double rad  = 0;
#else
	int no = params[PARAMID_PATTERNNO]->u.pd.value-1; /* 0 origin */
	double magn = ((double)params[PARAMID_MAGNITUDE]->u.fs_d.value);
	double rad  = ((double)params[PARAMID_ANGLE]->u.ad.value) * M_PI / 180.0;
#endif

	if (no != hatchingPtNo) {
		hatchingPtNo = no;
		no_changed = true;

		char errmsg[8192];
		bool ret;
		ret = LoadHatchingPattern(errmsg);
		if (!ret) {
			PF_SPRINTF(out_data->return_msg,
					   "UpdateHatchingPattern failed: %s", errmsg);
			out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
			return PF_Err_INTERNAL_STRUCT_DAMAGED;
		}
	}

	if (hatching_org[0] == NULL) {
		PF_SPRINTF(out_data->return_msg,
				   "UpdateHatchingPattern failed: hatching_org[0] = NULL");
		out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
		return PF_Err_INTERNAL_STRUCT_DAMAGED;
	}
	
	if (no_changed || 
		magn != hatchingMagn ||
		rad != hatchingRad
		) {

		hatchingMagn = magn;
		hatchingRad = rad;

		InitHatchingPattern(input);
	}

	return err;
}

void InitAnimEffect(char *res_path) {
	snprintf(g_resource_path, sizeof(g_resource_path), "%s", res_path);
	hatchingPtNo = -100;
}

void ExitAnimEffect() {
	ReleaseImages();
	ReleaseHatchingPattern();
}


PF_Err 
RenderAnimEffect (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err				err		= PF_Err_NONE;

	PF_EffectWorld *input = &params[0]->u.ld;
	int w = input->width;
	int h = input->height;
	int mw = input->rowbytes / sizeof(PF_Pixel);

#if 0 /* TODO: AviUtil params */
	int hatchingMode = 1;
	int freq = 1; /* fp->track[g_trackMap["更新間隔"]];*/
	int rseed = 0; /* fp->track[g_trackMap["乱数ｼｰﾄﾞ"]];*/
	int lineOn		= 1; /* fp->check[g_checkMap["ライン"]];*/
	int hatchingOn	= 1; /* fp->check[g_checkMap["ハッチング"]];*/
#else /* AviUtil */
	int hatchingMode = params[PARAMID_MODE]->u.pd.value; /* 1 origin */
	param::X = params[PARAMID_LINESTEP]->u.fs_d.value;
	param::Y = params[PARAMID_LINEINTERVAL]->u.fs_d.value;
	param::Z = params[PARAMID_LINELENGTH]->u.fs_d.value;
	lineWidth = params[PARAMID_LINEWIDTH]->u.fs_d.value;
	lineNoise = params[PARAMID_LINENOISE]->u.fs_d.value;
	VFlag = params[PARAMID_VFLAG]->u.fs_d.value;
	mDARK = ((double)params[PARAMID_MDARK]->u.fs_d.value);
	inclination = ((double)params[PARAMID_INCLINATION]->u.fs_d.value);
	hDenseMax = ((double)params[PARAMID_DENSEMAX]->u.fs_d.value);
	int freq = params[PARAMID_FREQUENCY]->u.fs_d.value;
	int rseed = params[PARAMID_RSEED]->u.fs_d.value;
	incRange = ((double)params[PARAMID_INCRANGE]->u.fs_d.value);
	xRange = params[PARAMID_XRANGE]->u.fs_d.value;
	yRange = params[PARAMID_YRANGE]->u.fs_d.value;

	s_coef = ((double)params[PARAMID_SATURATION]->u.fs_d.value);
	v_coef = ((double)params[PARAMID_BRIGHTNESS]->u.fs_d.value);

	lineType = params[PARAMID_LINETYPE]->u.pd.value-1; /* 0 origin */
	cannyThr1 = params[PARAMID_CANNYTH1]->u.fs_d.value;
	cannyThr2 = params[PARAMID_CANNYTH2]->u.fs_d.value;


	int lineOn		= params[PARAMID_LINE]->u.bd.value;
	int hatchingOn	= params[PARAMID_HATCHING]->u.bd.value;
	hatchingDiffuse = (params[PARAMID_DIFFUSE]->u.bd.value == TRUE);
	hatchingIntrType = (enum INTR_TYPE)(params[PARAMID_INTERPOLATION]->u.pd.value-1); /* 0 origin */
	makeMask = (params[PARAMID_MAKEMASK]->u.bd.value == TRUE);
#endif /* AviUtils */


	err = UpdateHatchingPattern(in_data, out_data, params, output);
	if (err != PF_Err_NONE) {
		return err;
	}

	switch(hatchingMode){
		case 1: 
			hatchingType = 0;
			DARK=1.0;
			patternOffset=0;
			background=000;
			patInv=false;
			break;

		case 2:
			hatchingType = 1;
			DARK=1.0;
			patternOffset=0;
			background=000;
			patInv=true;
			break;

		case 3:
			hatchingType = 2;
			DARK=0.7;
			patternOffset=1;
			background= makeMask ? 0 : 255;
			patInv=true;
			break; // 特別反転版　支離滅裂

		case 4:
			hatchingType = 3;
			DARK=0.0;
			patternOffset=1;
			background= makeMask ? 0 : 255;
			patInv=true;
			break; // 白黒版

		case 5: 
			hatchingType = 4;
			DARK=1.0;
			patternOffset=0;
			background = makeMask ? 0 : 255;
			patInv=true;
			break; // 1の背景白版

		case 6: 
			hatchingType = 5;
			DARK=1.0;
			patternOffset=0;
			background = makeMask ? 0 : 255;
			patInv=true;
			break; // 色相
	}

	patInv ^= (params[PARAMID_INVERT]->u.bd.value == TRUE);

	if (hatchingMode != 4) {
		DARK *= mDARK;
		addDARK = 0;
	} else {
		addDARK = 255.0*(1-mDARK);
	}

	if (hatching[0] == NULL) {
		//InitHatchingPattern();
		PF_SPRINTF(out_data->return_msg, "Initialize failed for Hatching patterns.");
		out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
		return PF_Err_INTERNAL_STRUCT_DAMAGED;
	}


	if (img0 == NULL) {
		ReleaseImages();
		CreateImages(cvSize(w,h));
	} else {
		CvSize size = cvGetSize(img0);
		if (size.width != w || size.height != h) {
			ReleaseImages();
			CreateImages(cvSize(w,h));
		}
	}

#if 1 /* AviUtil */
	if (freq < 0) {
		/* TODO: not implemented yet */
	}
	/* After Effects -> OpenCV */
	/* TODO: error handling */
	PF_Pixel *input_p = NULL;
	err = PF_GET_PIXEL_DATA8(input, NULL, &input_p);
	CopyPFToCvImg(input_p, input->width, input->height,
				  (input->rowbytes / sizeof(PF_Pixel)), 
				  img0);
	
#else /* AviUtil */
	//AviUtl → OpenCV
	PIXEL_YC *org_buf = buffer0;
	if (freq < 0) {
		fp->exfunc->set_ycp_filtering_cache_size(fp, mw, mh, 5, NULL);
		int afreq = abs(freq);
		int frame = (fpip->frame / afreq) * afreq;
		org_buf = (PIXEL_YC *)fp->exfunc->get_ycp_filtering_cache_ex(fp, fpip->editp, frame, NULL, NULL);
	}
	CopyBufToCvBGRImg(org_buf, img0, 0,0,w,h, 0,0, mw,mh);
#endif /* AviUtil */

	//hatching
	if (hatchingOn){
		rng_state = cvRNG(((int64)rseed) << 32);
#if 0 /* TODO: freq not implemented */
		if (freq == 0) {
			rng_state = cvRNG(((int64)rseed) << 32);
		} else {
			int afreq = abs(freq);
			rng_state = cvRNG((((int64)rseed) << 32) + fpip->frame / afreq);
		}
#endif /* TODO: freq not implemented */
		drawHatching(img0, img1);
	}else{
		cvSet(img1, cvScalarAll(background),0);
	}

	//line
	if (lineOn){
		/*
		IplImage *linesrc = NULL;
		if (freq >= 0) {
			linesrc = img0;
		} else {
			fp->exfunc->set_ycp_filtering_cache_size(fp, mw, mh, 5, NULL);
			int afreq = abs(freq);
			int frame = (fpip->frame / afreq) * afreq;
			PIXEL_YC *lsrc = (PIXEL_YC *)fp->exfunc->get_ycp_filtering_cache_ex(fp, fpip->editp, frame, NULL, NULL);
			linesrc = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 3);
			CopyBufToCvBGRImg(lsrc, linesrc, 0,0,w,h, 0,0, mw,mh);
		}

		drawEdge(linesrc,img1,2);

		if (freq < 0) {
			cvReleaseImage(&linesrc);
		}
		*/

		drawEdge(img0, img1, 2);
	}

	/* OpenCV -> After Effects */
	/* TODO: error handling */
	PF_Pixel *output_p = NULL;
	err = PF_GET_PIXEL_DATA8(output, NULL, &output_p);
	CopyCvImgToPF(img1, img1->width, img1->height,
				  (output->rowbytes / sizeof(PF_Pixel)),
				   output_p);

	return err;
}
