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

ld41_island ld41_island_random(whitgl_random_seed* seed)
{
	ld41_island island = ld41_island_zero;
	island.color_ramp.src.r = whitgl_random_int(seed, 0xff);
	island.color_ramp.src.g = whitgl_random_int(seed, 0xff);
	island.color_ramp.src.b = whitgl_random_int(seed, 0xff);
	island.color_ramp.dest.r = whitgl_random_int(seed, 0xff);
	island.color_ramp.dest.g = whitgl_random_int(seed, 0xff);
	island.color_ramp.dest.b = whitgl_random_int(seed, 0xff);
	island.color_ramp.ctrl.r = whitgl_random_int(seed, 0xff);
	island.color_ramp.ctrl.g = whitgl_random_int(seed, 0xff);
	island.color_ramp.ctrl.b = whitgl_random_int(seed, 0xff);
	return island;
}

ld41_island ld41_island_lerp(const ld41_island* src, const ld41_island* dest, whitgl_float t)
{
	ld41_island island = ld41_island_zero;
	island.color_ramp.src = whitgl_sys_color_blend(src->color_ramp.src, dest->color_ramp.src, t);
	island.color_ramp.dest = whitgl_sys_color_blend(src->color_ramp.dest, dest->color_ramp.dest, t);
	island.color_ramp.ctrl = whitgl_sys_color_blend(src->color_ramp.ctrl, dest->color_ramp.ctrl, t);
	return island;
}
