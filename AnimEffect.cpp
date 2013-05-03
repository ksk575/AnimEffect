/*
 * AnimEffect plugin for After Effect / Premiere Pro CS6
 *
 * Copyright 2013 kskP / original works by t-ogura, nmlP
 *
 */

#include "AnimEffect.h"
#include "AnimEffect_Func.h"

static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	
	suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
											"%s v%d.%d\r%s",
											STR(StrID_Name), 
											MAJOR_VERSION, 
											MINOR_VERSION, 
											STR(StrID_Description));
	return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	/* Init AnimEffect structures */
	CFBundleRef pluginBundle = CFBundleGetBundleWithIdentifier(CFSTR(ANIMEFFECT_BUNDLE_ID));
	CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(pluginBundle);
	char bundlePath[PATH_MAX];
	if (!CFURLGetFileSystemRepresentation(resourcesURL, true, (UInt8 *)bundlePath, sizeof(bundlePath)))
	{
		PF_SPRINTF(out_data->return_msg, "Could not get bundlePath for %s", ANIMEFFECT_BUNDLE_ID);
		return PF_Err_INTERNAL_STRUCT_DAMAGED;
	}
	InitAnimEffect(bundlePath);

	/* Setup OutFlags for AE plugin */
	out_data->my_version = PF_VERSION(	MAJOR_VERSION, 
										MINOR_VERSION,
										BUG_VERSION, 
										STAGE_VERSION, 
										BUILD_VERSION);

	out_data->out_flags = PF_OutFlag_NONE
	    | PF_OutFlag_WIDE_TIME_INPUT		// 0x00000002
	    | PF_OutFlag_NON_PARAM_VARY			// 0x00000004
//	    | PF_OutFlag_DISPLAY_ERROR_MESSAGE	// 0x00000100
	    ;

	out_data->out_flags2 = PF_OutFlag2_NONE
	    | PF_OutFlag2_PPRO_DO_NOT_CLONE_SEQUENCE_DATA_FOR_RENDER // 0x00008000
	    | PF_OutFlag2_AUTOMATIC_WIDE_TIME_INPUT // 0x00020000
	    ;
	return PF_Err_NONE;
}

static PF_Err 
GlobalSetdown(PF_InData *in_data)
{
	ExitAnimEffect();
	return PF_Err_NONE;
}

static PF_Err 
ParamsSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err		err		= PF_Err_NONE;
	PF_ParamDef	def;	

	AEFX_CLR_STRUCT(def);
	PF_ADD_POPUP(	STR(StrID_MODE), 6, 1, STR(StrID_MODE_CHOICES),
					PARAMID_MODE);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_LINESTEP), 1, 100, 1, 100, 5,
							PF_Precision_INTEGER, 0, 0,
							PARAMID_LINESTEP);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_LINEINTERVAL), 1, 100, 1, 100, 54,
							PF_Precision_INTEGER, 0, 0,
							PARAMID_LINEINTERVAL);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_LINELENGTH), -1000, 1000, -1000, 1000, 0,
							PF_Precision_INTEGER, 0, 0,
							PARAMID_LINELENGTH);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_LINEWIDTH), -100, 100, -100, 100, 1,
							PF_Precision_INTEGER, 0, 0,
							PARAMID_LINEWIDTH);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_LINENOISE), 0, 1000, 0, 1000, 15,
							PF_Precision_INTEGER, 0, 0,
							PARAMID_LINENOISE);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_VFLAG), -50, 9, -50, 9, 0,
							PF_Precision_INTEGER, 0, 0,
							PARAMID_VFLAG);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_MDARK), 0, 2.0, 0, 2.0, 1.0,
							PF_Precision_HUNDREDTHS, 0, 0,
							PARAMID_MDARK);

	AEFX_CLR_STRUCT(def);
	PF_ADD_POPUP(	STR(StrID_PATTERNNO), 4, 1, STR(StrID_PATTERNNO_CHOICES),
					PARAMID_PATTERNNO);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_INCLINATION), -5.0, 5.0, -5.0, 5.0, -1.0,
							PF_Precision_HUNDREDTHS, 0, 0,
							PARAMID_INCLINATION);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_DENSEMAX), -1.0, 1.0, -1.0, 1.0, 1.0,
							PF_Precision_HUNDREDTHS, 0, 0,
							PARAMID_DENSEMAX);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_FREQUENCY), -100, 100, -100, 100, 1,
							PF_Precision_INTEGER, 0, 0,
							PARAMID_FREQUENCY);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_RSEED), 0, 65535, 0, 65535, 0,
							PF_Precision_INTEGER, 0, 0,
							PARAMID_RSEED);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_INCRANGE), 0, 2.0, 0, 2.0, 1.0,
							PF_Precision_HUNDREDTHS, 0, 0,
							PARAMID_INCRANGE);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_XRANGE), 0, 10000, 0, 10000, 10000,
							PF_Precision_INTEGER, 0, 0,
							PARAMID_XRANGE);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_YRANGE), 0, 10000, 0, 10000, 10000,
							PF_Precision_INTEGER, 0, 0,
							PARAMID_YRANGE);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_MAGNITUDE), 0.01, 10, 0.01, 10, 1,
							PF_Precision_HUNDREDTHS, 0, 0,
							PARAMID_MAGNITUDE);

	AEFX_CLR_STRUCT(def);
	PF_ADD_ANGLE(	STR(StrID_ANGLE), 0, PARAMID_ANGLE);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_SATURATION), 0, 50, 0, 50, 1,
							PF_Precision_HUNDREDTHS, 0, 0,
							PARAMID_SATURATION);
	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_BRIGHTNESS), 0, 50, 0, 50, 2,
							PF_Precision_HUNDREDTHS, 0, 0,
							PARAMID_BRIGHTNESS);

	AEFX_CLR_STRUCT(def);
	PF_ADD_POPUP(	STR(StrID_LINETYPE), 4, 1, STR(StrID_LINETYPE_CHOICES),
					PARAMID_LINETYPE);

	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_CANNYTH1), 0, 10000, 0, 10000, 2000,
							PF_Precision_INTEGER, 0, 0,
							PARAMID_CANNYTH1);
	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(	STR(StrID_CANNYTH2), 0, 10000, 0, 10000, 3000,
							PF_Precision_INTEGER, 0, 0,
							PARAMID_CANNYTH2);
	AEFX_CLR_STRUCT(def);
	PF_ADD_CHECKBOXX(	STR(StrID_LINE), TRUE, 0, PARAMID_LINE);

	AEFX_CLR_STRUCT(def);
	PF_ADD_CHECKBOXX(	STR(StrID_HATCHING), TRUE, 0, PARAMID_HATCHING);

	AEFX_CLR_STRUCT(def);
	PF_ADD_CHECKBOXX(	STR(StrID_DIFFUSE), FALSE, 0, PARAMID_DIFFUSE);

	AEFX_CLR_STRUCT(def);
	PF_ADD_CHECKBOXX(	STR(StrID_INVERT), FALSE, 0, PARAMID_INVERT);

	AEFX_CLR_STRUCT(def);
	PF_ADD_POPUP(	STR(StrID_INTERPOLATION), 3, 1, STR(StrID_INTERPOLATION_CHOICES),
					PARAMID_INTERPOLATION);

	AEFX_CLR_STRUCT(def);
	PF_ADD_CHECKBOXX(	STR(StrID_MAKEMASK), FALSE, 0, PARAMID_MAKEMASK);

	AEFX_CLR_STRUCT(def);
	def.flags = PF_ParamFlag_START_COLLAPSED;
	PF_ADD_TOPIC(	STR(StrID_TOPIC_LINE), PARAMID_TOPIC_LINE);

	AEFX_CLR_STRUCT(def);
	PF_END_TOPIC(PARAMID_TOPIC_LINE_END);

	AEFX_CLR_STRUCT(def);
	def.flags = PF_ParamFlag_START_COLLAPSED;
	PF_ADD_TOPIC(	STR(StrID_TOPIC_HATCHING), PARAMID_TOPIC_HATCHING);

	AEFX_CLR_STRUCT(def);
	PF_END_TOPIC(PARAMID_TOPIC_HATCHING_END);

	out_data->num_params = PARAMID_NUM_PARAMS;

	return err;
}

static PF_Err 
Render (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err				err		= PF_Err_NONE;

	err = RenderAnimEffect(in_data, out_data, params, output);

	return err;
}


DllExport	
PF_Err 
EntryPointFunc (
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra)
{
	PF_Err		err = PF_Err_NONE;
	
	try {
		switch (cmd) {
			case PF_Cmd_ABOUT:

				err = About(in_data,
							out_data,
							params,
							output);
				break;
				
			case PF_Cmd_GLOBAL_SETUP:

				err = GlobalSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_PARAMS_SETUP:

				err = ParamsSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_RENDER:

				err = Render(	in_data,
								out_data,
								params,
								output);
				break;

			case PF_Cmd_GLOBAL_SETDOWN:
				err = GlobalSetdown(in_data);
				break;
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

