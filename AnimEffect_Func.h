#pragma once

#include "AnimEffect.h"

void InitAnimEffect(char *res_path);
void ExitAnimEffect();
PF_Err RenderAnimEffect (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );

