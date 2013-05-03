/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007 Adobe Systems Incorporated                       */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

#include "AnimEffect.h"

typedef struct {
	A_u_long	index;
	A_char		str[256];
} TableString;


#if 0 /* Japanese Strings. Not ready yet; some string is not displayed correctly */
TableString		g_strs[StrID_NUMTYPES] = {
	StrID_NONE,		"",
	StrID_Name,		"手描き風エフェクト",
	StrID_Description,	"AnimEffect\rCopyright 2013 kskP / original works by t-ogura and nmlP.",
	StrID_MODE,		"モード",
	StrID_MODE_CHOICES,		"1|2|3|4|5|6",
	StrID_LINESTEP,		"Line Step",
	StrID_LINEINTERVAL,	"抽出間隔",
	StrID_LINELENGTH,	"Line Length",
	StrID_LINEWIDTH,	"Line Width",
	StrID_LINENOISE,	"Line Noise",
	StrID_VFLAG,		"VFlag",
	StrID_MDARK,		"線濃度",
	StrID_PATTERNNO,	"パターンNo.",
	StrID_PATTERNNO_CHOICES,	"0|1|2|3",
	StrID_INCLINATION,	"傾き",
	StrID_DENSEMAX,		"濃度上限",
	StrID_FREQUENCY,	"更新間隔",
	StrID_RSEED,		"乱数シード",
	StrID_INCRANGE,		"傾変動幅",
	StrID_XRANGE,		"X変動幅",
	StrID_YRANGE,		"Y変動幅",
	StrID_MAGNITUDE,	"拡大率",
	StrID_ANGLE,		"回転角度",
	StrID_SATURATION,	"彩度係数",
	StrID_BRIGHTNESS,	"明度系数",
	StrID_LINETYPE,		"Line Type",
	StrID_LINETYPE_CHOICES,		"NORMAL|Canny Gray|Canny RGB|Canny Gray RGB",
	StrID_CANNYTH1,		"CannyTh1",
	StrID_CANNYTH2,		"CannyTh2",
	StrID_LINE,		"輪郭",
	StrID_HATCHING,		"ハッチング",
	StrID_DIFFUSE,		"拡散",
	StrID_INVERT,		"ハッチングパターン反転",
	StrID_INTERPOLATION,	"ハッチング補間",
	StrID_INTERPOLATION_CHOICES,	"NONE|Bi-Linear|Bi-Cubic",
	StrID_MAKEMASK,		"マスク作成",
	StrID_TOPIC_LINE,	"輪郭パラメタ",
	StrID_TOPIC_HATCHING,	"ハッチングパラメタ",
};
#endif /* Japanese Strings */

#if 1 /* English description */
TableString		g_strs[StrID_NUMTYPES] = {
	StrID_NONE,		"",
	StrID_Name,		"AnimEffect",
	StrID_Description,	"AnimEffect\rCopyright 2013 kskP / original works by t-ogura and nmlP.",
	StrID_MODE,		"Mode",
	StrID_MODE_CHOICES,		"1|2|3|4|5|6",
	StrID_LINESTEP,		"Line Step",
	StrID_LINEINTERVAL,	"Line Interval",
	StrID_LINELENGTH,	"Line Length",
	StrID_LINEWIDTH,	"Line Width",
	StrID_LINENOISE,	"Line Noise",
	StrID_VFLAG,		"VFlag",
	StrID_MDARK,		"mDARK",
	StrID_PATTERNNO,	"Pattern No.",
	StrID_PATTERNNO_CHOICES,	"0|1|2|3",
	StrID_INCLINATION,	"Inclination",
	StrID_DENSEMAX,		"Dense Max",
	StrID_FREQUENCY,	"Frequency",
	StrID_RSEED,		"Random Seed",
	StrID_INCRANGE,		"Inclination Range",
	StrID_XRANGE,		"X Range",
	StrID_YRANGE,		"Y Range",
	StrID_MAGNITUDE,	"Magnitude",
	StrID_ANGLE,		"Angle",
	StrID_SATURATION,	"Saturation",
	StrID_BRIGHTNESS,	"Brightness",
	StrID_LINETYPE,		"Line Type",
	StrID_LINETYPE_CHOICES,		"NORMAL|Canny Gray|Canny RGB|Canny Gray RGB",
	StrID_CANNYTH1,		"CannyTh1",
	StrID_CANNYTH2,		"CannyTh2",
	StrID_LINE,		"Edge",
	StrID_HATCHING,		"Hatching",
	StrID_DIFFUSE,		"Diffuse",
	StrID_INVERT,		"Invert",
	StrID_INTERPOLATION,	"Hatching Interpolation",
	StrID_INTERPOLATION_CHOICES,	"NONE|Bi-Linear|Bi-Cubic",
	StrID_MAKEMASK,		"Make Mask",
	StrID_TOPIC_LINE,	"Edge Parameters",
	StrID_TOPIC_HATCHING,	"Hatching Parameters",
};
#endif  /* English description; not used */

char	*GetStringPtr(int strNum)
{
	return g_strs[strNum].str;
}
	
