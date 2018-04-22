#ifndef ISLAND_H_
#define ISLAND_H_

#include <whitgl/random.h>
#include <whitgl/sys.h>

typedef struct
{
	whitgl_float r;
	whitgl_float g;
	whitgl_float b;
} ld41_float_color;

typedef struct
{
	ld41_float_color src;
	ld41_float_color dest;
	ld41_float_color ctrl;
} ld41_color_ramp;

void ld41_color_ramp_palette(const ld41_color_ramp* ramp, whitgl_sys_color* colors, whitgl_int num_colors);

typedef enum
{
	TYPE_MOUND,
	TYPE_PLATEAU,
	TYPE_SPIKE,
	TYPE_NOISE,
	TYPE_DIP,
	TYPE_MAX,
} ld41_blob_type;

static char* blob_type_name_array[] =
{
	"mound",
	"plateau",
	"spike",
	"noise",
	"dip",
	"max error",
};
#define MAX_BUMP_HEIGHT (0.5)
typedef struct
{
	whitgl_int type;
	whitgl_float angle;
	whitgl_float dist;
	whitgl_float height;
	whitgl_float size;
} ld41_height_blob;

#define NUM_BLOBS (5)
typedef struct
{
	ld41_color_ramp color_ramp;
	ld41_color_ramp sky_ramp;
	ld41_height_blob blobs[NUM_BLOBS];
	whitgl_bool button_randomize;
	whitgl_bool button_save_gif;
	whitgl_bool button_quit;
} ld41_island;
static const ld41_island ld41_island_zero =
{
	{{0,0,0},{0,0,0},{0,0,0}},
	{{0,0,0},{0,0,0},{0,0,0}},
	{{TYPE_MOUND,0,0,0,0},{TYPE_MOUND,0,0,0,0},{TYPE_MOUND,0,0,0,0},{TYPE_MOUND,0,0,0,0}},
	false,
	false,
	false,
};

bool ld41_island_init();
void ld41_island_shutdown();

ld41_island ld41_island_random(whitgl_random_seed* seed);
ld41_island ld41_island_lerp(const ld41_island* src, const ld41_island* dest, whitgl_float dist);
float ld41_island_update_model(const ld41_island* model);

#endif // ISLAND_H_
