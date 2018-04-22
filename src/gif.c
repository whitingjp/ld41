
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gif_lib/gif_lib.h>

#include "gif.h"

void quit_gif_error(const char* location, int gif_error)
{
	printf("%s %s",location, GifErrorString(gif_error));
	exit(EXIT_FAILURE);
}

void gif_start(gif_accumulator* accumulator, const char* filename, whitgl_ivec size, whitgl_sys_color* colors, whitgl_int num_colors)
{
	int num_pixels = size.x*size.y;
	accumulator->size = size;
	accumulator->current = malloc(sizeof(unsigned char)*num_pixels);
	accumulator->accumulated = malloc(sizeof(unsigned char)*num_pixels);
	accumulator->render = malloc(sizeof(unsigned char)*num_pixels);

	whitgl_int i;
	for(i=0; i<num_pixels; i++)
		accumulator->accumulated[i] = 0;

	int error_code;
	accumulator->gif_file = EGifOpenFileName(filename, false, &error_code);
	if(!accumulator->gif_file)
		quit_gif_error("!accumulator->gif_file", error_code);

	if(num_colors > 256)
		num_colors = 256;

	int power_num_colors = 1;
	while(power_num_colors < num_colors)
		power_num_colors *= 2;

	accumulator->palette = malloc(sizeof(whitgl_sys_color)*power_num_colors);
	accumulator->palette_size = power_num_colors;
	GifColorType* color_map_data = malloc(sizeof(GifColorType)*power_num_colors);
	for(i=0; i<power_num_colors; i++)
	{
		whitgl_sys_color in;
		if(i >= num_colors)
			in = whitgl_sys_color_zero;
		else
			in = colors[i];
		color_map_data[i].Red = in.r;
		color_map_data[i].Green = in.g;
		color_map_data[i].Blue = in.b;
		accumulator->palette[i] = in;
	}
	ColorMapObject* color_map = GifMakeMapObject(power_num_colors, color_map_data);
	free(color_map_data);

	EGifSetGifVersion(accumulator->gif_file, true);

    if (EGifPutScreenDesc(accumulator->gif_file, size.x, size.y, GifBitSize(power_num_colors), 0, color_map) == GIF_ERROR)
		quit_gif_error("EGifPutScreenDesc", accumulator->gif_file->Error);


	GifByteType buf[16];
	memcpy(buf, "NETSCAPE2.0", 11);
	if(EGifPutExtensionLeader(accumulator->gif_file, 0xFF) == GIF_ERROR)
		quit_gif_error("EGifPutExtensionLeader", accumulator->gif_file->Error);
    if(EGifPutExtensionBlock(accumulator->gif_file, 11, buf) == GIF_ERROR)
   		quit_gif_error("EGifPutExtensionBlock", accumulator->gif_file->Error);
	buf[0] = 1;
	buf[1] = 0;
	buf[2] = 0;
    if(EGifPutExtensionBlock(accumulator->gif_file, 3, buf) == GIF_ERROR)
   		quit_gif_error("EGifPutExtensionBlock", accumulator->gif_file->Error);
    if(EGifPutExtensionTrailer(accumulator->gif_file) == GIF_ERROR)
    	quit_gif_error("EGifPutExtensionTrailer", accumulator->gif_file->Error);

}
void gif_add_frame(gif_accumulator* accumulator, whitgl_sys_color *data, int delay)
{
	whitgl_int num_pixels = accumulator->size.x*accumulator->size.y;
	whitgl_int i;
	for(i=0; i<num_pixels; i++)
	{
		whitgl_ivec p = {i%accumulator->size.x, i/accumulator->size.x};
		p.y = accumulator->size.y-p.y-1; // flip vertically, because our src data is for some reason
		whitgl_sys_color pixel_col = data[p.x+p.y*accumulator->size.x];
		whitgl_int j;
		whitgl_int best_dist = 256*3;
		unsigned char best_col = 0;
		for(j=1; j<accumulator->palette_size; j++)
		{
			whitgl_float dist = 0;
			dist += whitgl_fabs(pixel_col.r-accumulator->palette[j].r);
			dist += whitgl_fabs(pixel_col.g-accumulator->palette[j].g);
			dist += whitgl_fabs(pixel_col.b-accumulator->palette[j].b);
			if(dist > best_dist)
				continue;
			best_dist = dist;
			best_col = j;
		}
		if(pixel_col.a == 0)
			best_col = 0;
		accumulator->current[i] = best_col;
	}

	whitgl_iaabb bounds = {accumulator->size,{0,0}};
	for(i=0; i<num_pixels; i++)
	{
		unsigned char cur = accumulator->current[i];
		if(cur == 0 || cur == accumulator->accumulated[i])
		{
			accumulator->render[i] = 0;
		} else
		{
			accumulator->render[i] = cur;
			accumulator->accumulated[i] = cur;
			whitgl_ivec p = {i%accumulator->size.x, i/accumulator->size.x};
			if(p.x < bounds.a.x)
				bounds.a.x = p.x;
			if(p.x+1 > bounds.b.x)
				bounds.b.x = p.x+1;
			if(p.y < bounds.a.y)
				bounds.a.y = p.y;
			if(p.y+1 > bounds.b.y)
				bounds.b.y = p.y+1;
		}
	}
	if(bounds.b.x == 0 || bounds.b.y == 0)
	{
		return; // ignore dead frames
		// whitgl_iaabb safe_bounds = {{0,0},{1,1}};
		// bounds = safe_bounds;
	}


	GifByteType buf[16];
	GraphicsControlBlock gcb;
	gcb.DisposalMode = DISPOSE_DO_NOT;
	gcb.UserInputFlag = false;
	gcb.DelayTime = delay;
	gcb.TransparentColor = 0;

    int Len = EGifGCBToExtension(&gcb, (GifByteType *)buf);

	EGifPutExtensionLeader(accumulator->gif_file, GRAPHICS_EXT_FUNC_CODE);
    EGifPutExtensionBlock(accumulator->gif_file, Len, buf);
    EGifPutExtensionTrailer(accumulator->gif_file);

    if (EGifPutImageDesc(accumulator->gif_file, bounds.a.x, bounds.a.y, bounds.b.x-bounds.a.x, bounds.b.y-bounds.a.y, false, NULL) == GIF_ERROR)
		quit_gif_error("EGifPutImageDesc", accumulator->gif_file->Error);

	for(i=bounds.a.y; i<bounds.b.y; i++)
	{
		if (EGifPutLine(accumulator->gif_file, &accumulator->render[i*accumulator->size.x+bounds.a.x], bounds.b.x-bounds.a.x) == GIF_ERROR)
			quit_gif_error("EGifPutLine", accumulator->gif_file->Error);
	}
}
void gif_finalize(gif_accumulator* accumulator)
{
	int error_code;
    if (EGifCloseFile(accumulator->gif_file, &error_code) == GIF_ERROR)
    	quit_gif_error("EGifCloseFile", error_code);

	free(accumulator->current);
	free(accumulator->accumulated);
	free(accumulator->render);
	free(accumulator->palette);
}
