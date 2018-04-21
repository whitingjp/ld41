#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GLEW_STATIC
#include <GL/glew.h>

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
out vec4 outColor;\
uniform sampler2D palette;\
uniform float num_colors;\
void main()\
{\
	if(fragmentPosition.y < 0.0)\
		discard;\
	float band = (floor(fragmentPosition.y*num_colors*1.2)+2)/num_colors;\
	vec4 col = texture( palette, vec2(band, 0.25) );\
	outColor = vec4(col);\
}\
";

const char* reflection_src = "\
#version 150\
\n\
in vec3 fragmentPosition;\
out vec4 outColor;\
uniform sampler2D palette;\
uniform float num_colors;\
void main()\
{\
	if(fragmentPosition.y > 0.0)\
		discard;\
	if(mod(gl_FragCoord.y,2) < 1.5)\
		discard;\
	float band = (floor(-fragmentPosition.y*num_colors*1.2)+2)/num_colors;\
	vec4 col = texture( palette, vec2(band, 0.25) );\
	outColor = vec4(col);\
}\
";



const char* skysphere_src = "\
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
	float band = (floor(fragmentPosition.y*num_colors*(1.0/8.0))+2)/num_colors;\
	vec4 col = texture( palette, vec2(band, 0.75) );\
	outColor = vec4(col);\
}\
";

int main()
{
	WHITGL_LOG("Starting main.");

	whitgl_sys_setup setup = whitgl_sys_setup_zero;
	setup.size.x = 440*1;
	setup.size.y = 220*1;
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

	whitgl_shader sky_shader = whitgl_shader_zero;
	sky_shader.fragment_src = skysphere_src;
	sky_shader.num_uniforms = 2;
	sky_shader.uniforms[0].name = "palette";
	sky_shader.uniforms[0].type = WHITGL_UNIFORM_IMAGE;
	sky_shader.uniforms[1].name = "num_colors";
	sky_shader.uniforms[1].type = WHITGL_UNIFORM_FLOAT;
	if(!whitgl_change_shader( WHITGL_SHADER_EXTRA_0, sky_shader))
		return false;

	whitgl_shader reflection_shader = whitgl_shader_zero;
	reflection_shader.fragment_src = reflection_src;
	reflection_shader.num_uniforms = 2;
	reflection_shader.uniforms[0].name = "palette";
	reflection_shader.uniforms[0].type = WHITGL_UNIFORM_IMAGE;
	reflection_shader.uniforms[1].name = "num_colors";
	reflection_shader.uniforms[1].type = WHITGL_UNIFORM_FLOAT;
	if(!whitgl_change_shader( WHITGL_SHADER_EXTRA_1, reflection_shader))
		return false;


	static whitgl_int num_colors = 8;
	whitgl_set_shader_image(WHITGL_SHADER_MODEL, 0, 0);
	whitgl_set_shader_image(WHITGL_SHADER_EXTRA_0, 0, 0);
	whitgl_set_shader_image(WHITGL_SHADER_EXTRA_1, 0, 0);

	whitgl_set_shader_float(WHITGL_SHADER_MODEL, 1, num_colors);
	whitgl_set_shader_float(WHITGL_SHADER_EXTRA_0, 1, num_colors);
	whitgl_set_shader_float(WHITGL_SHADER_EXTRA_1, 1, num_colors);


	WHITGL_LOG("Initiating sound");
	whitgl_sound_init();
	WHITGL_LOG("Initiating input");
	whitgl_input_init();
	WHITGL_LOG("Initiating timer");
	whitgl_timer_init();

	if(!ld41_island_init())
		WHITGL_PANIC("Failed to load land.wmd");

	if(!whitgl_load_model(1, "data/model/skysphere.wmd"))
		WHITGL_PANIC("Failed to load skysphere");

	ld41_island island = ld41_island_zero;
	ld41_island island_prev = island;
	ld41_island island_target = island;
	whitgl_float island_lerp = 1;

	whitgl_sys_color colors[num_colors*2];
	whitgl_ivec color_image_size = {num_colors,2};
	whitgl_sys_add_image_from_data(0, color_image_size, (unsigned char*)colors);

	whitgl_sys_set_clear_color(colors[1]);

	whitgl_int frames_remaining = 0;
	whitgl_sys_color* capture_data = malloc(sizeof(whitgl_sys_color)*setup.size.x*setup.size.y);
	gif_accumulator gif;


	whitgl_random_seed seed = whitgl_random_seed_init(whitgl_sys_get_time()*10000);

	whitgl_float time = 0;

	bool running = true;
	while(running)
	{
		whitgl_sound_update();
		whitgl_timer_tick();
		while(whitgl_timer_should_do_frame(60))
		{
			whitgl_input_update();
			time = whitgl_fwrap(time+1/480.0, 0, 1);
			if(whitgl_input_pressed(WHITGL_INPUT_B))
			{
				gif_start(&gif, setup.size, colors, num_colors*2);
				frames_remaining = 128;
			}
			// if(frames_remaining % 32 == 0)
			// {
			// 	seed = whitgl_random_seed_init(whitgl_iwrap(frames_remaining/32+1, 0, 8)+8);
			// 	island_prev = ld41_island_random(&seed);
			// 	seed = whitgl_random_seed_init(whitgl_iwrap(frames_remaining/32, 0, 8)+8);
			// 	island_target = ld41_island_random(&seed);
			// 	island_lerp = 0;
			// }
			if(whitgl_input_pressed(WHITGL_INPUT_A))
			{
				island_prev = island;
				island_target = ld41_island_random(&seed);
				island_lerp = 0;
			}
			if(island_lerp < 1)
			{
				island_lerp = island_lerp+1/24.0;
				if(island_lerp > 1)
					island_lerp = 1;
				island = ld41_island_lerp(&island_prev, &island_target, island_lerp);
			}
			ld41_color_ramp_palette(&island.color_ramp, &colors[1], num_colors-1);
			ld41_color_ramp_palette(&island.sky_ramp, &colors[1+num_colors], num_colors-1);
			whitgl_sys_update_image_from_data(0, color_image_size, (unsigned char*)colors);
			whitgl_sys_set_clear_color(colors[1]);

			ld41_island_update_model(&island);

			if(whitgl_input_pressed(WHITGL_INPUT_ESC))
				running = false;
			if(whitgl_sys_should_close())
				running = false;
		}
		if(frames_remaining > 0)
			whitgl_sys_capture_frame_to_data(capture_data, true);
		whitgl_sys_draw_init(0);

		whitgl_float fov = whitgl_tau*0.2;
		whitgl_fmat perspective = whitgl_fmat_perspective(fov, (float)setup.size.x/(float)setup.size.y, 0.1f, 20.0f);
		whitgl_fvec3 up = {0,1,0};

		whitgl_float render_time = time;
		if(frames_remaining > 0)
			render_time = (128.0-frames_remaining)/128.0;
		whitgl_fmat rotate = whitgl_fmat_rot_y(render_time*whitgl_tau);
		whitgl_fvec3 camera_pos = {0,0.25,-1.3};
		camera_pos = whitgl_fvec3_apply_fmat(camera_pos, rotate);
		whitgl_fvec3 camera_to = {0,0,0};
		whitgl_fmat view = whitgl_fmat_lookAt(camera_pos, camera_to, up);


		glFrontFace(GL_CW);
		static const whitgl_fmat whitgl_fmat_flipy =
		{
		{
			1,0,0,0,
			0,-1,0,0,
			0,0,1,0,
			0,0,0,1
		}
		};
		whitgl_sys_draw_model(0, WHITGL_SHADER_EXTRA_1, whitgl_fmat_flipy, view, perspective);

		glFrontFace(GL_CCW);
		whitgl_sys_draw_model(0, WHITGL_SHADER_MODEL, whitgl_fmat_identity, view, perspective);
		whitgl_sys_draw_model(1, WHITGL_SHADER_EXTRA_0, whitgl_fmat_identity, view, perspective);


		whitgl_sys_draw_finish();

		if(frames_remaining > 0)
		{
			gif_add_frame(&gif, capture_data, 4);
			frames_remaining--;
			if(frames_remaining == 0)
				gif_finalize(&gif);
		}

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

	ld41_island_shutdown();

	WHITGL_LOG("Shutting down input");
	whitgl_input_shutdown();
	WHITGL_LOG("Shutting down sound");
	whitgl_sound_shutdown();

	whitgl_sys_close();

	return 0;
}
