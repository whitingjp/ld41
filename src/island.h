#ifndef ISLAND_H_
#define ISLAND_H_

#include <whitgl/random.h>
#include <whitgl/sys.h>

typedef struct
{
	whitgl_sys_color src;
	whitgl_sys_color dest;
	whitgl_sys_color ctrl;
} ld41_color_ramp;

void ld41_color_ramp_palette(const ld41_color_ramp* ramp, whitgl_sys_color* colors, whitgl_int num_colors);

typedef struct
{
	whitgl_float angle;
	whitgl_float dist;
	whitgl_float height;
} ld41_height_blob;

#define NUM_BLOBS (4)
typedef struct
{
	ld41_color_ramp color_ramp;
	ld41_height_blob blobs[NUM_BLOBS];
} ld41_island;
static const ld41_island ld41_island_zero =
{
	{{0x32,0x5a,0x1a,0xff},{0xea,0x65,0x41,0xff},{0xb8,0xe3,0x92,0xff}},
	{{0,0,0},{0,0,0},{0,0,0},{0,0,0}},
};

bool ld41_island_init();
void ld41_island_shutdown();

ld41_island ld41_island_random(whitgl_random_seed* seed);
ld41_island ld41_island_lerp(const ld41_island* src, const ld41_island* dest, whitgl_float dist);
void ld41_island_update_model(const ld41_island* model);

#endif // ISLAND_H_
