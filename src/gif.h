#ifndef GIF_H_
#define GIF_H_

#include <whitgl/math.h>
#include <whitgl/sys.h>

#include <gif_lib/gif_lib.h>

void gif_save();


typedef struct
{
	whitgl_ivec size;
	unsigned char* current;
	unsigned char* accumulated;
	unsigned char* render;
	whitgl_sys_color* palette;
	whitgl_int palette_size;
	GifFileType* gif_file;
} gif_accumulator;

void gif_start(gif_accumulator* accumulator, const char* filename, whitgl_ivec size, whitgl_sys_color* colors, whitgl_int num_colors);
void gif_add_frame(gif_accumulator* accumulator, whitgl_sys_color* data, int delay);
void gif_finalize(gif_accumulator* accumulator);

#endif // #GIF_H_
