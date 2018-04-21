#include "island.h"

void ld41_color_ramp_palette(const ld41_color_ramp* ramp, whitgl_sys_color* colors, whitgl_int num_colors)
{
	whitgl_int i;
	for(i=0; i<num_colors; i++)
	{
		whitgl_float factor = i/((float)num_colors-1);
		whitgl_sys_color a = whitgl_sys_color_blend(ramp->src, ramp->ctrl, factor);
		whitgl_sys_color b = whitgl_sys_color_blend(ramp->ctrl, ramp->dest, factor);
		whitgl_sys_color col = whitgl_sys_color_blend(a, b, factor);
		colors[i] = col;
	}
}
