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

whitgl_sys_color ld41_float_color_to_sys_color(ld41_float_color color)
{
	whitgl_sys_color col = {color.r*0xff, color.g*0xff, color.b*0xff, 0xff};
	return col;
}
ld41_float_color ld41_float_color_blend(ld41_float_color a, ld41_float_color b, whitgl_float fac)
{
	whitgl_float inv = 1-fac;
	ld41_float_color out;
	out.r = (a.r*inv + b.r*fac);
	out.g = (a.g*inv + b.g*fac);
	out.b = (a.b*inv + b.b*fac);
	return out;
}

void ld41_color_ramp_palette(const ld41_color_ramp* ramp, whitgl_sys_color* colors, whitgl_int num_colors)
{
	whitgl_int i;
	for(i=0; i<num_colors; i++)
	{
		whitgl_float factor = i/((float)num_colors-1);
		ld41_float_color a = ld41_float_color_blend(ramp->src, ramp->ctrl, factor);
		ld41_float_color b = ld41_float_color_blend(ramp->ctrl, ramp->dest, factor);
		ld41_float_color col = ld41_float_color_blend(a, b, factor);
		colors[i] = ld41_float_color_to_sys_color(col);
	}
}

ld41_island ld41_island_random(whitgl_random_seed* seed)
{
	ld41_island island = ld41_island_zero;

	island.color_ramp.src.r = whitgl_random_float(seed);
	island.color_ramp.src.g = whitgl_random_float(seed);
	island.color_ramp.src.b = whitgl_random_float(seed);
	island.color_ramp.dest.r = whitgl_random_float(seed);
	island.color_ramp.dest.g = whitgl_random_float(seed);
	island.color_ramp.dest.b = whitgl_random_float(seed);
	island.color_ramp.ctrl.r = whitgl_random_float(seed);
	island.color_ramp.ctrl.g = whitgl_random_float(seed);
	island.color_ramp.ctrl.b = whitgl_random_float(seed);

	island.moon.size = whitgl_random_float(seed);
	island.moon.height = whitgl_random_float(seed);
	island.moon.rotate = whitgl_random_float(seed);

	island.sky_ramp = island.color_ramp;
	island.sky_ramp.dest.r = whitgl_random_float(seed);
	island.sky_ramp.dest.g = whitgl_random_float(seed);
	island.sky_ramp.dest.b = whitgl_random_float(seed);
	whitgl_int i;
	for(i=0; i<NUM_BLOBS; i++)
	{
		island.blobs[i].type = whitgl_random_int(seed, TYPE_MAX);
		island.blobs[i].angle = whitgl_random_float(seed)*whitgl_tau;
		island.blobs[i].dist = whitgl_random_float(seed)*1.8-0.9;
		island.blobs[i].height = whitgl_random_float(seed)*MAX_BUMP_HEIGHT;
		island.blobs[i].size = whitgl_random_float(seed);
	}

	return island;
}

ld41_island ld41_island_lerp(const ld41_island* src, const ld41_island* dest, whitgl_float t)
{
	ld41_island island = ld41_island_zero;
	island.color_ramp.src = ld41_float_color_blend(src->color_ramp.src, dest->color_ramp.src, t);
	island.color_ramp.dest = ld41_float_color_blend(src->color_ramp.dest, dest->color_ramp.dest, t);
	island.color_ramp.ctrl = ld41_float_color_blend(src->color_ramp.ctrl, dest->color_ramp.ctrl, t);
	island.sky_ramp.src = ld41_float_color_blend(src->sky_ramp.src, dest->sky_ramp.src, t);
	island.sky_ramp.dest = ld41_float_color_blend(src->sky_ramp.dest, dest->sky_ramp.dest, t);
	island.sky_ramp.ctrl = ld41_float_color_blend(src->sky_ramp.ctrl, dest->sky_ramp.ctrl, t);
	island.moon.size = whitgl_finterpolate(src->moon.size, dest->moon.size, t);
	island.moon.height = whitgl_finterpolate(src->moon.height, dest->moon.height, t);
	island.moon.rotate = whitgl_finterpolate(src->moon.rotate, dest->moon.rotate, t);
	whitgl_int i;
	for(i=0; i<NUM_BLOBS; i++)
	{
		island.blobs[i].type =  t > 0.5 ? dest->blobs[i].type : src->blobs[i].type;
		island.blobs[i].angle = whitgl_angle_lerp(src->blobs[i].angle, dest->blobs[i].angle, t);
		island.blobs[i].dist = whitgl_finterpolate(src->blobs[i].dist, dest->blobs[i].dist, t);
		island.blobs[i].height = whitgl_finterpolate(src->blobs[i].height, dest->blobs[i].height, t);
		island.blobs[i].size = whitgl_finterpolate(src->blobs[i].size, dest->blobs[i].size, t);
	}

	return island;
}

float _ld41_blob_height(const ld41_height_blob* blob, whitgl_fvec p)
{
	whitgl_fvec offset = whitgl_fvec_scale_val(whitgl_angle_to_fvec(blob->angle), blob->dist);
	whitgl_fvec diff = whitgl_fvec_sub(p, offset);
	whitgl_float mag = whitgl_fvec_magnitude(diff);
	mag /= 0.2+blob->size*2;

	whitgl_float factor = 0;
	switch(blob->type)
	{
		case TYPE_MOUND:
			factor = whitgl_fclamp(1-mag*1.5,0,1);
			factor = factor*factor;
			break;
		case TYPE_PLATEAU:
			factor = 1-whitgl_fsmoothstep(mag,0.2,0.3);
			break;
		case TYPE_SPIKE:
			factor = 1-mag*3;
			break;
		case TYPE_NOISE:
		{
			whitgl_random_seed seed = whitgl_random_seed_init(diff.x*100000+diff.y*10000);
			factor = (whitgl_random_float(&seed)*2-1)*0.5*(1-mag);
			break;
		}
		case TYPE_DIP:
			factor = whitgl_fclamp(1-mag*1.5,0,1);
			factor = -(factor*factor);
		default:
			break;
	}
	if(blob->type != TYPE_DIP)
		factor = whitgl_fclamp(factor, 0, 1);
	else
		factor = whitgl_fclamp(factor, -1, 0);
	return factor*blob->height;
}

float _ld41_island_height_at_point(const ld41_island* island, whitgl_fvec p)
{
	whitgl_float height = 0;
	whitgl_int i;
	for(i=0; i<NUM_BLOBS; i++)
	{
		height += _ld41_blob_height(&island->blobs[i], p);
	}
	whitgl_float mag = whitgl_fvec_magnitude(p);
	whitgl_float border = whitgl_fsmoothstep(1-mag, 0.1, 0.2);
	height *= border;
	height -= 0.1;
	return height;
}

float ld41_island_update_model(const ld41_island* island)
{
	whitgl_float amount_above = 0;
	whitgl_int i;
	for(i=0; i<_model_num_vertices; i++)
	{
		whitgl_fvec3 pos = {_model_data[i*9+0], 0, _model_data[i*9+2]};
		whitgl_fvec top_down = {pos.x, pos.z};
		float height = _ld41_island_height_at_point(island, top_down);
		_model_data[i*9+1] = height;
		if(height > 0)
			amount_above += height;
	}

	whitgl_sys_update_model_from_data(0, _model_num_vertices, (char*)_model_data);
	return amount_above;
}
