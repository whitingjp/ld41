#include "island.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <whitgl/logging.h>

float* _model_data;
int _model_num_vertices;

bool ld41_island_init()
{
	const char* filename = "data/model/land.wmd";
	FILE *src;
	int read;
	int readSize;
	src = fopen(filename, "rb");
	if (src == NULL)
	{
		WHITGL_LOG("Failed to open %s for load.", filename);
		return false;
	}
	read = fread( &readSize, 1, sizeof(readSize), src );
	if(read != sizeof(readSize))
	{
		WHITGL_LOG("Failed to read size from %s", filename);
		fclose(src);
		return false;
	}
	_model_data = (float*)malloc(readSize);
	read = fread( _model_data, 1, readSize, src );
	if(read != readSize)
	{
		WHITGL_LOG("Failed to read object from %s", filename);
		fclose(src);
		return false;
	}
	WHITGL_LOG("Loaded data from %s", filename);
	fclose(src);

	_model_num_vertices = ((readSize/sizeof(float))/3)/3;
	return true;
}

void ld41_island_shutdown()
{
	free(_model_data);
}

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

void ld41_island_update_model(const ld41_island* island)
{
	(void)island;
	whitgl_int i;
	for(i=0; i<_model_num_vertices; i++)
	{
		whitgl_fvec3 pos = {_model_data[i*9+0], 0, _model_data[i*9+2]};
		whitgl_fvec top_down = {pos.x, pos.z};
		whitgl_float mag = whitgl_fvec_magnitude(top_down);

		whitgl_random_seed seed = whitgl_random_seed_init(top_down.x*1000+top_down.y*10000);
		whitgl_float random = whitgl_random_float(&seed);

		whitgl_fvec offset = {-0.2,-0.4};
		whitgl_float offset_mag = whitgl_fclamp(whitgl_fvec_magnitude(whitgl_fvec_add(top_down, offset)),0,1);
		_model_data[i*9+1] = (1-mag)*(0.5+((whitgl_fsin(pos.x*whitgl_tau*2)+1)/4))*0.8+random/32-whitgl_fpow(1-offset_mag,3);
	}

	whitgl_sys_update_model_from_data(0, _model_num_vertices, (char*)_model_data);
}