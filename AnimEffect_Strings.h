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

#pragma once

typedef enum {
	StrID_NONE, 
	StrID_Name,
	StrID_Description,
	// Parameters
	StrID_MODE,
	StrID_MODE_CHOICES,
	StrID_LINE,
	StrID_TOPIC_LINE,
	StrID_LINESTEP,
	StrID_LINEINTERVAL,
	StrID_LINELENGTH,
	StrID_LINEWIDTH,
	StrID_LINENOISE,
	StrID_MDARK,
	StrID_LINETYPE,
	StrID_LINETYPE_CHOICES,
	StrID_CANNYTH1,
	StrID_CANNYTH2,
	StrID_HATCHING,
	StrID_TOPIC_HATCHING,
	StrID_PATTERNNO,
	StrID_PATTERNNO_CHOICES,
	StrID_INCLINATION,
	StrID_INCRANGE,
	StrID_DENSEMAX,
	StrID_XRANGE,
	StrID_YRANGE,
	StrID_MAGNITUDE,
	StrID_ANGLE,
	StrID_SATURATION,
	StrID_BRIGHTNESS,
	StrID_VFLAG,
	StrID_RSEED,
	StrID_DIFFUSE,
	StrID_INVERT,
	StrID_INTERPOLATION,
	StrID_INTERPOLATION_CHOICES,
	StrID_FREQUENCY,
	StrID_MAKEMASK,
	StrID_NUMTYPES
} StrIDType;

extern void InitLocale(void);

