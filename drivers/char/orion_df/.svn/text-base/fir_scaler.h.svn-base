#ifndef __DF_SCALOR_H__
#define __DF_SCALOR_H__

typedef struct _SCALER_CFG_
{
	int VFIRTapNum;
	int VFIRPhaseLog2Bit;
	unsigned int *VFIRCoeff;
	int VFIRInitPhase;

	int HFIRTapNum;
	int HFIRPhaseLog2Bit;
	unsigned int *HFIRCoeff;
	int HFIRInitPhase;

}scaler_cfg;

int GenFIRBilinearCoeff(signed int *FIRCoeff, int TapNum, int PhaseMax, int SrcSize, int DesSize);
int GenFIRCoeff(signed int *FIRCoeff, int TapNum, int PhaseMax, int SrcSize, int DesSize);
int GenFIRCoeffBilinear(signed int *FIRCoeff, int TapNum, int PhaseMax, int SrcSize, int DesSize);
int GenFIRCoeffRaw(signed int *FIRCoeff, int TapNum, int PhaseMax, int SrcSize, int DesSize, int KerFuncIdx);

int General32FIR(unsigned int *DataIn, unsigned int *Coeff, int FIRTapNum, signed int *DataOut);
unsigned int HW8TapFIR(unsigned int *DataIn, unsigned int *Coeff, int FIRTapNum, signed int *DataOut);

int ScalerHFIR
(
	int StepFractionBitWidth,
	unsigned char *SrcLine,
	unsigned char *DesLine,
	int SrcWidth, 
	int DesWidth, 
	unsigned int ScalerStep, 
	int FIRTapNum,
	int FIRPhaseLog2Bit,
	unsigned int *FIRCoeff,
	int FIRInitPhase,
	int KeyEna,
	unsigned char	*VAlphaOutLine,
	unsigned char	*DesKeyAlphaLine
);

unsigned int LumaKeying
(
	unsigned char  Luma,
	unsigned char  KeyMax,
	unsigned char  KeyMin,	
	unsigned char  KeyAlpha0,
	unsigned char  KeyAlpha1
);

int FIRScalerRaw
(
	//SrcInfo
	unsigned char *SrcBuf,
	int SrcLinePitch,
	int SrcWidth,
	int SrcHeight,
	unsigned char *DesBuf,
	int DesLinePitch,
	int DesWidth,
	int DesHeight,
	scaler_cfg *ScalerCfg,
	int KeyEna,
	unsigned char  KeyMax,
	unsigned char  KeyMin,	
	unsigned char  KeyAlpha0,
	unsigned char  KeyAlpha1,
	int AlphaLinePitch,
	unsigned char *DesKeyAlpha
);

int FIRScalerHFIR
(
	//SrcInfo
	unsigned char *SrcBuf,
	int SrcLinePitch,
	int SrcWidth,
	int SrcHeight,
	unsigned char *DesBuf,
	int DesLinePitch,
	int DesWidth,
	int YFlag,
	scaler_cfg *ScalerCfg
);

int FIRScalerVFIR
(
	//SrcInfo
	unsigned char *SrcBuf,
	int SrcLinePitch,
	int SrcWidth,
	int SrcHeight,
	unsigned char *DesBuf,
	int DesHeight,
	int YFlag,
	scaler_cfg *ScalerCfg
);


double CubicKernel
(
	double s, // sample position of the filter
	double m
);

double LinearKernel
(
	double s, // sample position of the filter
	double m
);

int GenFIRCubicCoeff
(
	double *fCoeff,
	int TapNum,
	int PhaseMax,
	int SrcSize,
	int DesSize,
	int KenerlSel
);

int DintChromaScalerHD2SD
(
	char *SrcBuf,
	int SrcLinePitch,
	int SrcWidth,
	int SrcHeight,
	char *DesBuf,
	int DesLinePitch,
	int DesWidth,
	int DesHeight,
	int VInitPhase,
	int HInitPhase
);

int DintLumaScalerHD2SD
(
	char *SrcBuf,
	int SrcLinePitch,
	int SrcWidth,
	int SrcHeight,
	char *DesBuf,
	int DesLinePitch,
	int DesWidth,
	int DesHeight,
	scaler_cfg *ScalerCfg
);

#endif
