/*
 * AnimEffect plugin for After Effect / Premiere Pro CS6
 *
 * Copyright 2013 kskP / original works by t-ogura, nmlP
 *
 */

#pragma once

#ifndef ANIMEFFECT_H
#define ANIMEFFECT_H

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned short		u_int16;
typedef unsigned long		u_long;
typedef short int			int16;
#define PF_TABLE_BITS	12
#define PF_TABLE_SZ_16	4096

#include "AEConfig.h"

#ifdef AE_OS_WIN
	typedef unsigned short PixelType;
	#include <Windows.h>
#endif

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"

#include "AnimEffect_Strings.h"

#if 1 /* KSK */
#include "opencv/cv.h"
#include "opencv/highgui.h"

#define ANIMEFFECT_BUNDLE_ID "com.adobe.AfterEffects.ksk.AnimEffect"

#endif /* KSK */

/* Versioning information */

#define	MAJOR_VERSION	1
#define	MINOR_VERSION	0
#define	BUG_VERSION		0
#define	STAGE_VERSION	PF_Stage_DEVELOP
#define	BUILD_VERSION	1


/* Parameter defaults */

enum {
    PARAMID_INPUT = 0,
    PARAMID_MODE,
    PARAMID_LINESTEP,
    PARAMID_LINEINTERVAL,
    PARAMID_LINELENGTH,
    PARAMID_LINEWIDTH,
    PARAMID_LINENOISE,
    PARAMID_VFLAG,
    PARAMID_MDARK,
    PARAMID_PATTERNNO,
    PARAMID_INCLINATION,
    PARAMID_DENSEMAX,
    PARAMID_FREQUENCY,
    PARAMID_RSEED,
    PARAMID_INCRANGE,
    PARAMID_XRANGE,
    PARAMID_YRANGE,
    PARAMID_MAGNITUDE,
    PARAMID_ANGLE,
    PARAMID_SATURATION,
    PARAMID_BRIGHTNESS,
    PARAMID_LINETYPE,
    PARAMID_CANNYTH1,
    PARAMID_CANNYTH2,
    PARAMID_LINE,
    PARAMID_HATCHING,
    PARAMID_DIFFUSE,
    PARAMID_INVERT,
    PARAMID_INTERPOLATION,
    PARAMID_MAKEMASK,
    PARAMID_TOPIC_LINE,
    PARAMID_TOPIC_LINE_END,
    PARAMID_TOPIC_HATCHING,
    PARAMID_TOPIC_HATCHING_END,
    PARAMID_NUM_PARAMS
};

#define	ANIMEFFECT_GAIN_MIN		0
#define	ANIMEFFECT_GAIN_MAX		100
#define	ANIMEFFECT_GAIN_DFLT		10

enum {
	ANIMEFFECT_INPUT = 0,
	ANIMEFFECT_GAIN,
	ANIMEFFECT_COLOR,
	ANIMEFFECT_NUM_PARAMS
};

enum {
	GAIN_DISK_ID = 1,
	COLOR_DISK_ID,
};

typedef struct GainInfo{
	PF_FpLong	gainF;
} GainInfo, *GainInfoP, **GainInfoH;

#ifdef __cplusplus
	extern "C" {
#endif
	
DllExport	PF_Err 
EntryPointFunc(	
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra) ;

#ifdef __cplusplus
}
#endif

#endif // ANIMEFFECT_H
