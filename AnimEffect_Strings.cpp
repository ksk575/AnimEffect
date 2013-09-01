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


/* Japanese Strings */
TableString		g_strs_ja[StrID_NUMTYPES] = {
	StrID_NONE,		"",
	StrID_Name,		"��`�����G�t�F�N�g",
	StrID_Description,	"AnimEffect\rCopyright 2013 kskP / original works by t-ogura and nmlP.",
	StrID_MODE,		"���[�h",
	StrID_MODE_CHOICES,		"1|2|3|4|5|6",
	StrID_LINE,		"�֊s",
	StrID_TOPIC_LINE,	"�֊s�p�����^",
	StrID_LINESTEP,		"Line Step",
	StrID_LINEINTERVAL,	"���o�Ԋu",
	StrID_LINELENGTH,	"Line Length",
	StrID_LINEWIDTH,	"Line Width",
	StrID_LINENOISE,	"Line Noise",
	StrID_MDARK,		"���Z�x",
	StrID_LINETYPE,		"Line Type",
	StrID_LINETYPE_CHOICES,		"NORMAL|Canny Gray|Canny RGB|Canny Gray RGB",
	StrID_CANNYTH1,		"CannyTh1",
	StrID_CANNYTH2,		"CannyTh2",
	StrID_HATCHING,		"�n�b�`���O",
	StrID_TOPIC_HATCHING,	"�n�b�`���O�p�����^",
	StrID_PATTERNNO,	"�p�^�[��No.",
	StrID_PATTERNNO_CHOICES,	"0|1|2|3",
	StrID_INCLINATION,	"�X��",
	StrID_INCRANGE,		"�X�ϓ���",
	StrID_DENSEMAX,		"�Z�x���",
	StrID_XRANGE,		"X�ϓ���",
	StrID_YRANGE,		"Y�ϓ���",
	StrID_MAGNITUDE,	"�g�嗦",
	StrID_ANGLE,		"��]�p�x",
	StrID_SATURATION,	"�ʓx�W��",
	StrID_BRIGHTNESS,	"���x�n��",
	StrID_VFLAG,		"VFlag",
	StrID_RSEED,		"�����V�[�h",
	StrID_DIFFUSE,		"�g�U",
	StrID_INVERT,		"�p�^�[�����]",
	StrID_INTERPOLATION,	"���",
	StrID_INTERPOLATION_CHOICES,	"NONE|Bi-Linear|Bi-Cubic",
	StrID_FREQUENCY,	"�X�V�Ԋu",
	StrID_MAKEMASK,		"�}�X�N�쐬",
};

/* English Strings */
TableString		g_strs_en[StrID_NUMTYPES] = {
	StrID_NONE,		"",
	StrID_Name,		"AnimEffect",
	StrID_Description,	"AnimEffect\rCopyright 2013 kskP / original works by t-ogura and nmlP.",
	StrID_MODE,		"Mode",
	StrID_MODE_CHOICES,		"1|2|3|4|5|6",
	StrID_LINE,		"Edge",
	StrID_TOPIC_LINE,	"Edge Parameters",
	StrID_LINESTEP,		"Line Step",
	StrID_LINEINTERVAL,	"Line Interval",
	StrID_LINELENGTH,	"Line Length",
	StrID_LINEWIDTH,	"Line Width",
	StrID_LINENOISE,	"Line Noise",
	StrID_MDARK,		"mDARK",
	StrID_LINETYPE,		"Line Type",
	StrID_LINETYPE_CHOICES,		"NORMAL|Canny Gray|Canny RGB|Canny Gray RGB",
	StrID_CANNYTH1,		"CannyTh1",
	StrID_CANNYTH2,		"CannyTh2",
	StrID_HATCHING,		"Hatching",
	StrID_TOPIC_HATCHING,	"Hatching Parameters",
	StrID_PATTERNNO,	"Pattern No.",
	StrID_PATTERNNO_CHOICES,	"0|1|2|3",
	StrID_INCLINATION,	"Inclination",
	StrID_INCRANGE,		"Inclination Range",
	StrID_DENSEMAX,		"Dense Max",
	StrID_XRANGE,		"X Range",
	StrID_YRANGE,		"Y Range",
	StrID_MAGNITUDE,	"Magnitude",
	StrID_ANGLE,		"Angle",
	StrID_SATURATION,	"Saturation",
	StrID_BRIGHTNESS,	"Brightness",
	StrID_VFLAG,		"VFlag",
	StrID_RSEED,		"Random Seed",
	StrID_DIFFUSE,		"Diffuse",
	StrID_INVERT,		"Invert",
	StrID_INTERPOLATION,	"Interpolation",
	StrID_INTERPOLATION_CHOICES,	"NONE|Bi-Linear|Bi-Cubic",
	StrID_FREQUENCY,	"Frequency",
	StrID_MAKEMASK,		"Make Mask",
};


static TableString *g_strs = g_strs_en;

void InitLocale(void)
{
	TextEncoding e = GetApplicationTextEncoding();
	if (e == kTextEncodingMacJapanese) {
	    g_strs = g_strs_ja;
	}
}

char	*GetStringPtr(int strNum)
{
	return g_strs[strNum].str;
}
	
