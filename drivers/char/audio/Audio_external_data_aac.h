#ifndef __AUDIO_EXTERNAL_DATA_AAC_H__
#define __AUDIO_EXTERNAL_DATA_AAC_H__

// ROM data for AAC
const unsigned short kbdLongWindow[1024] = {
	0x0013, 	0x001c, 	0x0024, 	0x002b, 	0x0032, 	0x0038, 	0x003f, 	0x0046, 	
		0x004c, 	0x0053, 	0x005a, 	0x0061, 	0x0068, 	0x006f, 	0x0076, 	0x007e, 	
		0x0086, 	0x008d, 	0x0095, 	0x009d, 	0x00a6, 	0x00ae, 	0x00b7, 	0x00bf, 	
		0x00c8, 	0x00d1, 	0x00db, 	0x00e4, 	0x00ee, 	0x00f8, 	0x0102, 	0x010c, 	
		0x0117, 	0x0121, 	0x012c, 	0x0138, 	0x0143, 	0x014e, 	0x015a, 	0x0166, 	
		0x0172, 	0x017f, 	0x018c, 	0x0199, 	0x01a6, 	0x01b3, 	0x01c1, 	0x01cf, 	
		0x01dd, 	0x01eb, 	0x01fa, 	0x0208, 	0x0218, 	0x0227, 	0x0237, 	0x0246, 	
		0x0257, 	0x0267, 	0x0278, 	0x0289, 	0x029a, 	0x02ab, 	0x02bd, 	0x02cf, 	
		0x02e1, 	0x02f4, 	0x0307, 	0x031a, 	0x032e, 	0x0341, 	0x0355, 	0x036a, 	
		0x037e, 	0x0393, 	0x03a8, 	0x03be, 	0x03d4, 	0x03ea, 	0x0401, 	0x0417, 	
		0x042e, 	0x0446, 	0x045e, 	0x0476, 	0x048e, 	0x04a7, 	0x04c0, 	0x04d9, 	
		0x04f3, 	0x050d, 	0x0528, 	0x0542, 	0x055d, 	0x0579, 	0x0595, 	0x05b1, 	
		0x05cd, 	0x05ea, 	0x0607, 	0x0625, 	0x0643, 	0x0661, 	0x0680, 	0x069f, 	
		0x06be, 	0x06de, 	0x06fe, 	0x071f, 	0x073f, 	0x0761, 	0x0782, 	0x07a4, 	
		0x07c7, 	0x07ea, 	0x080d, 	0x0830, 	0x0854, 	0x0879, 	0x089e, 	0x08c3, 	
		0x08e8, 	0x090e, 	0x0935, 	0x095c, 	0x0983, 	0x09aa, 	0x09d2, 	0x09fb, 	
		0x0a24, 	0x0a4d, 	0x0a77, 	0x0aa1, 	0x0acb, 	0x0af6, 	0x0b22, 	0x0b4e, 	
		0x0b7a, 	0x0ba7, 	0x0bd4, 	0x0c01, 	0x0c2f, 	0x0c5e, 	0x0c8d, 	0x0cbc, 	
		0x0cec, 	0x0d1c, 	0x0d4d, 	0x0d7e, 	0x0daf, 	0x0de1, 	0x0e14, 	0x0e47, 	
		0x0e7a, 	0x0eae, 	0x0ee2, 	0x0f17, 	0x0f4c, 	0x0f82, 	0x0fb8, 	0x0fef, 	
		0x1026, 	0x105e, 	0x1096, 	0x10ce, 	0x1107, 	0x1141, 	0x117b, 	0x11b5, 	
		0x11f0, 	0x122b, 	0x1267, 	0x12a3, 	0x12e0, 	0x131d, 	0x135b, 	0x139a, 	
		0x13d8, 	0x1418, 	0x1457, 	0x1497, 	0x14d8, 	0x1519, 	0x155b, 	0x159d, 	
		0x15e0, 	0x1623, 	0x1667, 	0x16ab, 	0x16f0, 	0x1735, 	0x177a, 	0x17c1, 	
		0x1807, 	0x184e, 	0x1896, 	0x18de, 	0x1927, 	0x1970, 	0x19ba, 	0x1a04, 	
		0x1a4e, 	0x1a9a, 	0x1ae5, 	0x1b31, 	0x1b7e, 	0x1bcb, 	0x1c19, 	0x1c67, 	
		0x1cb6, 	0x1d05, 	0x1d55, 	0x1da5, 	0x1df6, 	0x1e47, 	0x1e99, 	0x1eeb, 	
		0x1f3e, 	0x1f91, 	0x1fe5, 	0x2039, 	0x208e, 	0x20e3, 	0x2139, 	0x218f, 	
		0x21e6, 	0x223d, 	0x2295, 	0x22ed, 	0x2346, 	0x239f, 	0x23f9, 	0x2454, 	
		0x24ae, 	0x250a, 	0x2566, 	0x25c2, 	0x261f, 	0x267c, 	0x26da, 	0x2738, 	
		0x2797, 	0x27f6, 	0x2856, 	0x28b6, 	0x2917, 	0x2979, 	0x29da, 	0x2a3d, 	
		0x2a9f, 	0x2b03, 	0x2b66, 	0x2bcb, 	0x2c2f, 	0x2c95, 	0x2cfa, 	0x2d61, 	
		0x2dc7, 	0x2e2e, 	0x2e96, 	0x2efe, 	0x2f67, 	0x2fd0, 	0x3039, 	0x30a4, 	
		0x310e, 	0x3179, 	0x31e4, 	0x3250, 	0x32bd, 	0x332a, 	0x3397, 	0x3405, 	
		0x3473, 	0x34e2, 	0x3551, 	0x35c0, 	0x3630, 	0x36a1, 	0x3712, 	0x3783, 	
		0x37f5, 	0x3868, 	0x38da, 	0x394e, 	0x39c1, 	0x3a35, 	0x3aaa, 	0x3b1f, 	
		0x3b94, 	0x3c0a, 	0x3c80, 	0x3cf7, 	0x3d6e, 	0x3de5, 	0x3e5d, 	0x3ed6, 	
		0x3f4e, 	0x3fc8, 	0x4041, 	0x40bb, 	0x4136, 	0x41b0, 	0x422c, 	0x42a7, 	
		0x4323, 	0x43a0, 	0x441c, 	0x449a, 	0x4517, 	0x4595, 	0x4613, 	0x4692, 	
		0x4711, 	0x4791, 	0x4811, 	0x4891, 	0x4911, 	0x4992, 	0x4a13, 	0x4a95, 	
		0x4b17, 	0x4b99, 	0x4c1c, 	0x4c9f, 	0x4d23, 	0x4da6, 	0x4e2a, 	0x4eaf, 	
		0x4f33, 	0x4fb8, 	0x503e, 	0x50c3, 	0x5149, 	0x51d0, 	0x5256, 	0x52dd, 	
		0x5365, 	0x53ec, 	0x5474, 	0x54fc, 	0x5584, 	0x560d, 	0x5696, 	0x571f, 	
		0x57a9, 	0x5833, 	0x58bd, 	0x5947, 	0x59d2, 	0x5a5d, 	0x5ae8, 	0x5b73, 	
		0x5bff, 	0x5c8b, 	0x5d17, 	0x5da3, 	0x5e30, 	0x5ebd, 	0x5f4a, 	0x5fd7, 	
		0x6065, 	0x60f2, 	0x6180, 	0x620f, 	0x629d, 	0x632c, 	0x63ba, 	0x6449, 	
		0x64d8, 	0x6568, 	0x65f7, 	0x6687, 	0x6717, 	0x67a7, 	0x6837, 	0x68c8, 	
		0x6958, 	0x69e9, 	0x6a7a, 	0x6b0b, 	0x6b9c, 	0x6c2d, 	0x6cbf, 	0x6d50, 	
		0x6de2, 	0x6e74, 	0x6f06, 	0x6f98, 	0x702a, 	0x70bd, 	0x714f, 	0x71e1, 	
		0x7274, 	0x7307, 	0x739a, 	0x742c, 	0x74bf, 	0x7552, 	0x75e6, 	0x7679, 	
		0x770c, 	0x779f, 	0x7833, 	0x78c6, 	0x795a, 	0x79ed, 	0x7a81, 	0x7b14, 	
		0x7ba8, 	0x7c3b, 	0x7ccf, 	0x7d63, 	0x7df6, 	0x7e8a, 	0x7f1e, 	0x7fb2, 	
		0x8045, 	0x80d9, 	0x816c, 	0x8200, 	0x8294, 	0x8327, 	0x83bb, 	0x844f, 	
		0x84e2, 	0x8576, 	0x8609, 	0x869d, 	0x8730, 	0x87c3, 	0x8857, 	0x88ea, 	
		0x897d, 	0x8a10, 	0x8aa3, 	0x8b36, 	0x8bc9, 	0x8c5c, 	0x8cee, 	0x8d81, 	
		0x8e14, 	0x8ea6, 	0x8f38, 	0x8fca, 	0x905c, 	0x90ee, 	0x9180, 	0x9212, 	
		0x92a3, 	0x9335, 	0x93c6, 	0x9457, 	0x94e8, 	0x9579, 	0x960a, 	0x969a, 	
		0x972a, 	0x97bb, 	0x984b, 	0x98da, 	0x996a, 	0x99f9, 	0x9a89, 	0x9b18, 	
		0x9ba7, 	0x9c35, 	0x9cc4, 	0x9d52, 	0x9de0, 	0x9e6e, 	0x9efc, 	0x9f89, 	
		0xa016, 	0xa0a3, 	0xa130, 	0xa1bd, 	0xa249, 	0xa2d5, 	0xa361, 	0xa3ec, 	
		0xa477, 	0xa502, 	0xa58d, 	0xa618, 	0xa6a2, 	0xa72c, 	0xa7b6, 	0xa83f, 	
		0xa8c8, 	0xa951, 	0xa9d9, 	0xaa62, 	0xaaea, 	0xab71, 	0xabf9, 	0xac80, 	
		0xad06, 	0xad8d, 	0xae13, 	0xae99, 	0xaf1e, 	0xafa3, 	0xb028, 	0xb0ac, 	
		0xb130, 	0xb1b4, 	0xb238, 	0xb2bb, 	0xb33e, 	0xb3c0, 	0xb442, 	0xb4c4, 	
		0xb545, 	0xb5c6, 	0xb647, 	0xb6c7, 	0xb747, 	0xb7c6, 	0xb845, 	0xb8c4, 	
		0xb942, 	0xb9c0, 	0xba3e, 	0xbabb, 	0xbb38, 	0xbbb4, 	0xbc30, 	0xbcac, 	
		0xbd27, 	0xbda2, 	0xbe1c, 	0xbe96, 	0xbf10, 	0xbf89, 	0xc002, 	0xc07a, 	
		0xc0f2, 	0xc16a, 	0xc1e1, 	0xc257, 	0xc2cd, 	0xc343, 	0xc3b8, 	0xc42d, 	
		0xc4a2, 	0xc516, 	0xc589, 	0xc5fc, 	0xc66f, 	0xc6e1, 	0xc753, 	0xc7c4, 	
		0xc835, 	0xc8a5, 	0xc915, 	0xc985, 	0xc9f4, 	0xca62, 	0xcad0, 	0xcb3e, 	
		0xcbab, 	0xcc18, 	0xcc84, 	0xccf0, 	0xcd5b, 	0xcdc6, 	0xce30, 	0xce9a, 	
		0xcf03, 	0xcf6c, 	0xcfd4, 	0xd03c, 	0xd0a3, 	0xd10a, 	0xd171, 	0xd1d6, 	
		0xd23c, 	0xd2a1, 	0xd305, 	0xd369, 	0xd3cd, 	0xd430, 	0xd492, 	0xd4f4, 	
		0xd555, 	0xd5b6, 	0xd617, 	0xd677, 	0xd6d6, 	0xd735, 	0xd794, 	0xd7f1, 	
		0xd84f, 	0xd8ac, 	0xd908, 	0xd964, 	0xd9bf, 	0xda1a, 	0xda75, 	0xdacf, 	
		0xdb28, 	0xdb81, 	0xdbd9, 	0xdc31, 	0xdc88, 	0xdcdf, 	0xdd35, 	0xdd8b, 	
		0xdde0, 	0xde35, 	0xde8a, 	0xdedd, 	0xdf30, 	0xdf83, 	0xdfd5, 	0xe027, 	
		0xe078, 	0xe0c9, 	0xe119, 	0xe169, 	0xe1b8, 	0xe207, 	0xe255, 	0xe2a2, 	
		0xe2ef, 	0xe33c, 	0xe388, 	0xe3d4, 	0xe41f, 	0xe469, 	0xe4b3, 	0xe4fd, 	
		0xe546, 	0xe58f, 	0xe5d7, 	0xe61e, 	0xe665, 	0xe6ac, 	0xe6f2, 	0xe737, 	
		0xe77c, 	0xe7c1, 	0xe805, 	0xe848, 	0xe88b, 	0xe8ce, 	0xe910, 	0xe952, 	
		0xe993, 	0xe9d3, 	0xea13, 	0xea53, 	0xea92, 	0xead1, 	0xeb0f, 	0xeb4c, 	
		0xeb89, 	0xebc6, 	0xec02, 	0xec3e, 	0xec79, 	0xecb4, 	0xecee, 	0xed28, 	
		0xed61, 	0xed9a, 	0xedd3, 	0xee0b, 	0xee42, 	0xee79, 	0xeeb0, 	0xeee6, 	
		0xef1b, 	0xef50, 	0xef85, 	0xefb9, 	0xefed, 	0xf020, 	0xf053, 	0xf086, 	
		0xf0b8, 	0xf0e9, 	0xf11a, 	0xf14b, 	0xf17b, 	0xf1ab, 	0xf1da, 	0xf209, 	
		0xf237, 	0xf265, 	0xf293, 	0xf2c0, 	0xf2ed, 	0xf319, 	0xf345, 	0xf370, 	
		0xf39b, 	0xf3c6, 	0xf3f0, 	0xf41a, 	0xf443, 	0xf46c, 	0xf495, 	0xf4bd, 	
		0xf4e5, 	0xf50c, 	0xf533, 	0xf559, 	0xf580, 	0xf5a5, 	0xf5cb, 	0xf5f0, 	
		0xf614, 	0xf638, 	0xf65c, 	0xf680, 	0xf6a3, 	0xf6c5, 	0xf6e8, 	0xf70a, 	
		0xf72b, 	0xf74c, 	0xf76d, 	0xf78e, 	0xf7ae, 	0xf7ce, 	0xf7ed, 	0xf80c, 	
		0xf82b, 	0xf849, 	0xf867, 	0xf885, 	0xf8a2, 	0xf8bf, 	0xf8dc, 	0xf8f8, 	
		0xf914, 	0xf930, 	0xf94b, 	0xf966, 	0xf981, 	0xf99b, 	0xf9b5, 	0xf9cf, 	
		0xf9e8, 	0xfa01, 	0xfa1a, 	0xfa32, 	0xfa4b, 	0xfa62, 	0xfa7a, 	0xfa91, 	
		0xfaa8, 	0xfabf, 	0xfad5, 	0xfaeb, 	0xfb01, 	0xfb17, 	0xfb2c, 	0xfb41, 	
		0xfb56, 	0xfb6a, 	0xfb7e, 	0xfb92, 	0xfba6, 	0xfbb9, 	0xfbcc, 	0xfbdf, 	
		0xfbf2, 	0xfc04, 	0xfc16, 	0xfc28, 	0xfc3a, 	0xfc4b, 	0xfc5c, 	0xfc6d, 	
		0xfc7d, 	0xfc8e, 	0xfc9e, 	0xfcae, 	0xfcbd, 	0xfccd, 	0xfcdc, 	0xfceb, 	
		0xfcfa, 	0xfd08, 	0xfd17, 	0xfd25, 	0xfd33, 	0xfd40, 	0xfd4e, 	0xfd5b, 	
		0xfd68, 	0xfd75, 	0xfd82, 	0xfd8e, 	0xfd9b, 	0xfda7, 	0xfdb3, 	0xfdbe, 	
		0xfdca, 	0xfdd5, 	0xfde0, 	0xfdeb, 	0xfdf6, 	0xfe01, 	0xfe0b, 	0xfe16, 	
		0xfe20, 	0xfe2a, 	0xfe34, 	0xfe3d, 	0xfe47, 	0xfe50, 	0xfe59, 	0xfe62, 	
		0xfe6b, 	0xfe74, 	0xfe7c, 	0xfe84, 	0xfe8d, 	0xfe95, 	0xfe9d, 	0xfea5, 	
		0xfeac, 	0xfeb4, 	0xfebb, 	0xfec2, 	0xfeca, 	0xfed1, 	0xfed7, 	0xfede, 	
		0xfee5, 	0xfeeb, 	0xfef2, 	0xfef8, 	0xfefe, 	0xff04, 	0xff0a, 	0xff10, 	
		0xff15, 	0xff1b, 	0xff21, 	0xff26, 	0xff2b, 	0xff30, 	0xff35, 	0xff3a, 	
		0xff3f, 	0xff44, 	0xff49, 	0xff4d, 	0xff52, 	0xff56, 	0xff5a, 	0xff5e, 	
		0xff63, 	0xff67, 	0xff6b, 	0xff6e, 	0xff72, 	0xff76, 	0xff79, 	0xff7d, 	
		0xff80, 	0xff84, 	0xff87, 	0xff8a, 	0xff8e, 	0xff91, 	0xff94, 	0xff97, 	
		0xff9a, 	0xff9c, 	0xff9f, 	0xffa2, 	0xffa4, 	0xffa7, 	0xffaa, 	0xffac, 	
		0xffae, 	0xffb1, 	0xffb3, 	0xffb5, 	0xffb7, 	0xffba, 	0xffbc, 	0xffbe, 	
		0xffc0, 	0xffc2, 	0xffc3, 	0xffc5, 	0xffc7, 	0xffc9, 	0xffca, 	0xffcc, 	
		0xffce, 	0xffcf, 	0xffd1, 	0xffd2, 	0xffd4, 	0xffd5, 	0xffd6, 	0xffd8, 	
		0xffd9, 	0xffda, 	0xffdc, 	0xffdd, 	0xffde, 	0xffdf, 	0xffe0, 	0xffe1, 	
		0xffe2, 	0xffe3, 	0xffe4, 	0xffe5, 	0xffe6, 	0xffe7, 	0xffe8, 	0xffe9, 	
		0xffea, 	0xffea, 	0xffeb, 	0xffec, 	0xffed, 	0xffed, 	0xffee, 	0xffef, 	
		0xffef, 	0xfff0, 	0xfff1, 	0xfff1, 	0xfff2, 	0xfff2, 	0xfff3, 	0xfff3, 	
		0xfff4, 	0xfff4, 	0xfff5, 	0xfff5, 	0xfff6, 	0xfff6, 	0xfff6, 	0xfff7, 	
		0xfff7, 	0xfff7, 	0xfff8, 	0xfff8, 	0xfff8, 	0xfff9, 	0xfff9, 	0xfff9, 	
		0xfffa, 	0xfffa, 	0xfffa, 	0xfffa, 	0xfffb, 	0xfffb, 	0xfffb, 	0xfffb, 	
		0xfffc, 	0xfffc, 	0xfffc, 	0xfffc, 	0xfffc, 	0xfffc, 	0xfffd, 	0xfffd, 	
		0xfffd, 	0xfffd, 	0xfffd, 	0xfffd, 	0xfffd, 	0xfffe, 	0xfffe, 	0xfffe, 	
		0xfffe, 	0xfffe, 	0xfffe, 	0xfffe, 	0xfffe, 	0xfffe, 	0xfffe, 	0xfffe, 	
		0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	
		0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	
		0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	
		0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	
		0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	0xffff, 	
};

// ROM data for SBR
/* Huffman tables from appendix 4.A.6.1, includes offset of -LAV[i] for table i */
const signed short huffTabSBR[604] = {
	/* SBR table sbr_tenv15 [121] (signed) */
	0,   -1,    1,   -2,    2,   -3,    3,   -4,    4,   -5,    5,   -6,    6,   -7,    7,   -8, 
		-9,    8,  -10,    9,  -11,   10,  -12,  -13,   11,  -14,   12,  -15,  -16,   13,  -19,  -18, 
		-17,   14,  -24,  -20,   16,  -26,  -21,   15,  -23,  -25,  -22,  -60,  -59,  -58,  -57,  -56, 
		-55,  -54,  -53,  -52,  -51,  -50,  -49,  -48,  -47,  -46,  -45,  -44,  -43,  -42,  -41,  -40, 
		-39,  -38,  -37,  -36,  -35,  -34,  -33,  -32,  -31,  -30,  -29,  -28,  -27,   17,   18,   19, 
		20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35, 
		36,   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51, 
		52,   53,   54,   55,   56,   57,   58,   59,   60, 
		/* SBR table sbr_fenv15 [121] (signed) */
		0,   -1,    1,   -2,   -3,    2,   -4,    3,   -5,    4,   -6,    5,   -7,    6,   -8,    7, 
		-9,    8,  -10,    9,  -11,   10,   11,  -12,   12,  -13,   13,   14,  -14,  -15,   15,   16, 
		17,  -16,  -17,  -18,  -19,   18,   19,  -20,  -21,   20,   21,  -24,  -23,  -22,  -26,  -28, 
		22,   23,   25,  -41,  -25,   26,   27,  -30,  -27,   24,   28,   44,  -51,  -46,  -44,  -43, 
		-37,  -33,  -31,  -29,   30,   37,   42,   47,   48,  -60,  -59,  -58,  -57,  -56,  -55,  -54, 
		-53,  -52,  -50,  -49,  -48,  -47,  -45,  -42,  -40,  -39,  -38,  -36,  -35,  -34,  -32,   29, 
		31,   32,   33,   34,   35,   36,   38,   39,   40,   41,   43,   45,   46,   49,   50,   51, 
		52,   53,   54,   55,   56,   57,   58,   59,   60, 
		/* SBR table sbr_tenv15b [49] (signed) */
		0,    1,   -1,    2,   -2,    3,   -3,    4,   -4,   -5,    5,   -6,    6,    7,   -7,    8, 
		-24,  -23,  -22,  -21,  -20,  -19,  -18,  -17,  -16,  -15,  -14,  -13,  -12,  -11,  -10,   -9, 
		-8,    9,   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22,   23, 
		24, 
		/* SBR table sbr_fenv15b [49] (signed) */
		0,   -1,    1,   -2,    2,    3,   -3,   -4,    4,   -5,    5,   -6,    6,   -7,    7,    8, 
		-9,   -8,  -24,  -23,  -22,  -21,  -20,  -19,  -18,  -17,  -16,  -15,  -14,  -13,  -12,  -11, 
		-10,    9,   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22,   23, 
		24, 
		/* SBR table sbr_tenv30 [63] (signed) */
		0,   -1,    1,   -2,    2,   -3,    3,   -4,    4,   -5,    5,   -6,   -7,    6,   -8,    7, 
		-9,  -10,    8,    9,   10,  -13,  -11,  -12,  -14,   11,   12,  -31,  -30,  -29,  -28,  -27, 
		-26,  -25,  -24,  -23,  -22,  -21,  -20,  -19,  -18,  -17,  -16,  -15,   13,   14,   15,   16, 
		17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31, 
		/* SBR table sbr_fenv30 [63] (signed) */
		0,   -1,    1,   -2,    2,   -3,    3,   -4,    4,   -5,    5,   -6,    6,   -7,    7,   -8, 
		8,    9,   -9,  -10,   10,   11,  -11,  -12,   12,   13,  -13,  -15,   14,   15,  -14,   18, 
		-18,  -24,  -19,   16,   17,  -22,  -21,  -16,   20,   21,   22,   25,  -23,  -20,   24,  -31, 
		-30,  -29,  -28,  -27,  -26,  -25,  -17,   19,   23,   26,   27,   28,   29,   30,   31, 
		/* SBR table sbr_tenv30b [25] (signed) */
		0,    1,   -1,   -2,    2,    3,   -3,   -4,    4,   -5,  -12,  -11,  -10,   -9,   -8,   -7, 
		-6,    5,    6,    7,    8,    9,   10,   11,   12, 
		/* SBR table sbr_fenv30b [25] (signed) */
		0,   -1,    1,   -2,    2,    3,   -3,   -4,    4,   -5,    5,    6,  -12,  -11,  -10,   -9, 
		-8,   -7,   -6,    7,    8,    9,   10,   11,   12, 
		/* SBR table sbr_tnoise30 [63] (signed) */
		0,    1,   -1,   -2,    2,   -3,    3,   -4,    4,   -5,    5,   11,  -31,  -30,  -29,  -28, 
		-27,  -26,  -25,  -24,  -23,  -22,  -21,  -20,  -19,  -18,  -17,  -16,  -15,  -14,  -13,  -12, 
		-11,  -10,   -9,   -8,   -7,   -6,    6,    7,    8,    9,   10,   12,   13,   14,   15,   16, 
		17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31, 
		/* SBR table sbr_tnoise30b [25] (signed) */
		0,   -1,    1,   -2,    2,  -12,  -11,  -10,   -9,   -8,   -7,   -6,   -5,   -4,   -3,    3, 
		4,    5,    6,    7,    8,    9,   10,   11,   12, 
};

const int cTabA[165] = {
	  0x00000000, 0x0055dba1, 0x01b2e41d, 0x09015651, 0x2e3a7532, 0xffed978a, 0x006090c4, 0x01fd3ba0, 0x08a24899, 0x311af3a4, 
		0xfff0065d, 0x006b47fa, 0x024bf7a1, 0x082f552e, 0x33ff670e, 0xffef7b8b, 0x0075fded, 0x029e35b4, 0x07a8127d, 0x36e69691, 
		0xffee1650, 0x00807994, 0x02f3e48d, 0x070bbf58, 0x39ce0477, 0xffecc31b, 0x008a7dd7, 0x034d01f0, 0x06593912, 0x3cb41219, 
		0xffeb50b2, 0x009424c6, 0x03a966bb, 0x0590a67d, 0x3f962fb8, 0xffe9ca76, 0x009d10bf, 0x04083fec, 0x04b0adcb, 0x4272a385, 
		0xffe88ba8, 0x00a520bb, 0x04694101, 0x03b8f8dc, 0x4547daea, 0xffe79e16, 0x00abe79e, 0x04cc2fcf, 0x02a99097, 0x4812f848, 
		0xffe6d466, 0x00b1978d, 0x05303f87, 0x01816e06, 0x4ad237a2, 0xffe65416, 0x00b5c867, 0x05950122, 0x0040c496, 0x4d83976c, 
		0xffe66dd0, 0x00b8394b, 0x05f9c051, 0xfee723c6, 0x5024d70e, 0xffe69423, 0x00b8c6b0, 0x065dd56a, 0xfd7475d8, 0x52b449de, 
		0xffe75361, 0x00b73ab0, 0x06c0f0c0, 0xfbe8f5bd, 0x552f8ff7, 0xffe85b4b, 0x00b36acd, 0x0721bf22, 0xfa44a069, 0x579505f5, 
		0xffea353a, 0x00acbd2f, 0x077fedb3, 0xf887507c, 0x59e2f69e, 0xffec8409, 0x00a3508f, 0x07da2b7f, 0xf6b1f3c3, 0x5c16d0ae, 
		0xffef2395, 0x0096dcc2, 0x08303897, 0xf4c473c6, 0x5e2f6367, 0xfff294c3, 0x00872c63, 0x0880ffdd, 0xf2bf6ea4, 0x602b0c7f, 
		0xfff681d6, 0x007400b8, 0x08cb4e23, 0xf0a3959f, 0x6207f220, 0xfffb42b0, 0x005d36df, 0x090ec1fc, 0xee71b2fe, 0x63c45243, 
		0x00007134, 0x00426f36, 0x0949eaac, 0xec2a3f5f, 0x655f63f2, 0x0006b1cf, 0x0023b989, 0x097c1ee8, 0xe9cea84a, 0x66d76725, 
		0x000d31b5, 0x0000e790, 0x09a3e163, 0xe75f8bb8, 0x682b39a4, 0x001471f8, 0xffda17f2, 0x09c0e59f, 0xe4de0cb0, 0x6959709d, 
		0x001c3549, 0xffaea5d6, 0x09d19ca9, 0xe24b8f66, 0x6a619c5e, 0x0024dd50, 0xff7ee3f1, 0x09d5560b, 0xdfa93ab5, 0x6b42a864, 
		0x002d8e42, 0xff4aabc8, 0x09caeb0f, 0xdcf898fb, 0x6bfbdd98, 0x003745f9, 0xff120d70, 0x09b18a1d, 0xda3b176a, 0x6c8c4c7a, 
		0x004103f4, 0xfed4bec3, 0x09881dc5, 0xd7722f04, 0x6cf4073e, 0x004b6c46, 0xfe933dc0, 0x094d7ec2, 0xd49fd55f, 0x6d32730f, 
		0x0055dba1, 0x01b2e41d, 0x09015651, 0x2e3a7532, 0x6d474e1d, 
};
/* coefficient table 4.A.87, format = Q31
* reordered as cTab[0], cTab[64], cTab[128], ... cTab[576], cTab[1], cTab[65], cTab[129], ... cTab[639] 
* keeping full table (not using symmetry) to allow sequential access in synth filter inner loop
* format = Q31
*/
const int cTabS[640] = {
	0x00000000, 0x0055dba1, 0x01b2e41d, 0x09015651, 0x2e3a7532, 0x6d474e1d, 0xd1c58ace, 0x09015651, 0xfe4d1be3, 0x0055dba1, 
		0xffede50e, 0x005b5371, 0x01d78bfc, 0x08d3e41b, 0x2faa221c, 0x6d41d963, 0xd3337b3d, 0x09299ead, 0xfe70b8d1, 0x0050b177, 
		0xffed978a, 0x006090c4, 0x01fd3ba0, 0x08a24899, 0x311af3a4, 0x6d32730f, 0xd49fd55f, 0x094d7ec2, 0xfe933dc0, 0x004b6c46, 
		0xffefc9b9, 0x0065fde5, 0x02244a24, 0x086b1eeb, 0x328cc6f0, 0x6d18520e, 0xd60a46e5, 0x096d0e21, 0xfeb48d0d, 0x00465348, 
		0xfff0065d, 0x006b47fa, 0x024bf7a1, 0x082f552e, 0x33ff670e, 0x6cf4073e, 0xd7722f04, 0x09881dc5, 0xfed4bec3, 0x004103f4, 
		0xffeff6ca, 0x0070c8a5, 0x0274ba43, 0x07ee507c, 0x3572ec70, 0x6cc59bab, 0xd8d7f21f, 0x099ec3dc, 0xfef3f6ab, 0x003c1fa4, 
		0xffef7b8b, 0x0075fded, 0x029e35b4, 0x07a8127d, 0x36e69691, 0x6c8c4c7a, 0xda3b176a, 0x09b18a1d, 0xff120d70, 0x003745f9, 
		0xffeedfa4, 0x007b3875, 0x02c89901, 0x075ca90c, 0x385a49c4, 0x6c492217, 0xdb9b5b12, 0x09c018ce, 0xff2ef725, 0x00329ab6, 
		0xffee1650, 0x00807994, 0x02f3e48d, 0x070bbf58, 0x39ce0477, 0x6bfbdd98, 0xdcf898fb, 0x09caeb0f, 0xff4aabc8, 0x002d8e42, 
		0xffed651d, 0x0085c217, 0x03201116, 0x06b559c3, 0x3b415115, 0x6ba4629f, 0xde529086, 0x09d1fa23, 0xff6542d1, 0x00293718, 
		0xffecc31b, 0x008a7dd7, 0x034d01f0, 0x06593912, 0x3cb41219, 0x6b42a864, 0xdfa93ab5, 0x09d5560b, 0xff7ee3f1, 0x0024dd50, 
		0xffebe77b, 0x008f4bfc, 0x037ad438, 0x05f7fb90, 0x3e25b17e, 0x6ad73e8d, 0xe0fc421e, 0x09d52709, 0xff975c01, 0x002064f8, 
		0xffeb50b2, 0x009424c6, 0x03a966bb, 0x0590a67d, 0x3f962fb8, 0x6a619c5e, 0xe24b8f66, 0x09d19ca9, 0xffaea5d6, 0x001c3549, 
		0xffea9192, 0x0098b855, 0x03d8afe6, 0x05237f9d, 0x41058bc6, 0x69e29784, 0xe396a45d, 0x09cab9f2, 0xffc4e365, 0x0018703f, 
		0xffe9ca76, 0x009d10bf, 0x04083fec, 0x04b0adcb, 0x4272a385, 0x6959709d, 0xe4de0cb0, 0x09c0e59f, 0xffda17f2, 0x001471f8, 
		0xffe940f4, 0x00a1039c, 0x043889c6, 0x0437fb0a, 0x43de620a, 0x68c7269b, 0xe620c476, 0x09b3d77f, 0xffee183b, 0x0010bc63, 
		0xffe88ba8, 0x00a520bb, 0x04694101, 0x03b8f8dc, 0x4547daea, 0x682b39a4, 0xe75f8bb8, 0x09a3e163, 0x0000e790, 0x000d31b5, 
		0xffe83a07, 0x00a8739d, 0x049aa82f, 0x03343533, 0x46aea856, 0x6785c24d, 0xe89971b7, 0x099140a7, 0x00131c75, 0x0009aa3f, 
		0xffe79e16, 0x00abe79e, 0x04cc2fcf, 0x02a99097, 0x4812f848, 0x66d76725, 0xe9cea84a, 0x097c1ee8, 0x0023b989, 0x0006b1cf, 
		0xffe7746e, 0x00af374c, 0x04fe20be, 0x02186a91, 0x4973fef1, 0x661fd6b8, 0xeafee7f1, 0x0963ed46, 0x0033b927, 0x00039609, 
		0xffe6d466, 0x00b1978d, 0x05303f87, 0x01816e06, 0x4ad237a2, 0x655f63f2, 0xec2a3f5f, 0x0949eaac, 0x00426f36, 0x00007134, 
		0xffe6afee, 0x00b3d15c, 0x05626209, 0x00e42fa2, 0x4c2ca3df, 0x64964063, 0xed50a31d, 0x092d7970, 0x00504f41, 0xfffdfa25, 
		0xffe65416, 0x00b5c867, 0x05950122, 0x0040c496, 0x4d83976c, 0x63c45243, 0xee71b2fe, 0x090ec1fc, 0x005d36df, 0xfffb42b0, 
		0xffe681c6, 0x00b74c37, 0x05c76fed, 0xff96db90, 0x4ed62be3, 0x62ea6474, 0xef8d4d7b, 0x08edfeaa, 0x006928a0, 0xfff91fca, 
		0xffe66dd0, 0x00b8394b, 0x05f9c051, 0xfee723c6, 0x5024d70e, 0x6207f220, 0xf0a3959f, 0x08cb4e23, 0x007400b8, 0xfff681d6, 
		0xffe66fac, 0x00b8fe0d, 0x062bf5ec, 0xfe310657, 0x516eefb9, 0x611d58a3, 0xf1b461ab, 0x08a75da4, 0x007e0393, 0xfff48700, 
		0xffe69423, 0x00b8c6b0, 0x065dd56a, 0xfd7475d8, 0x52b449de, 0x602b0c7f, 0xf2bf6ea4, 0x0880ffdd, 0x00872c63, 0xfff294c3, 
		0xffe6fed4, 0x00b85f70, 0x068f8b44, 0xfcb1d740, 0x53f495aa, 0x5f30ff5f, 0xf3c4e887, 0x08594887, 0x008f87aa, 0xfff0e7ef, 
		0xffe75361, 0x00b73ab0, 0x06c0f0c0, 0xfbe8f5bd, 0x552f8ff7, 0x5e2f6367, 0xf4c473c6, 0x08303897, 0x0096dcc2, 0xffef2395, 
		0xffe80414, 0x00b58c8c, 0x06f1825d, 0xfb19b7bd, 0x56654bdd, 0x5d26be9b, 0xf5be0fa9, 0x08061671, 0x009da526, 0xffedc418, 
		0xffe85b4b, 0x00b36acd, 0x0721bf22, 0xfa44a069, 0x579505f5, 0x5c16d0ae, 0xf6b1f3c3, 0x07da2b7f, 0x00a3508f, 0xffec8409, 
		0xffe954d0, 0x00b06b68, 0x075112a2, 0xf96916f5, 0x58befacd, 0x5b001db8, 0xf79fa13a, 0x07ad8c26, 0x00a85e94, 0xffeb3849, 
		0xffea353a, 0x00acbd2f, 0x077fedb3, 0xf887507c, 0x59e2f69e, 0x59e2f69e, 0xf887507c, 0x077fedb3, 0x00acbd2f, 0xffea353a, 
		0xffeb3849, 0x00a85e94, 0x07ad8c26, 0xf79fa13a, 0x5b001db8, 0x58befacd, 0xf96916f5, 0x075112a2, 0x00b06b68, 0xffe954d0, 
		0xffec8409, 0x00a3508f, 0x07da2b7f, 0xf6b1f3c3, 0x5c16d0ae, 0x579505f5, 0xfa44a069, 0x0721bf22, 0x00b36acd, 0xffe85b4b, 
		0xffedc418, 0x009da526, 0x08061671, 0xf5be0fa9, 0x5d26be9b, 0x56654bdd, 0xfb19b7bd, 0x06f1825d, 0x00b58c8c, 0xffe80414, 
		0xffef2395, 0x0096dcc2, 0x08303897, 0xf4c473c6, 0x5e2f6367, 0x552f8ff7, 0xfbe8f5bd, 0x06c0f0c0, 0x00b73ab0, 0xffe75361, 
		0xfff0e7ef, 0x008f87aa, 0x08594887, 0xf3c4e887, 0x5f30ff5f, 0x53f495aa, 0xfcb1d740, 0x068f8b44, 0x00b85f70, 0xffe6fed4, 
		0xfff294c3, 0x00872c63, 0x0880ffdd, 0xf2bf6ea4, 0x602b0c7f, 0x52b449de, 0xfd7475d8, 0x065dd56a, 0x00b8c6b0, 0xffe69423, 
		0xfff48700, 0x007e0393, 0x08a75da4, 0xf1b461ab, 0x611d58a3, 0x516eefb9, 0xfe310657, 0x062bf5ec, 0x00b8fe0d, 0xffe66fac, 
		0xfff681d6, 0x007400b8, 0x08cb4e23, 0xf0a3959f, 0x6207f220, 0x5024d70e, 0xfee723c6, 0x05f9c051, 0x00b8394b, 0xffe66dd0, 
		0xfff91fca, 0x006928a0, 0x08edfeaa, 0xef8d4d7b, 0x62ea6474, 0x4ed62be3, 0xff96db90, 0x05c76fed, 0x00b74c37, 0xffe681c6, 
		0xfffb42b0, 0x005d36df, 0x090ec1fc, 0xee71b2fe, 0x63c45243, 0x4d83976c, 0x0040c496, 0x05950122, 0x00b5c867, 0xffe65416, 
		0xfffdfa25, 0x00504f41, 0x092d7970, 0xed50a31d, 0x64964063, 0x4c2ca3df, 0x00e42fa2, 0x05626209, 0x00b3d15c, 0xffe6afee, 
		0x00007134, 0x00426f36, 0x0949eaac, 0xec2a3f5f, 0x655f63f2, 0x4ad237a2, 0x01816e06, 0x05303f87, 0x00b1978d, 0xffe6d466, 
		0x00039609, 0x0033b927, 0x0963ed46, 0xeafee7f1, 0x661fd6b8, 0x4973fef1, 0x02186a91, 0x04fe20be, 0x00af374c, 0xffe7746e, 
		0x0006b1cf, 0x0023b989, 0x097c1ee8, 0xe9cea84a, 0x66d76725, 0x4812f848, 0x02a99097, 0x04cc2fcf, 0x00abe79e, 0xffe79e16, 
		0x0009aa3f, 0x00131c75, 0x099140a7, 0xe89971b7, 0x6785c24d, 0x46aea856, 0x03343533, 0x049aa82f, 0x00a8739d, 0xffe83a07, 
		0x000d31b5, 0x0000e790, 0x09a3e163, 0xe75f8bb8, 0x682b39a4, 0x4547daea, 0x03b8f8dc, 0x04694101, 0x00a520bb, 0xffe88ba8, 
		0x0010bc63, 0xffee183b, 0x09b3d77f, 0xe620c476, 0x68c7269b, 0x43de620a, 0x0437fb0a, 0x043889c6, 0x00a1039c, 0xffe940f4, 
		0x001471f8, 0xffda17f2, 0x09c0e59f, 0xe4de0cb0, 0x6959709d, 0x4272a385, 0x04b0adcb, 0x04083fec, 0x009d10bf, 0xffe9ca76, 
		0x0018703f, 0xffc4e365, 0x09cab9f2, 0xe396a45d, 0x69e29784, 0x41058bc6, 0x05237f9d, 0x03d8afe6, 0x0098b855, 0xffea9192, 
		0x001c3549, 0xffaea5d6, 0x09d19ca9, 0xe24b8f66, 0x6a619c5e, 0x3f962fb8, 0x0590a67d, 0x03a966bb, 0x009424c6, 0xffeb50b2, 
		0x002064f8, 0xff975c01, 0x09d52709, 0xe0fc421e, 0x6ad73e8d, 0x3e25b17e, 0x05f7fb90, 0x037ad438, 0x008f4bfc, 0xffebe77b, 
		0x0024dd50, 0xff7ee3f1, 0x09d5560b, 0xdfa93ab5, 0x6b42a864, 0x3cb41219, 0x06593912, 0x034d01f0, 0x008a7dd7, 0xffecc31b, 
		0x00293718, 0xff6542d1, 0x09d1fa23, 0xde529086, 0x6ba4629f, 0x3b415115, 0x06b559c3, 0x03201116, 0x0085c217, 0xffed651d, 
		0x002d8e42, 0xff4aabc8, 0x09caeb0f, 0xdcf898fb, 0x6bfbdd98, 0x39ce0477, 0x070bbf58, 0x02f3e48d, 0x00807994, 0xffee1650, 
		0x00329ab6, 0xff2ef725, 0x09c018ce, 0xdb9b5b12, 0x6c492217, 0x385a49c4, 0x075ca90c, 0x02c89901, 0x007b3875, 0xffeedfa4, 
		0x003745f9, 0xff120d70, 0x09b18a1d, 0xda3b176a, 0x6c8c4c7a, 0x36e69691, 0x07a8127d, 0x029e35b4, 0x0075fded, 0xffef7b8b, 
		0x003c1fa4, 0xfef3f6ab, 0x099ec3dc, 0xd8d7f21f, 0x6cc59bab, 0x3572ec70, 0x07ee507c, 0x0274ba43, 0x0070c8a5, 0xffeff6ca, 
		0x004103f4, 0xfed4bec3, 0x09881dc5, 0xd7722f04, 0x6cf4073e, 0x33ff670e, 0x082f552e, 0x024bf7a1, 0x006b47fa, 0xfff0065d, 
		0x00465348, 0xfeb48d0d, 0x096d0e21, 0xd60a46e5, 0x6d18520e, 0x328cc6f0, 0x086b1eeb, 0x02244a24, 0x0065fde5, 0xffefc9b9, 
		0x004b6c46, 0xfe933dc0, 0x094d7ec2, 0xd49fd55f, 0x6d32730f, 0x311af3a4, 0x08a24899, 0x01fd3ba0, 0x006090c4, 0xffed978a, 
		0x0050b177, 0xfe70b8d1, 0x09299ead, 0xd3337b3d, 0x6d41d963, 0x2faa221c, 0x08d3e41b, 0x01d78bfc, 0x005b5371, 0xffede50f, 
};

/* noise table 4.A.88, format = Q31 */
const int noiseTab[512*2] = {
	0x8010fd38, 0xb3dc7948, 0x7c4e2301, 0xa9904192, 0x121622a7, 0x86489625, 0xc3d53d25, 0xd0343fa9, 
		0x674d6f70, 0x25f4e9fd, 0xce1a8c8b, 0x72a726c5, 0xfea6efc6, 0xaa4adb1a, 0x8b2dd628, 0xf14029e4, 
		0x46321c1a, 0x604889a0, 0x33363b63, 0x815ed069, 0x802b4315, 0x8f2bf7f3, 0x85b86073, 0x745cfb46, 
		0xc57886b3, 0xb76731f0, 0xa2a66772, 0x828ca631, 0x60cc145e, 0x1ad1010f, 0x090c83d4, 0x9bd7ba87, 
		0x5f5aeea2, 0x8b4dbd99, 0x848e7b1e, 0x86bb9fa2, 0x26f18ae5, 0xc0b81194, 0x553407bf, 0x52c17953, 
		0x755f468d, 0x166b04f8, 0xa5687981, 0x4343248b, 0xa6558d5e, 0xc5f6fab7, 0x80a4fb8c, 0x8cb53cb7, 
		0x7da68a54, 0x9cd8df8a, 0xba05376c, 0xfcb58ee2, 0xfdd657a4, 0x005e35ca, 0x91c75c55, 0x367651e6, 
		0x816abf85, 0x8f831c4f, 0x423f9c9c, 0x55aa919e, 0x80779834, 0xb59f4244, 0x800a095c, 0x7de9e0cc, 
		0x46bda5cb, 0x4c184464, 0x2c438f71, 0x797216b5, 0x5035cee6, 0xa0c3a26e, 0x9d3f95fa, 0xd4a100c0, 
		0x8ac30dac, 0x04b87397, 0x9e5ac516, 0x8b0b442e, 0x66210ad6, 0x88ba7598, 0x45b9bd33, 0xf0be5087, 
		0x9261b85e, 0x364f6a31, 0x891c4b50, 0x23ad08ce, 0xf10366a6, 0x80414276, 0x1b562e06, 0x8be21591, 
		0x9e798195, 0x7fb4045c, 0x7d9506cf, 0x854e691f, 0x9207f092, 0x7a94c9d5, 0x88911536, 0x3f45cc61, 
		0x27059279, 0xa5b57109, 0x6d2bb67b, 0x3bdc5379, 0x74e662d8, 0x80348f8c, 0xf875e638, 0x5a8caea1, 
		0x2459ae75, 0x2c54b939, 0x79ee3203, 0xb9bc8683, 0x9b6f630c, 0x9f45b351, 0x8563b2b9, 0xe5dbba41, 
		0x697c7d0d, 0x7bb7c90e, 0xac900866, 0x8e6b5177, 0x8822dd37, 0x7fd5a91e, 0x7506da05, 0x82302aca, 
		0xa5e4be04, 0x4b4288eb, 0x00b8bc9f, 0x4f1033e4, 0x7200d612, 0x43900c8c, 0xa815b900, 0x676ed1d4, 
		0x5c5f23b2, 0xa758ee11, 0xaf73abfa, 0x11714ec0, 0x265239e0, 0xc50de679, 0x8a84e341, 0xa1438354, 
		0x7f1a341f, 0x343ec96b, 0x696e71b0, 0xa13bde39, 0x81e75094, 0x80091111, 0x853a73bf, 0x80f9c1ee, 
		0xe4980086, 0x886a8e28, 0xa7e89426, 0xdd93edd7, 0x7592100d, 0x0bfa8123, 0x850a26d4, 0x2e34f395, 
		0x421b6c00, 0xa4a462e4, 0x4e3f5090, 0x3c189f4c, 0x3c971a56, 0xdd0376d2, 0x747a5367, 0x7bcbc9d7, 
		0x3966be6a, 0x7efda616, 0x55445e15, 0x7ba2ab3f, 0x5fe684f2, 0x8cf42af9, 0x808c61c3, 0x4390c27b, 
		0x7cac62ff, 0xea6cab22, 0x5d0902ad, 0xc27b7208, 0x7a27389d, 0x5820a357, 0xa29bbe59, 0x9df0f1fd, 
		0x92bd67e5, 0x7195b587, 0x97cac65b, 0x8339807e, 0x8f72d832, 0x5fad8685, 0xa462d9d3, 0x81d46214, 
		0x6ae93e1d, 0x6b23a5b9, 0xc2732874, 0x81795268, 0x7c568cb6, 0x668513ea, 0x428d024e, 0x66b78b3a, 
		0xfee9ef03, 0x9ddcbb82, 0xa605f07e, 0x46dc55e0, 0x85415054, 0xc89ec271, 0x7c42edfb, 0x0befe59b, 
		0x89b8f607, 0x6d732a1a, 0xa7081ebd, 0x7e403258, 0x21feeb7b, 0x5dd7a1e7, 0x23e3a31a, 0x129bc896, 
		0xa11a6b54, 0x7f1e031c, 0xfdc1a4d1, 0x96402e53, 0xb9700f1a, 0x8168ecd6, 0x7d63d3cc, 0x87a70d65, 
		0x81075a7a, 0x55c8caa7, 0xa95d00b5, 0x102b1652, 0x0bb30215, 0xe5b63237, 0xa446ca44, 0x82d4c333, 
		0x67b2e094, 0x44c3d661, 0x33fd6036, 0xde1ea2a1, 0xa95e8e47, 0x78f66eb9, 0x6f2aef1e, 0xe8887247, 
		0x80a3b70e, 0xfca0d9d3, 0x6bf0fd20, 0x0d5226de, 0xf4341c87, 0x5902df05, 0x7ff1a38d, 0xf02e5a5b, 
		0x99f129af, 0x8ac63d01, 0x7b53f599, 0x7bb32532, 0x99ac59b0, 0x5255a80f, 0xf1320a41, 0x2497aa5c, 
		0xcce60bd8, 0x787c634b, 0x7ed58c5b, 0x8a28eb3a, 0x24a5e647, 0x8b79a2c1, 0x955f5ce5, 0xa9d12bc4, 
		0x7a1e20c6, 0x3eeda7ac, 0xf7be823a, 0x042924ce, 0x808b3f03, 0x364248da, 0xac2895e5, 0x69a8b5fa, 
		0x97fe8b63, 0xbdeac9aa, 0x8073e0ad, 0x6c25dba7, 0x005e51d2, 0x52e74389, 0x59d3988c, 0xe5d1f39c, 
		0x7b57dc91, 0x341adbe7, 0xa7d42b8d, 0x74e9f335, 0xd35bf7d8, 0x5b7c0a4b, 0x75bc0874, 0x552129bf, 
		0x8144b70d, 0x6de93bbb, 0x5825f14b, 0x473ec5ca, 0x80a8f37c, 0xe6552d69, 0x7898360b, 0x806379b0, 
		0xa9b59339, 0x3f6bf60c, 0xc367d731, 0x920ade99, 0x125592f7, 0x877e5ed1, 0xda895d95, 0x075f2ece, 
		0x380e5f5e, 0x9b006b62, 0xd17a6dd2, 0x530a0e13, 0xf4cc9a14, 0x7d0a0ed4, 0x847c6e3f, 0xbaee4975, 
		0x47131163, 0x64fb2cac, 0x5e2100a6, 0x7b756a42, 0xd87609f4, 0x98bfe48c, 0x0493745e, 0x836c5784, 
		0x7e5ccb40, 0x3df6b476, 0x97700d28, 0x8bbd93fd, 0x56de9cdb, 0x680b4e65, 0xebc3d90e, 0x6d286793, 
		0x6753712e, 0xe05c98a7, 0x3d2b6b85, 0xc4b18ddb, 0x7b59b869, 0x31435688, 0x811888e9, 0xe011ee7a, 
		0x6a5844f9, 0x86ae35ea, 0xb4cbc10b, 0x01a6f5d6, 0x7a49ed64, 0x927caa49, 0x847ddaed, 0xae0d9bb6, 
		0x836bdb04, 0x0fd810a6, 0x74fe126b, 0x4a346b5f, 0x80184d36, 0x5afd153c, 0x90cc8102, 0xe606d0e6, 
		0xde69aa58, 0xa89f1222, 0xe06df715, 0x8fd16144, 0x0317c3e8, 0x22ce92fc, 0x690c3eca, 0x93166f02, 
		0x71573414, 0x8d43cffb, 0xe8bd0bb6, 0xde86770f, 0x0bf99a41, 0x4633a661, 0xba064108, 0x7adafae3, 
		0x2f6cde5d, 0xb350a52c, 0xa5ebfb0b, 0x74c57b46, 0xd3b603b5, 0x80b70892, 0xa7f7fa53, 0xd94b566c, 
		0xdda3fd86, 0x6a635793, 0x3ed005ca, 0xc5f087d8, 0x31e3a746, 0x7a4278f9, 0x82def1f9, 0x06caa2b2, 
		0xe9d2c349, 0x8940e7f7, 0x7feef8dd, 0x4a9b01f0, 0xacde69f8, 0x57ddc280, 0xf09e4ba4, 0xb6d9f729, 
		0xb48c18f2, 0xd3654aa9, 0xca7a03c8, 0x14d57545, 0x7fda87a5, 0x0e411366, 0xb77d0df0, 0x8c2aa467, 
		0x787f2590, 0x2d292db1, 0x9f12682c, 0x44ac364d, 0x1a4b31a6, 0x871f7ded, 0x7ff99167, 0x6630a1d5, 
		0x25385eb9, 0x2d4dd549, 0xaf8a7004, 0x319ebe0f, 0x379ab730, 0x81dc56a4, 0x822d8523, 0x1ae8554c, 
		0x18fa0786, 0x875f7de4, 0x85ca350f, 0x7de818dc, 0x7786a38f, 0xa5456355, 0x92e60f88, 0xf5526122, 
		0x916039bc, 0xc561e2de, 0x31c42042, 0x7c82e290, 0x75d158b2, 0xb015bda1, 0x7220c750, 0x46565441, 
		0xd0da1fdd, 0x7b777481, 0x782e73c6, 0x8cd72b7b, 0x7f1006aa, 0xfb30e51e, 0x87994818, 0x34e7c7db, 
		0x7faae06b, 0xea74fbc0, 0xd20c7af4, 0xc44f396b, 0x06b4234e, 0xdf2e2a93, 0x2efb07c8, 0xce861911, 
		0x7550ea05, 0xd8d90bbb, 0x58522eec, 0x746b3520, 0xce844ce9, 0x7f5cacc3, 0xda8f17e0, 0x2fedf9cb, 
		0xb2f77ec4, 0x6f13f4c0, 0x834de085, 0x7b7ace4b, 0x713b16ac, 0x499c5ab0, 0x06a7961d, 0x1b39a48a, 
		0xbb853e6e, 0x7c781cc1, 0xc0baebf5, 0x7dace394, 0x815ceebc, 0xcc7b27d4, 0x8274b181, 0xa2be40a2, 
		0xdd01d5dc, 0x7fefeb14, 0x0813ec78, 0xba3077cc, 0xe5cf1e1c, 0xedcfacae, 0x54c43a9b, 0x5cd62a42, 
		0x93806b55, 0x03095c5b, 0x8e076ae3, 0x71bfcd2a, 0x7ac1989b, 0x623bc71a, 0x5e15d4d2, 0xfb341dd1, 
		0xd75dfbca, 0xd0da32be, 0xd4569063, 0x337869da, 0x3d30606a, 0xcd89cca2, 0x7dd2ae36, 0x028c03cd, 
		0xd85e052c, 0xe8dc9ec5, 0x7ffd9241, 0xde5bf4c6, 0x88c4b235, 0x8228be2e, 0x7fe6ec64, 0x996abe6a, 
		0xdeb0666d, 0x9eb86611, 0xd249b922, 0x18b3e26b, 0x80211168, 0x5f8bb99c, 0x6ecb0dd2, 0x4728ff8d, 
		0x2ac325b8, 0x6e5169d2, 0x7ebbd68d, 0x05e41d17, 0xaaa19f28, 0x8ab238a6, 0x51f105be, 0x140809cc, 
		0x7f7345d9, 0x3aae5a9d, 0xaecec6e4, 0x1afb3473, 0xf6229ed1, 0x8d55f467, 0x7e32003a, 0x70f30c14, 
		0x6686f33f, 0xd0d45ed8, 0x644fab57, 0x3a3fbbd3, 0x0b255fc4, 0x679a1701, 0x90e17b6e, 0x325d537b, 
		0xcd7b9b87, 0xaa7be2a2, 0x7d47c966, 0xa33dbce5, 0x8659c3bb, 0x72a41367, 0x15c446e0, 0x45fe8b0a, 
		0x9d8ddf26, 0x84d47643, 0x7fabe0da, 0x36a70122, 0x7a28ebfe, 0x7c29b8b8, 0x7f760406, 0xbabe4672, 
		0x23ea216e, 0x92bcc50a, 0x6d20dba2, 0xad5a7c7e, 0xbf3897f5, 0xabb793e1, 0x8391fc7e, 0xe270291c, 
		0x7a248d58, 0x80f8fd15, 0x83ef19f3, 0x5e6ece7d, 0x278430c1, 0x35239f4d, 0xe09c073b, 0x50e78cb5, 
		0xd4b811bd, 0xce834ee0, 0xf88aaa34, 0xf71da5a9, 0xe2b0a1d5, 0x7c3aef31, 0xe84eabca, 0x3ce25964, 
		0xf29336d3, 0x8fa78b2c, 0xa3fc3415, 0x63e1313d, 0x7fbc74e0, 0x7340bc93, 0x49ae583b, 0x8b79de4b, 
		0x25011ce9, 0x7b462279, 0x36007db0, 0x3da1599c, 0x77780772, 0xc845c9bb, 0x83ba68be, 0x6ee507d1, 
		0x2f0159b8, 0x5392c4ed, 0x98336ff6, 0x0b3c7f11, 0xde697aac, 0x893fc8d0, 0x6b83f8f3, 0x47799a0d, 
		0x801d9dfc, 0x8516a83e, 0x5f8d22ec, 0x0f8ba384, 0xa049dc4b, 0xdd920b05, 0x7a99bc9f, 0x9ad19344, 
		0x7a345dba, 0xf501a13f, 0x3e58bf19, 0x7fffaf9a, 0x3b4e1511, 0x0e08b991, 0x9e157620, 0x7230a326, 
		0x4977f9ff, 0x2d2bbae1, 0x607aa7fc, 0x7bc85d5f, 0xb441bbbe, 0x8d8fa5f2, 0x601cce26, 0xda1884f2, 
		0x81c82d64, 0x200b709c, 0xcbd36abe, 0x8cbdddd3, 0x55ab61d3, 0x7e3ee993, 0x833f18aa, 0xffc1aaea, 
		0x7362e16a, 0x7fb85db2, 0x904ee04c, 0x7f04dca6, 0x8ad7a046, 0xebe7d8f7, 0xfbc4c687, 0xd0609458, 
		0x093ed977, 0x8e546085, 0x7f5b8236, 0x7c47e118, 0xa01f2641, 0x7ffb3e48, 0x05de7cda, 0x7fc281b9, 
		0x8e0278fc, 0xd74e6d07, 0x94c24450, 0x7cf9e641, 0x2ad27871, 0x919fa815, 0x805fd205, 0x7758397f, 
		0xe2c7e02c, 0x1828e194, 0x5613d6fe, 0xfb55359f, 0xf9699516, 0x8978ee26, 0x7feebad9, 0x77d71d82, 
		0x55b28b60, 0x7e997600, 0x80821a6b, 0xc6d78af1, 0x691822ab, 0x7f6982a0, 0x7ef56f99, 0x5c307f40, 
		0xac6f8b76, 0x42cc8ba4, 0x782c61d9, 0xa0224dd0, 0x7bd234d1, 0x74576e3b, 0xe38cfe9a, 0x491e66ef, 
		0xc78291c5, 0x895bb87f, 0x924f7889, 0x71b89394, 0x757b779d, 0xc4a9c604, 0x5cdf7829, 0x8020e9df, 
		0x805e8245, 0x4a82c398, 0x6360bd62, 0x78bb60fc, 0x09e0d014, 0x4b0ea180, 0xb841978b, 0x69a0e864, 
		0x7df35977, 0x3284b0dd, 0x3cdc2efd, 0x57d31f5e, 0x541069cc, 0x1776e92e, 0x04309ea3, 0xa015eb2d, 
		0xce7bfabc, 0x41b638f8, 0x8365932e, 0x846ab44c, 0xbbcc80cb, 0x8afa6cac, 0x7fc422ea, 0x4e403fc0, 
		0xbfac9aee, 0x8e4c6709, 0x028e01fb, 0x6d160a9b, 0x7fe93004, 0x790f9cdc, 0x6a1f37a0, 0xf7e7ef30, 
		0xb4ea0f04, 0x7bf4c8e6, 0xe981701f, 0xc258a9d3, 0x6acbbfba, 0xef5479c7, 0x079c8bd8, 0x1a410f56, 
		0x6853b799, 0x86cd4f01, 0xc66e23b6, 0x34585565, 0x8d1fe00d, 0x7fcdba1a, 0x32c9717b, 0xa02f9f48, 
		0xf64940db, 0x5ed7d8f1, 0x61b823b2, 0x356f8918, 0xa0a7151e, 0x793fc969, 0x530beaeb, 0x34e93270, 
		0x4fc4ddb5, 0x88d58b6c, 0x36094774, 0xf620ac80, 0x03763a72, 0xf910c9a6, 0x6666fb2d, 0x752c8be8, 
		0x9a6dfdd8, 0xd1a7117d, 0x51c1b1d4, 0x0a67773d, 0x43b32a79, 0x4cdcd085, 0x5f067d30, 0x05bfe92a, 
		0x7ed7d203, 0xe71a3c85, 0x99127ce2, 0x8eb3cac4, 0xad4bbcea, 0x5c6a0fd0, 0x0eec04af, 0x94e95cd4, 
		0x8654f921, 0x83eabb5d, 0xb058d7ca, 0x69f12d3c, 0x03d881b2, 0x80558ef7, 0x82938cb3, 0x2ec0e1d6, 
		0x80044422, 0xd1e47051, 0x720fc6ff, 0x82b20316, 0x0d527b02, 0x63049a15, 0x7ad5b9ad, 0xd2a4641d, 
		0x41144f86, 0x7b04917a, 0x15c4a2c0, 0x9da07916, 0x211df54a, 0x7fdd09af, 0xfe924f3f, 0x7e132cfe, 
		0x9a1d18d6, 0x7c56508b, 0x80f0f0af, 0x8095ced6, 0x8037d0d7, 0x026719d1, 0xa55fec43, 0x2b1c7cb7, 
		0xa5cd5ac1, 0x77639fad, 0x7fcd8b62, 0x81a18c27, 0xaee4912e, 0xeae9eebe, 0xeb3081de, 0x8532aada, 
		0xc822362e, 0x86a649a9, 0x8031a71d, 0x7b319dc6, 0xea8022e6, 0x814bc5a9, 0x8f62f7a1, 0xa430ea17, 
		0x388deafb, 0x883b5185, 0x776fe13c, 0x801c683f, 0x87c11b98, 0xb7cbc644, 0x8e9ad3e8, 0x3cf5a10c, 
		0x7ff6a634, 0x949ef096, 0x9f84aa7c, 0x010af13f, 0x782d1de8, 0xf18e492a, 0x6cf63b01, 0x4301cd81, 
		0x32d15c9e, 0x68ad8cef, 0xd09bd2d6, 0x908c5c15, 0xd1e36260, 0x2c5bfdd0, 0x88765a99, 0x93deba1e, 
		0xac6ae342, 0xe865b84c, 0x0f4f2847, 0x7fdf0499, 0x78b1c9b3, 0x6a73261e, 0x601a96f6, 0xd2847933, 
		0x489aa888, 0xe12e8093, 0x3bfa5a5f, 0xd96ba5f7, 0x7c8f4c8d, 0x80940c6f, 0xcef9dd1a, 0x7e1a055f, 
		0x3483558b, 0x02b59cc4, 0x0c56333e, 0x05a5b813, 0x92d66287, 0x7516b679, 0x71bfe03f, 0x8056bf68, 
		0xc24d0724, 0x8416bcf3, 0x234afbdb, 0x4b0d6f9c, 0xaba97333, 0x4b4f42b6, 0x7e8343ab, 0x7ffe2603, 
		0xe590f73c, 0x45e10c76, 0xb07a6a78, 0xb35609d3, 0x1a027dfd, 0x90cb6e20, 0x82d3fe38, 0x7b409257, 
		0x0e395afa, 0x1b802093, 0xcb0c6c59, 0x241e17e7, 0x1ee3ea0a, 0x41a82302, 0xab04350a, 0xf570beb7, 
		0xbb444b9b, 0x83021459, 0x838d65dc, 0x1c439c84, 0x6fdcc454, 0xef9ef325, 0x18626c1c, 0x020d251f, 
		0xc4aae786, 0x8614cb48, 0xf6f53ca6, 0x8710dbab, 0x89abec0d, 0xf29d41c1, 0x94b50336, 0xfdd49178, 
		0x604658d1, 0x800e85be, 0xca1bb079, 0x7fa48eeb, 0xa3b7fafe, 0xd330436b, 0x64eb604c, 0x43a658ae, 
		0x7caa1337, 0xddd445e6, 0x7efbf955, 0xb706ec71, 0x624a6b53, 0x9e0e231f, 0x97097248, 0xa1e1a17a, 
		0x68dd2e44, 0x7f9d2e14, 0xddcc7074, 0x58324197, 0xc88fc426, 0x6d3640ae, 0x7ef83600, 0x759a0270, 
		0x98b6d854, 0xd63c9b84, 0x372474a2, 0xe3f18cfd, 0x56ab0bdb, 0x85c9be7e, 0x47dfcfeb, 0xa5830d41, 
		0x0ddd6283, 0xf4f480ad, 0x74c60e38, 0xab8943c3, 0xc1508fe7, 0x480cdc39, 0x8e097362, 0xa44793be, 
		0x538b7e18, 0x545f5b41, 0x56529175, 0x9771a97e, 0xc2da7421, 0xea8265f2, 0x805d1163, 0x883c5d28, 
		0x8ba94c48, 0x4f676e65, 0xf78735b3, 0xe1853671, 0x7f454f53, 0x18147f85, 0x7d09e15d, 0xdb4f3494, 
		0x795c8973, 0x83310632, 0x85d8061c, 0x9a1a0ebf, 0xc125583c, 0x2a1b1a95, 0x7fd9103f, 0x71e98c72, 
		0x40932ed7, 0x91ed227a, 0x3c5e560e, 0xe816dee9, 0xb0891b80, 0x600038ba, 0xc7d9a80d, 0x7fff5e09, 
		0x7e3f4351, 0xbb6b4424, 0xb14448d4, 0x8d6bb7e1, 0xfb153626, 0xa68ad537, 0xd9782006, 0xf62f6991, 
		0x359ba8c1, 0x02ccff0b, 0x91bf2256, 0x7ea71c4d, 0x560ce5df, 0xeeba289b, 0xa574c4e7, 0x9e04f6ee, 
		0x7860a5ec, 0x0b8db4a2, 0x968ba3d7, 0x0b6c77df, 0xd6f3157d, 0x402eff1a, 0x49b820b3, 0x8152aebb, 
		0xd180b0b6, 0x098604d4, 0x7ff92224, 0xede9c996, 0x89c58061, 0x829624c4, 0xc6e71ea7, 0xba94d915, 
		0x389c3cf6, 0x5b4c5a06, 0x04b335e6, 0x516a8aab, 0x42c8d7d9, 0x92b12af6, 0x86c8549f, 0xfda98acf, 
		0x819673b6, 0x69545dac, 0x6feaa230, 0x726e6d3f, 0x886ebdfe, 0x34f5730a, 0x7af63ba2, 0x77307bbf, 
		0x7cd80630, 0x6e45efe0, 0x7f8ad7eb, 0x59d7df99, 0x86c70946, 0xda233629, 0x753f6cbf, 0x825eeb40, 
};

#endif
