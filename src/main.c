#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <whitgl/input.h>
#include <whitgl/logging.h>
#include <whitgl/math.h>
#include <whitgl/random.h>
#include <whitgl/sound.h>
#include <whitgl/sys.h>
#include <whitgl/timer.h>

#include <island.h>
#include <gif.h>

const char* model_src = "\
#version 150\
\n\
in vec3 fragmentPosition;\
in vec3 fragmentColor;\
in vec3 fragmentNormal;\
out vec4 outColor;\
uniform sampler2D palette;\
uniform float num_colors;\
void main()\
{\
	if(fragmentPosition.y < 0.0)\
		discard;\
	float band = (floor(fragmentPosition.y*num_colors*1.2)+2)/num_colors;\
	vec4 col = texture( palette, vec2(band, 0.5) );\
	outColor = vec4(col);\
}\
";

whitgl_bool load_model(whitgl_int id, const char* filename)
{
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
	float* data = (float*)malloc(readSize);
	read = fread( data, 1, readSize, src );
	if(read != readSize)
	{
		WHITGL_LOG("Failed to read object from %s", filename);
		fclose(src);
		return false;
	}
	WHITGL_LOG("Loaded data from %s", filename);
	fclose(src);

	whitgl_int num_vertices = ((readSize/sizeof(float))/3)/3;

	whitgl_int i;
	for(i=0; i<num_vertices; i++)
	{
		whitgl_fvec3 pos = {data[i*9+0], data[i*9+1], data[i*9+2]};
		whitgl_fvec top_down = {pos.x, pos.z};
		whitgl_float mag = whitgl_fvec_magnitude(top_down);


		whitgl_random_seed seed = whitgl_random_seed_init(top_down.x*1000+top_down.y*10000);
		whitgl_float random = whitgl_random_float(&seed);

		whitgl_fvec offset = {-0.2,-0.4};
		whitgl_float offset_mag = whitgl_fclamp(whitgl_fvec_magnitude(whitgl_fvec_add(top_down, offset)),0,1);
		data[i*9+1] = (1-mag)*(0.5+((whitgl_fsin(pos.x*whitgl_tau*2)+1)/4))*0.8+random/32-whitgl_fpow(1-offset_mag,3);
	}

	whitgl_sys_update_model_from_data(id, num_vertices, (char*)data);
	return true;
}

int main()
{
	WHITGL_LOG("Starting main.");

	whitgl_sys_setup setup = whitgl_sys_setup_zero;
	setup.size.x = 440;
	setup.size.y = 220;
	setup.pixel_size = 2;
	setup.start_focused = false;
	setup.name = "game";

	WHITGL_LOG("Initiating sys");
	if(!whitgl_sys_init(&setup))
	{
		WHITGL_LOG("Failed to initiate sys");
		return 1;
	}

	whitgl_sys_enable_depth(true);

	whitgl_shader model_shader = whitgl_shader_zero;
	model_shader.fragment_src = model_src;
	model_shader.num_uniforms = 2;
	model_shader.uniforms[0].name = "palette";
	model_shader.uniforms[0].type = WHITGL_UNIFORM_IMAGE;
	model_shader.uniforms[1].name = "num_colors";
	model_shader.uniforms[1].type = WHITGL_UNIFORM_FLOAT;
	if(!whitgl_change_shader( WHITGL_SHADER_MODEL, model_shader))
		return false;

	whitgl_set_shader_image(WHITGL_SHADER_MODEL, 0, 0);


	WHITGL_LOG("Initiating sound");
	whitgl_sound_init();
	WHITGL_LOG("Initiating input");
	whitgl_input_init();
	WHITGL_LOG("Initiating timer");
	whitgl_timer_init();

	load_model(0, "data/model/land.wmd");


	ld41_island island = ld41_island_zero;

	static whitgl_int num_colors = 8;

	whitgl_set_shader_float(WHITGL_SHADER_MODEL, 1, num_colors);

	whitgl_sys_color colors[num_colors];
	whitgl_sys_color magic_pink = {0xff,0x00,0xff,0x00};
	colors[0] = magic_pink;
	ld41_color_ramp_palette(&island.color_ramp, &colors[1], num_colors-1);

	whitgl_ivec size = {num_colors,1};
	whitgl_sys_add_image_from_data(0, size, (unsigned char*)colors);

	whitgl_sys_set_clear_color(colors[1]);

	gif_accumulator gif;
	gif_start(&gif, setup.size, colors, num_colors);


	whitgl_int frame = 0;
	whitgl_sys_color* capture_data = malloc(sizeof(whitgl_sys_color)*setup.size.x*setup.size.y);

	bool running = true;
	while(running)
	{
		whitgl_sound_update();
		whitgl_timer_tick();
		while(whitgl_timer_should_do_frame(60))
		{
			whitgl_input_update();
			if(whitgl_input_pressed(WHITGL_INPUT_ESC))
				running = false;
			if(whitgl_sys_should_close())
				running = false;
		}
		if(frame < 128)
			whitgl_sys_capture_frame_to_data(capture_data, true);
		whitgl_sys_draw_init(0);

		whitgl_float fov = whitgl_tau*0.2;
		whitgl_fmat perspective = whitgl_fmat_perspective(fov, (float)setup.size.x/(float)setup.size.y, 0.1f, 20.0f);
		whitgl_fvec3 up = {0,1,0};

		whitgl_fmat rotate = whitgl_fmat_rot_y(frame*(whitgl_tau/128.0));
		whitgl_fvec3 camera_pos = {0,0.25,-1.3};
		camera_pos = whitgl_fvec3_apply_fmat(camera_pos, rotate);
		whitgl_fvec3 camera_to = {0,0,0};
		whitgl_fmat view = whitgl_fmat_lookAt(camera_pos, camera_to, up);

		whitgl_sys_draw_model(0, WHITGL_SHADER_MODEL, whitgl_fmat_identity, view, perspective);

		whitgl_sys_draw_finish();

		if(frame < 128)
		{
			gif_add_frame(&gif, capture_data, 4);
		}
		frame++;
		if(frame == 128)
			gif_finalize(&gif);


#if defined _WIN32
		if(whitgl_sys_window_focused())
			Sleep(10);
		else
			Sleep(100);
#else
		if(whitgl_sys_window_focused())
			usleep(10*1000);
		else
			usleep(100*1000);
#endif
	}

	free(capture_data);

	WHITGL_LOG("Shutting down input");
	whitgl_input_shutdown();
	WHITGL_LOG("Shutting down sound");
	whitgl_sound_shutdown();

	whitgl_sys_close();

	return 0;
}
