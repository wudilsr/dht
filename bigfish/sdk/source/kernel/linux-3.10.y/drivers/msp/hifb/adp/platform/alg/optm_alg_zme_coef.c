#include "optm_alg_zme_coef.h"
//=====================Video ZME Coefficient START=================//
#ifndef HI_BUILD_IN_BOOT
HI_S16 OPTM_s16ZmeCoef_8T32P_Cubic[18][8] = {{ 0,  0,0, 511,   0,   0,  0,  0}, {-1,  3, -12, 511,  14,-4,  1,  0},
{-2,  6, -23, 509,  28,  -8,  2,  0}, {-2,  9, -33, 503,  44, -12,  3,  0}, {-3, 11,-41, 496,61, -16,4,  0},
{-3, 13, -48, 488,  79, -21,  5, -1}, {-3, 14, -54, 477,  98, -25,  7, -2}, {-4, 16,-59, 465,118, -30,8, -2},
{-4, 17, -63, 451, 138, -35,  9, -1}, {-4, 18, -66, 437, 158, -39, 10, -2}, {-4, 18,-68, 421,180, -44,11, -2},
{-4, 18, -69, 404, 201, -48, 13, -3}, {-4, 18, -70, 386, 222, -52, 14, -2}, {-4, 18,-70, 368,244, -56,15, -3},
{-4, 18, -69, 348, 265, -59, 16, -3}, {-4, 18, -67, 329, 286, -63, 16, -3}, {-3, 17,-65, 307,307, -65,17, -3},
{ 0,  0,0,   0,   0,   0,  0,  0}};//normalized ok

HI_S16 OPTM_s16ZmeCoef_8T32P_Lanc3[18][8] ={{4,-22,40,468,40,-22,4,0},{3,-18,26,468,54,-26,5,0},{2,-14,14,466,68,-30,6,0},
{2,-11,2,462,84,-34,7,0},{1,-7,-9,457,100,-38,8,0},{1,-4,-18,450,116,-42,9,0},
{1,-2,-27,443,133,-46,10,0},{0,2,-35,434,151,-50,10,0},{0,4,-42,425,168,-54,11,0},
{0,6,-49,414,186,-57,12,0},{0,8,-54,401,204,-60,13,0},{0,10,-58,387,222,-62,13,0},
{0,11,-62,374,240,-65,14,0},{0,12,-65,359,258,-66,14,0},{0,13,-67,344,276,-68,14,0},
{0,14,-68,327,293,-68,14,0},{0,14,-68,310,310,-68,14,0},{0,0,0,0,0,0,0,0}};//normalized ok


HI_S16 OPTM_s16ZmeCoef_8T32P_Lanc2[18][8] = {{-16,0,145,254,145,0,-16,0},{-16,-2,140,253,151,3,-17,0},
{-15,-5,135,253,157,5,-18,0},{-14,-7,129,252,162,8,-18,0},{-13,-9,123,252,167,11,-19,0},
{-13,-11,118,250,172,15,-19,0},{-12,-12,112,250,177,18,-20,-1},{-11,-14,107,247,183,21,-20,-1},
{-10,-15,101,245,188,25,-21,-1},{-9,-16,96,243,192,29,-21,-2},{-8,-18,90,242,197,33,-22,-2},
{-8,-19,85,239,202,37,-22,-2},{-7,-19,80,236,206,41,-22,-3},{-7,-20,75,233,210,46,-22,-3},
{-6,-21,69,230,215,50,-22,-3},{-5,-21,65,226,219,55,-22,-5},{-5,-21,60,222,222,60,-21,-5},
{0,0,0,0,0,0,0,0}};//normalized ok

//new coefficients
//---------------8 tap----------------------------
//2.426M
HI_S16 OPTM_s16ZmeCoef_8T32P_3M_a19[18][8] = {{-18,18,144,226,144,19,-17,-4},{-17,16,139,226,148,21,-17,-4},
{-17,13,135,227,153,24,-18,-5},{-17,11,131,226,157,27,-18,-5},{-17,9,126,225,161,30,-17,-5},
{-16,6,122,225,165,33,-17,-6},{-16,4,118,224,169,37,-17,-7},{-16,2,113,224,173,40,-17,-7},
{-15,0,109,222,177,43,-17,-7},{-15,-1,104,220,181,47,-16,-8},{-14,-3,100,218,185,51,-16,-9},
{-14,-5,96,217,188,54,-15,-9},{-14,-6,91,214,192,58,-14,-9},{-13,-7,87,212,195,62,-14,-10},
{-13,-9,83,210,198,66,-13,-10},{-12,-10,79,207,201,70,-12,-11},{-12,-11,74,205,205,74,-11,-12},
{0,0,0,0,0,0,0,0}};//normalized ok

//---------------6 tap----------------------------
HI_S16 OPTM_s16ZmeCoef_6T32P_Cubic[18][6] = {{0,0,511,0,0,0},{3,-12,511,13,-3,0},{6,-22,507,28,-7,0},
{8,-32,502,44,-11,1},{10,-40,495,61,-15,1},{11,-47,486,79,-19,2},{12,-53,476,98,-24,3},
{13,-58,464,117,-28,4},{14,-62,451,137,-33,5},{15,-65,437,157,-38,6},{15,-67,420,179,-42,7},
{15,-68,404,200,-46,7},{14,-68,386,221,-50,9},{14,-68,367,243,-54,10},{14,-67,348,264,-58,11},
{13,-66,328,286,-61,12},{13,-63,306,306,-63,13},{0,0,0,0,0,0}};//normalized ok

//4.667M
HI_S16 OPTM_s16ZmeCoef_6T32P_6M_a25[18][6] = {{-34,78,415,78,-34,9},{-32,66,415,90,-36,9},
{-29,54,413,103,-38,9},{-26,43,411,116,-41,9},{-23,33,406,129,-42,9},{-21,24,401,143,-44,9},
{-18,14,396,157,-46,9},{-16,6,389,171,-47,9},{-14,-2,382,185,-48,9},{-11,-9,374,199,-49,8},
{-9,-16,365,213,-49,8},{-7,-21,354,228,-49,7},{-5,-27,345,242,-49,6},{-3,-32,333,256,-48,6},
{-1,-36,322,269,-46,4},{0,-39,309,283,-44,3},{2,-42,296,296,-42,2},{0,0,0,0,0,0}};//normalized ok
//3.888M
HI_S16 OPTM_s16ZmeCoef_6T32P_5M_a25[18][6] = {{-31,104,362,104,-31,4},{-30,94,362,114,-32,4},
{-29,84,361,125,-32,3},{-28,75,359,136,-33,3},{-27,66,356,147,-33,3},{-25,57,353,158,-33,2},
{-24,49,349,169,-33,2},{-22,41,344,180,-32,1},{-20,33,339,191,-31,0},{-19,26,333,203,-30,-1},
{-17,19,327,214,-29,-2},{-16,13,320,225,-27,-3},{-14,7,312,236,-25,-4},{-13,1,305,246,-22,-5},
{-11,-4,295,257,-19,-6},{-10,-8,286,267,-16,-7},{-9,-12,277,277,-12,-9},{0,0,0,0,0,0}};//normalized ok
//3.216M
HI_S16 OPTM_s16ZmeCoef_6T32P_4M_a20[18][6] = {{-20,130,297,130,-20,-5},{-21,122,298,138,-19,-6},
{-22,115,297,146,-17,-7},{-22,108,296,153,-16,-7},{-23,101,295,161,-14,-8},{-23,93,294,169,-12,-9},
{-24,87,292,177,-10,-10},{-24,80,289,185,-7,-11},{-24,73,286,193,-4,-12},{-23,66,283,200,-1,-13},
{-23,60,279,208,2,-14},{-23,54,276,215,5,-15},{-22,48,271,222,9,-16},{-21,42,266,229,13,-17},
{-21,37,261,236,17,-18},{-21,32,255,242,22,-18},{-20,27,249,249,27,-20},{0,0,0,0,0,0}};//normalized ok
//2.16M
HI_S16 OPTM_s16ZmeCoef_6T32P_3M_a15[18][6] = {{16,136,217,136,16,-9},{13,132,217,141,18,-9},
{11,128,217,145,21,-10},{9,124,216,149,24,-10},{7,119,216,153,27,-10},{5,115,216,157,30,-11},
{3,111,215,161,33,-11},{1,107,214,165,36,-11},{0,102,213,169,39,-11},{-2,98,211,173,43,-11},
{-3,94,209,177,46,-11},{-4,90,207,180,50,-11},{-5,85,206,184,53,-11},{-6,81,203,187,57,-10},
{-7,77,201,190,61,-10},{-8,73,198,193,65,-9},{-9,69,196,196,69,-9},{0,0,0,0,0,0}};//normalized ok

//---------------4 tap----------------------------
HI_S16 OPTM_s16ZmeCoef_4T32P_Cubic[18][4] = {{0, 511,0,0}, {-19, 511,  21,	-1}, {-37, 509,  42,  -2}, {-51, 504,  64,-5},
{-64, 499,  86,  -9}, {-74, 492, 108, -14}, {-82, 484, 129, -19}, {-89, 474, 152,-25}, {-94, 463,174, -31},
{-97, 451, 196, -38}, {-98, 438, 217, -45}, {-98, 424, 238, -52}, {-98, 409, 260,-59}, {-95, 392,280, -65},
{-92, 376, 300, -72}, {-88, 358, 320, -78}, {-83, 339, 339, -83}, {  0,   0,	0,   0}};//normalized ok

//3.507M
HI_S16 OPTM_s16ZmeCoef_4T32P_5M_a15[18][4] = {{103,335,103,-29},{92,335,112,-27},{84,335,121,-28},
{75,334,131,-28},{67,332,141,-28},{59,329,152,-28},{51,326,162,-27},{43,323,173,-27},{36,319,183,-26},
{30,313,194,-25},{23,308,204,-23},{17,301,215,-21},{12,295,225,-20},{6,288,235,-17},{2,280,244,-14},
{-3,271,254,-10},{-7,263,263,-7},{0,0,0,0}};//normalized ok
//2.657M
HI_S16 OPTM_s16ZmeCoef_4T32P_4M_a15[18][4] = {{120,281,120,-9},{113,281,127,-9},{106,280,134,-8},
{99,279,141,-7},{92,277,148,-5},{85,275,156,-4},{79,273,162,-2},{72,270,170,0},{66,267,177,2},
{61,263,184,4},{56,259,191,6},{50,255,198,9},{44,251,205,12},{40,246,211,15},{34,241,218,19},
{31,235,224,22},{26,230,230,26},{0,0,0,0}};//normalized ok


//---------------2 tap----------------------------
HI_S16 OPTM_s16ZmeCoef_2T32P_Cubic[18][2] = {{511,0}, {511, 1}, {506,6}, {499,13}, {490,22},
{ 478,  34}, { 465,  47}, { 449,  63}, { 432,  80}, { 413,  99}, { 393,119}, { 372, 140},
{ 350, 162}, { 328, 184}, { 304, 208}, { 280, 232}, { 256, 256}, {0,0}};//normalized ok

//3.717M
HI_S16 OPTM_s16ZmeCoef_2T32P_6M_1_3[18][2] = {{420,92},{412,100},{404,108},{395,117},{386,126},
{376,136},{366,146},{356,156},{346,166},{335,177},{325,187},{313,199},{302,210},
{291,221},{279,233},{268,244},{256,256},{0,0}};//normalized ok
//3.56M
HI_S16 OPTM_s16ZmeCoef_2T32P_5M_1_0[18][2] = {{397,115},{389,123},{381,131},{373,139},{365,147},
{356,156},{347,165},{339,173},{330,182},{321,191},{312,200},{303,209},{293,219},
{284,228},{275,237},{265,247},{256,256},{0,0}};//normalized ok
//3.39M
HI_S16 OPTM_s16ZmeCoef_2T32P_4M_0_1[18][2] = {{366,146},{359,153},{351,161},{344,168},{337,175},
{330,182},{323,189},{316,196},{309,203},{302,210},{296,216},{289,223},{282,230},
{276,236},{269,243},{263,249},{256,256},{0,0}};//normalized ok

HI_S16 OPTM_s16ZmeCoef_2T32P_5M[18][2] = {{ 386, 126}, { 378, 134}, { 370, 142}, { 362, 150}, { 355, 157},
{ 347, 165}, { 338, 174}, { 330, 182}, { 322, 190}, { 314, 198}, { 306, 206}, { 298,214},
{ 289, 223}, { 281, 231}, { 273, 239}, { 264, 248}, { 256, 256}, {0,0}};//normalized ok


//5.0625M
HI_S16 OPTM_s16ZmeCoef_2T32P_Gus2_6_75M_a0_5[18][2] ={
{511,0},{498,14},{483,29},{468,44},{453,59},{437,75},
{422,90},{406,106},{390,122},{373,139},{357,155},{340,172},{324,188},{307,205},{290,222},{273,239},{256,256},{0,0}};

//4.509M
HI_S16 OPTM_s16ZmeCoef_2T32P_Gus2_6M_a0_5[18][2] ={{462,50},{449,63},{437,75},{424,88},{411,101},{398,114},
{386,126},{373,139},{360,152},{347,165},{334,178},{321,191},{308,204},{295,217},{282,230},{269,243},{256,256},{0,0}};
#endif
//=====================Video ZME Coefficient END=================//




//=====================Graphic ZME Coefficient START=================//
HI_S16 OPTM_s16ZmeCoef_8T8P_Cubic[8][8] = {
{0, 0, 0, 511, 0, 0, 0, 0, }, {-3, 11, -41, 496, 61, -16, 4, 0, },
{-4, 17, -63, 451, 138, -35, 9, -1, },{-4, 18, -70, 386, 222, -52, 14, -2, },
{-3, 17, -65, 307, 307, -65, 17, -3, },{-2, 14, -52, 222, 386, -70, 18, -4, },
{-1, 9, -35, 138, 451, -63, 17, -4, },{0, 4, -16, 61, 496, -41, 11, -3, }};//normalized ok

HI_S16 OPTM_s16ZmeCoef_8T8P_Lanc3[8][8] = {
{4,-22,40,468,40,-22,4,0},{1,-7,-9,457,100,-38,8,0},
{0,4,-42,425,168,-54,11,0},{0,11,-62,374,240,-65,14,0},
{0,14,-68,310,310,-68,14,0},{0,14,-65,240,374,-62,11,0},
{0,11,-54,168,425,-42,4,0},{0,8,-38,100,457,-9,-7,1}};	 //normalized ok   

HI_S16 OPTM_s16ZmeCoef_8T8P_Lanc2[8][8] = {
{-16,0,145,254,145,0,-16,0},{-13,-9,123,252,167,11,-19,0},
{-10,-15,101,245,188,25,-21,-1},{-7,-19,80,236,206,41,-22,-3},
{-5,-21,60,222,222,60,-21,-5},{-3,-22,41,206,236,80,-19,-7},
{-1,-21,25,188,245,101,-15,-10},{0,-19,11,167,252,123,-9,-13}};//normalized ok

//2.426M
HI_S16 OPTM_s16ZmeCoef_8T8P_3M_a19[8][8] = {
{-18,19,144,226,144,19,-18,-4},{-17,9,126,225,161,30,-17,-5},
{-15,0,109,222,177,43,-17,-7},{-14,-6,91,214,192,58,-14,-9},
{-12,-11,74,205,205,74,-11,-12},{-9,-14,58,192,214,91,-6,-14},
{-7,-17,43,177,222,109,0,-15},{-5,-17,30,161,225,126,9,-17}};//normalized ok


HI_S16 OPTM_s16ZmeCoef_4T16P_Cubic[16][4] ={
{0, 511, 0, 0, },{-37, 509, 42, -2, },{-64, 499, 86, -9, },{-82, 484, 129, -19, },
{-94, 463, 174, -31, },{-98, 438, 217, -45, },{-98, 409, 260, -59, },{-92, 376, 300, -72, },
{-83, 339, 339, -83, },{-72, 300, 376, -92, },{-59, 260, 409, -98, },{-45, 217, 438, -98, },
{-31, 174, 463, -94, },{-19, 129, 484, -82, },{-9, 86, 499, -64, },{-2, 42, 509, -37, }};//normalized ok

//4tap lanczos2 + LPF
//4.27M
HI_S16 OPTM_s16ZmeCoef_4T16P_Lanc2_6M_a15[16][4] = {
{79,383,79,-29},{58,384,102,-32},{40,380,127,-35},{23,374,153,-38},
{8,363,180,-39},{-6,349,208,-39},{-16,331,235,-38},{-25,311,262,-36},
{-31,287,287,-31},{-36,262,311,-25},{-38,235,331,-16},{-39,208,349,-6},
{-39,180,363,8},{-38,153,374,23},{-35,127,380,40},{-32,102,384,58}};//normalized ok
//3.35M
HI_S16 OPTM_s16ZmeCoef_4T16P_Lanc2_5M_a15[16][4] = {
{103,328,103,-22},{85,328,121,-22},{69,324,141,-22},{53,319,161,-21},
{40,311,181,-20},{27,301,201,-17},{16,288,221,-13},{7,273,240,-8},
{-2,258,258,-2},{-8,240,273,7},{-13,221,288,16},{-17,201,301,27},
{-20,181,311,40},{-21,161,319,53},{-22,141,324,69},{-22,121,328,85}};//normalized ok
//2.9M
HI_S16 OPTM_s16ZmeCoef_4T16P_Lanc2_4_5M_a15[16][4] = {
{113,301,113,-15},{97,300,129,-14},{82,297,145,-12},{68,292,162,-10},
{55,285,179,-7},{43,277,196,-4},{32,267,212,1},{23,255,227,7},
{14,242,242,14},{7,227,255,23},{1,212,267,32},{-4,196,277,43},
{-7,179,285,55},{-10,162,292,68},{-12,145,297,82},{-14,129,300,97}};//normalized ok
//2.58M
HI_S16 OPTM_s16ZmeCoef_4T16P_Lanc2_4M_a15[16][4] = {{121,276,121,-6},{107,275,134,-4},
{93,272,148,-1},{81,267,162,2},{68,262,176,6},{58,255,189,10},{48,246,202,16},
{38,237,214,23},{30,226,226,30},{23,214,237,38},{16,202,246,48},{10,189,255,58},
{6,176,262,68},{2,162,267,81},{-1,148,272,93},{-4,134,275,107}};//normalized ok
//2.11
HI_S16 OPTM_s16ZmeCoef_4T16P_Lanc2_3M_a13[16][4] = {{135,227,135,15},{124,226,143,19},
{115,222,151,24},{105,219,159,29},{95,215,167,35},{86,211,174,41},{78,206,181,47},
{69,201,188,54},{62,194,194,62},{54,188,201,69},{47,181,206,78},{41,174,211,86},
{35,167,215,95},{29,159,219,105},{24,151,222,115},{19,143,226,124}};//normalized ok


//new coefficients
//4.456M
HI_S16 OPTM_s16ZmeCoef_4T16P_6M_a15[16][4] = {{78,392,78,-36},{56,393,102,-39},
{36,391,128,-43},{18,384,155,-45},{2,373,184,-47},{-12,358,213,-47},{-23,340,241,-46},
{-32,319,269,-44},{-39,295,295,-39},{-44,269,319,-32},{-46,241,340,-23},{-47,213,358,-12},
{-47,184,373,2},{-45,155,384,18},{-43,128,391,36},{-39,102,393,56}};//normalized ok

//2.138M
HI_S16 OPTM_s16ZmeCoef_4T16P_3M_a13[16][4] = {{135,230,135,12},{124,228,143,17},
{114,225,152,21},{104,222,160,26},{94,218,168,32},{85,214,176,37},{76,208,183,45},
{67,203,190,52},{59,197,197,59},{52,190,203,67},{45,183,208,76},{37,176,214,85},
{32,168,218,94},{26,160,222,104},{21,152,225,114},{17,143,228,124}};//normalized ok

//5.0625M
HI_S16 OPTM_s16ZmeCoef_2T8P_Gus2_6_75M_a0_5[8][2] ={
{511,0},{453,59}, {390,122},{324,188},{256,256},{188,324},{122,390},{59,453}};

//3.39M
HI_S16 OPTM_s16ZmeCoef_2T8P_4M_0_1[8][2] = {
{366,146},{337,175},{309,203},{282,230},{256,256},{175,337},{203,309},{230,282},};//normalized ok


HI_S16 OPTM_s16ZmeCoef_2T16P_Gus2_6_75M_a0_5[16][2] ={
{511,0},  {483,29}, {453,59}, {422,90}, 
{390,122},{357,155},{324,188},{290,222},
{256,256},{222,290},{188,324},{155,357},
{122,390},{90,422}, {59,453}, {29,483} };


HI_S16 OPTM_s16ZmeCoef_2T16P_Gus2_6M_a0_5[16][2] ={
{462,50}, {437,75}, {411,101},{386,126},
{360,152},{334,178},{308,204},{282,230},
{256,256},{230,282},{204,308},{178,334},
{152,360},{126,386},{101,411},{75,437} };


HI_S16 OPTM_s16ZmeCoef_2T16P_4M_0_1[16][2] = {
{366,146},{351,161},{337,175},{323,189},
{309,203},{296,216},{282,230},{269,243},
{256,256},{161,351},{175,337},{189,323},
{203,309},{216,296},{230,282},{243,269},};//normalized ok



/**================================= begin ==================================**/
/**
 **and from HiFoneB2 4K TinyZME, 2T16P align with 0 by d00241380
 **/
HI_S16 OPTM_s16TinyZmeCoef_2T16P_Gus2_6_75M_a0_5[16][4] ={
	{ 0, 511,	0, 0},{ 0, 483,  29, 0},{ 0, 453,  59, 0},{ 0, 422,  90, 0}, 
	{ 0, 390, 122, 0},{ 0, 357, 155, 0},{ 0, 324, 188, 0},{ 0, 290, 222, 0},
	{ 0, 256, 256, 0},{ 0, 222, 290, 0},{ 0, 188, 324, 0},{ 0, 155, 357, 0},
	{ 0, 122, 390, 0},{ 0,	90, 422, 0},{ 0,  59, 453, 0},{ 0,	29, 483, 0} 
	};

HI_S16 OPTM_s16TinyZmeCoef_2T16P_Gus2_6M_a0_5[16][4] ={
	{ 0, 462,  50, 0},{ 0, 437,  75, 0},{ 0, 411, 101, 0},{ 0, 386, 126, 0},
	{ 0, 360, 152, 0},{ 0, 334, 178, 0},{ 0, 308, 204, 0},{ 0, 282, 230, 0},
	{ 0, 256, 256, 0},{ 0, 230, 282, 0},{ 0, 204, 308, 0},{ 0, 178, 334, 0},
	{ 0, 152, 360, 0},{ 0, 126, 386, 0},{ 0, 101, 411, 0},{ 0,	75, 437, 0}
	};

HI_S16 OPTM_s16TinyZmeCoef_2T16P_4M_0_1[16][4] = {
	{ 0, 366, 146, 0},{ 0, 351, 161, 0},{ 0, 337, 175, 0},{ 0, 323, 189, 0},
	{ 0, 309, 203, 0},{ 0, 296, 216, 0},{ 0, 282, 230, 0},{ 0, 269, 243, 0},
	{ 0, 256, 256, 0},{ 0, 161, 351, 0},{ 0, 175, 337, 0},{ 0, 189, 323, 0},
	{ 0, 203, 309, 0},{ 0, 216, 296, 0},{ 0, 230, 282, 0},{ 0, 243, 269, 0}
};
/**================================== end ====================================**/


//=====================Graphic ZME Coefficient END=================//


