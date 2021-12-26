
#include "fir_scaler.h"

enum _DF_VIDEO_SCALER_CFG_
{
	//Scaler Coeff Cfg
	DF_SCL_STEP_FRACTION_BIT_WIDTH  = 13,
	
	DF_SCL_COEFF_FRACTION_BIT_WIDTH = 10,
	DF_SCL_COEFF_INTREGER_BIT_WIDTH = 1,
	DF_SCL_COEFF_POLARITY_BIT       = 15,

	DF_SCL_WIDTH_MAX = 2048,
	DF_SCL_FIR_TAP_MAX  = 32,
	
	//Video Scaler Cfg
	DF_VIDEO_SCL_LUMA_VFIR_TAP_NUM          = 4,
	DF_VIDEO_SCL_LUMA_VFIR_PHASE_LOG2BITS   = 4,
	DF_VIDEO_SCL_LUMA_VFIR_PHASE_NUM        = 1 << DF_VIDEO_SCL_LUMA_VFIR_PHASE_LOG2BITS,
	DF_VIDEO_SCL_LUMA_HFIR_TAP_NUM          = 4,	
	DF_VIDEO_SCL_LUMA_HFIR_PHASE_LOG2BITS   = 4,
	DF_VIDEO_SCL_LUMA_HFIR_PHASE_NUM        = 1 << DF_VIDEO_SCL_LUMA_HFIR_PHASE_LOG2BITS,
	
	DF_VIDEO_SCL_CHROMA_VFIR_TAP_NUM        = 2,
	DF_VIDEO_SCL_CHROMA_VFIR_PHASE_LOG2BITS = 4,
	DF_VIDEO_SCL_CHROMA_VFIR_PHASE_NUM      = 1 << DF_VIDEO_SCL_CHROMA_VFIR_PHASE_LOG2BITS,
	DF_VIDEO_SCL_CHROMA_HFIR_TAP_NUM        = 2,	
	DF_VIDEO_SCL_CHROMA_HFIR_PHASE_LOG2BITS = 4,
	DF_VIDEO_SCL_CHROMA_HFIR_PHASE_NUM      = 1 << DF_VIDEO_SCL_CHROMA_HFIR_PHASE_LOG2BITS,

	//HD2SD Capture Block
	DF_HD2SD_SCL_LUMA_VFIR_TAP_NUM          = 2,
	DF_HD2SD_SCL_LUMA_VFIR_PHASE_LOG2BITS   = 4,
	DF_HD2SD_SCL_LUMA_VFIR_PHASE_NUM        = 1 << DF_HD2SD_SCL_LUMA_VFIR_PHASE_LOG2BITS,
	DF_HD2SD_SCL_LUMA_HFIR_TAP_NUM          = 4,	
	DF_HD2SD_SCL_LUMA_HFIR_PHASE_LOG2BITS   = 4,
	DF_HD2SD_SCL_LUMA_HFIR_PHASE_NUM        = 1 << DF_HD2SD_SCL_LUMA_HFIR_PHASE_LOG2BITS,
	
	DF_HD2SD_SCL_CHROMA_VFIR_TAP_NUM        = 2,
	DF_HD2SD_SCL_CHROMA_VFIR_PHASE_LOG2BITS = 4,
	DF_HD2SD_SCL_CHROMA_VFIR_PHASE_NUM      = 1 << DF_HD2SD_SCL_CHROMA_VFIR_PHASE_LOG2BITS,
	DF_HD2SD_SCL_CHROMA_HFIR_TAP_NUM        = 2,	
	DF_HD2SD_SCL_CHROMA_HFIR_PHASE_LOG2BITS = 4,
	DF_HD2SD_SCL_CHROMA_HFIR_PHASE_NUM      = 1 << DF_HD2SD_SCL_CHROMA_HFIR_PHASE_LOG2BITS,

	//Gfx Horizontal Scaler
	DF_GFX_SCL_STEP_FRACTION_BIT_WIDTH  = 10,
	DF_GFX_SCL_HFIR_TAP_NUM        = 2,
	DF_GFX_SCL_HFIR_PHASE_LOG2BITS = 6,
	DF_GFX_SCL_HFIR_PHASE_NUM      = 1 << DF_GFX_SCL_HFIR_PHASE_LOG2BITS,	
};

//************************************************************//
// generate the Cubic convolution interpolation coeff
//************************************************************//
int GenFIRCubicCoeff
(
 double *fCoeff,
 int TapNum,
 int PhaseMax,
 int SrcSize,
 int DesSize,
 int KenerlSel
 )
{
	double scaleratio,sampleInteral;
	int phaseindex;
	int tapindex;
	double samplePos;
	double sum;

	double (*fp)(double s,double m);
// 	assert(PhaseMax > 0);
// 	assert(TapNum > 0);
// 	assert(fCoeff != NULL);
// 	assert(DesSize >0 && SrcSize >0);
	scaleratio = (double)DesSize/SrcSize;
	sampleInteral = (double)1/PhaseMax;

	if (TapNum <= 2)
	{
		fp = LinearKernel;
		scaleratio = 1;
	}
	else
	{
		if (KenerlSel == 0)
		{
			fp = LinearKernel;
			scaleratio = 1;
		}
		else
		{
			fp = CubicKernel;
			if (scaleratio > 1) // scale down,should spread the interpolator kernel
			{
				scaleratio = 1;
			}
		}
	}
	for (phaseindex=0;phaseindex<PhaseMax;phaseindex++)
	{		
		samplePos = (TapNum-1)/2 + sampleInteral*(phaseindex);
		sum = 0;
		for(tapindex=0;tapindex<TapNum;tapindex++)
		{
			fCoeff[tapindex] = (*fp)(samplePos,scaleratio);
			sum += fCoeff[tapindex];
			samplePos = samplePos - 1;
		}
		for(tapindex=0;tapindex<TapNum;tapindex++)
		{
			fCoeff[tapindex] = fCoeff[tapindex] / sum;	// normalization
			/*
			   val = val * (1<<DF_SCL_COEFF_FRACTION_BIT_WIDTH);
			   valunsigned int = 0;
			   if (val<0)
			   {
			   valunsigned int = 1<<DF_SCL_COEFF_POLARITY_BIT;
			   val = -val;
			   }
			   FIRCoeff[tapindex] = valunsigned int + (unsigned int)val;
			   */
		}		
		//FIRCoeff = FIRCoeff + TapNum;
		fCoeff += TapNum;
	}

	return 1;
}

//************************************************************//
// the Cubic convolution interpolation kernel
// support area of the kernel is from -2 to 2
// refer to "Cubic Convolution Interpolation for Digital Image
// 	Processing",by ROBERT G.KEYS,in IEEE TRANSACTIONS
//	ON ACOUSTICS,SPEECH,1981
//************************************************************//
double CubicKernel
(
 double s, // sample position of the filter
 double m
 )
{
	double val = 0;
	s = s * m;
	if (s < 0)
		s = -s;
	if (s<=1)
		val=1.5*s*s*s-2.5*s*s+1;
	else
	{
		if (s<2)
			val=-0.5*s*s*s+2.5*s*s-4*s+2;
		else
			val = 0;
	}		
	return val;
}

//************************************************************//
// the linear interpolation kernel
// support area of the kernel is from -1 to 1
//************************************************************//
double LinearKernel
(
 double s, // sample position of the filter
 double m
 )
{
	double val = 0;
	if (s < 0)
		s = -s;
	if (s<=1)
		val= 1 - s;
	else
		val = 0;
	return val;
}

int GenFIRCoeff(signed int *FIRCoeff, int TapNum, int PhaseMax, int SrcSize, int DesSize)
{
	return GenFIRCoeffRaw(FIRCoeff, TapNum, PhaseMax, SrcSize, DesSize, 1);
}
int GenFIRCoeffBilinear(signed int *FIRCoeff, int TapNum, int PhaseMax, int SrcSize, int DesSize)
{
	return GenFIRCoeffRaw(FIRCoeff, TapNum, PhaseMax, SrcSize, DesSize, 0);
}
int GenFIRCoeffRaw(signed int *FIRCoeff, int TapNum, int PhaseMax, int SrcSize, int DesSize, int KerFuncIdx)
{
	int phaseindex = 0;
	int tapindex   = 0;
	int rt = 0;
	double *fCoeff = 0; /* NULL */
	double val = 0;
	signed int val32 = 0;
	
	fCoeff = (double *)malloc(sizeof(double)*TapNum * PhaseMax);
// 	assert(fCoeff != NULL);

	rt =  GenFIRCubicCoeff
		(
		 fCoeff,
		 TapNum,
		 PhaseMax,
		 SrcSize,
		 DesSize,
		 KerFuncIdx //1: Bicubic, 0 Bilinear
		);
	
	printf(" Generate Coeff: TapNum(%d),PhaseNum(%d), SrcSize(%d), DesSize(%d), CoeffType(%s)\n",
		TapNum, 
		PhaseMax, 
		SrcSize, 
		DesSize, 
		KerFuncIdx == 1 ? "Bicubic" : "Bilinear");
	
	for (phaseindex=0;phaseindex<PhaseMax;phaseindex++)
	{
		printf(" Phase(%2d):", phaseindex);
		for(tapindex=0;tapindex<TapNum;tapindex++)
		{
			val = fCoeff[tapindex  + phaseindex * TapNum];
			val32 = val * (1<<DF_SCL_COEFF_FRACTION_BIT_WIDTH);
			
			if (val32 < 0)
			{
				val32 = (1<<DF_SCL_COEFF_POLARITY_BIT) + (-val32);
			}
			FIRCoeff[tapindex] = val32;
			printf(" %8.4f (%08x) ", val, val32);
		}		
		FIRCoeff = FIRCoeff + TapNum;		
		printf(" \n");
	}
	
	free(fCoeff);
	
	return rt;
}

int main(void)
{
	unsigned int YVFIRCoeff[16][4];
	unsigned int YVFIRCoeff2[16][4];
	
	GenFIRCoeff(YVFIRCoeff[0], 
			4, /* DF_VIDEO_SCL_LUMA_HFIR_TAP_NUM, */ 
			16, /*DF_VIDEO_SCL_LUMA_HFIR_PHASE_NUM, */ 
			1024, // SrcCropYHeight, 
			1024 // DispWinYHeight
			);
	GenFIRCoeffBilinear(YVFIRCoeff2[0], 
			4, /* DF_VIDEO_SCL_LUMA_HFIR_TAP_NUM, */ 
			16, /* DF_VIDEO_SCL_LUMA_HFIR_PHASE_NUM, */ 
			1024, //SrcCropYHeight, 
			1024 //DispWinYHeight
			);

	{
		int ii, jj;

		for (ii = 0; ii < 16; ii++) { printf ("\n"); for (jj = 0; jj < 4; jj ++) printf("%d, ", YVFIRCoeff[ii][jj]); }
		printf("\n\n");
		for (ii = 0; ii < 16; ii++) { printf ("\n"); for (jj = 0; jj < 4; jj ++) printf("%d, ", YVFIRCoeff2[ii][jj]); }
	}

	return 0;
}
