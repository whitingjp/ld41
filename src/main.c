#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GLEW_STATIC
#include <GL/glew.h>

#if defined _WIN32
#include <windows.h> // TODO - Check this is required
#include <direct.h>
#endif

#include <whitgl/input.h>
#include <whitgl/logging.h>
#include <whitgl/math.h>
#include <whitgl/random.h>
#include <whitgl/sound.h>
#include <whitgl/sys.h>
#include <whitgl/timer.h>
#include <nfd.h>

#include <camera.h>
#include <gif.h>
#include <island.h>
#include <ui.h>



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
	float band = (floor(clamp(fragmentPosition.y,-1,8)*num_colors*(1.0/8.0))+2)/num_colors;\
	vec4 col = texture( palette, vec2(band, 0.75) );\
	outColor = vec4(col);\
}\
";

const char* flat_src = "\
#version 150\
\n\
uniform vec4 color;\
out vec4 outColor;\
void main()\
{\
	if(color.a < 0.6 && mod(gl_FragCoord.y,2) < 1.5)\
		discard;\
	outColor = vec4(color.rgb,1);\
}\
";

typedef enum
{
	MODE_NORMAL,
	MODE_RECORDING,
	MODE_POST_RECORDING,
} ld41_mode;

int main()
{
	WHITGL_LOG("Starting main.");

	whitgl_sys_setup setup = whitgl_sys_setup_zero;
	setup.size.x = 440*1;
	setup.size.y = 220*1;
	setup.pixel_size = 2;
	// setup.start_focused = false;
	setup.name = "Lofoten";
	setup.num_framebuffers = 2;

	WHITGL_LOG("Initiating sys");
	if(!whitgl_sys_init(&setup))
	{
		WHITGL_LOG("Failed to initiate sys");
		return 1;
	}

	whitgl_sys_enable_depth(true);

	WHITGL_LOG("Loading shaders");
	whitgl_shader flat_shader = whitgl_shader_zero;
	flat_shader.fragment_src = flat_src;
	flat_shader.num_uniforms = 1;
	flat_shader.uniforms[0].type = WHITGL_UNIFORM_COLOR;
	flat_shader.uniforms[0].name = "color";
	if(!whitgl_change_shader( WHITGL_SHADER_FLAT, flat_shader))
		return false;

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

	whitgl_random_seed seed = whitgl_random_seed_init(whitgl_sys_get_time()*10000);
	ld41_island island = ld41_island_zero;
	island.button_randomize = true;
	ld41_island island_prev = island;
	ld41_island island_target = island;
	whitgl_float island_lerp = 0.0;

	whitgl_sys_color colors[num_colors*2];
	colors[num_colors] = whitgl_sys_color_white;
	whitgl_ivec color_image_size = {num_colors,2};
	whitgl_sys_add_image_from_data(0, color_image_size, (unsigned char*)colors);

	whitgl_sys_add_image(1, "data/image/font.png");

	whitgl_sys_set_clear_color(colors[2]);

	whitgl_int frames_remaining = 0;
	ld41_mode game_mode = MODE_NORMAL;
	whitgl_sys_color* capture_data = malloc(sizeof(whitgl_sys_color)*setup.size.x*setup.size.y);
	gif_accumulator gif;

	ld41_menu* menu = malloc(sizeof(ld41_menu));
	ld41_menu_zero(menu, &island);
	ld41_menu_pointer menu_pointer = ld41_menu_pointer_zero;

	ld41_camera camera = ld41_camera_zero;

	float progress_bar_lerp = 1;
	whitgl_bool update_required = true;

	bool running = true;
	while(running)
	{
		whitgl_sound_update();
		whitgl_timer_tick();
		while(whitgl_timer_should_do_frame(60))
		{
			whitgl_input_update();
			// if(frames_remaining % 32 == 0)
			// {
			// 	seed = whitgl_random_seed_init(whitgl_iwrap(frames_remaining/32+1, 0, 8)+8);
			// 	island_prev = ld41_island_random(&seed);
			// 	seed = whitgl_random_seed_init(whitgl_iwrap(frames_remaining/32, 0, 8)+8);
			// 	island_target = ld41_island_random(&seed);
			// 	island_lerp = 0;
			// }
			if(island.button_randomize)
			{
				island.button_randomize = false;
				island_prev = island;

				float amount_above = 0;
				while(amount_above < 100)
				{
					island_target = ld41_island_random(&seed);
					amount_above = ld41_island_update_model(&island_target);
				}

				island_lerp = 0;
			}
			if(island.button_save_gif)
			{
				island.button_save_gif = false;
				nfdchar_t *savePath = NULL;
				nfdresult_t result = NFD_SaveDialog( "gif", NULL, &savePath );
				if ( result == NFD_OKAY )
				{
					WHITGL_LOG("Save gif to %s", savePath);
					gif_start(&gif, savePath, setup.size, colors, num_colors*2);
					frames_remaining = 128;
					game_mode = MODE_RECORDING;
				}
				else if ( result == NFD_CANCEL )
				{
					WHITGL_LOG("User Cancelled");
				}
				else
				{
					WHITGL_LOG("Dialog Error: %s", NFD_GetError() );
				}
				whitgl_grab_focus();
			}

			if(game_mode == MODE_NORMAL)
				update_required |= ld41_menu_update(menu, &menu_pointer, setup.size);
			else
				menu_pointer.lerp = whitgl_fclamp(menu_pointer.lerp-0.01, 0, 1);
			island.sky_ramp.src = island.color_ramp.src;
			island.sky_ramp.ctrl = island.color_ramp.ctrl;

			ld41_camera_update(&camera, game_mode == MODE_NORMAL && menu_pointer.up, game_mode == MODE_RECORDING);

			if(island_lerp < 1)
			{
				island_lerp = island_lerp+1/24.0;
				if(island_lerp > 1)
					island_lerp = 1;
				island = ld41_island_lerp(&island_prev, &island_target, island_lerp);
				update_required = true;
			}
			if(update_required)
			{
				ld41_island_update_model(&island);
				update_required = false;
			}

			ld41_color_ramp_palette(&island.color_ramp, &colors[1], num_colors-1);
			ld41_color_ramp_palette(&island.sky_ramp, &colors[1+num_colors], num_colors-1);
			whitgl_sys_update_image_from_data(0, color_image_size, (unsigned char*)colors);
			whitgl_sys_set_clear_color(colors[1]);

			if(game_mode != MODE_NORMAL)
				progress_bar_lerp = whitgl_fclamp(progress_bar_lerp-0.05, 0, 1);
			else
				progress_bar_lerp = whitgl_fclamp(progress_bar_lerp+0.05, 0, 1);

			if(game_mode == MODE_POST_RECORDING)
			{
				if(whitgl_input_pressed(WHITGL_INPUT_ANY))
					game_mode = MODE_NORMAL;
			}
			if(island.button_quit)
				running = false;
			if(whitgl_sys_should_close())
				running = false;
		}
		if(game_mode == MODE_RECORDING && menu_pointer.lerp <= 0)
			whitgl_sys_capture_frame_to_data(capture_data, true, 1);
		whitgl_sys_draw_init(1);

		whitgl_sys_enable_depth(true);

		whitgl_float fov = whitgl_tau*0.2;
		whitgl_fmat perspective = whitgl_fmat_perspective(fov, (float)setup.size.x/(float)setup.size.y, 0.1f, 20.0f);

		if(game_mode == MODE_RECORDING)
		{
			if(menu_pointer.lerp > 0)
			{
				camera.rot_y = (camera.rot_y*9+1)/10.0;
			}
			else
			{
				camera.rot_y = (128.0-frames_remaining)/128.0;
			}
		}
		whitgl_fmat view = ld41_camera_view(&camera);

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

		whitgl_sys_draw_init(0);
		whitgl_fmat ortho = whitgl_fmat_orthographic(0, setup.size.x, 0, setup.size.y, 0, 1.01);
		whitgl_fvec3 pane_verts[4] =
		{
			{0,setup.size.y,0},
			{setup.size.x,setup.size.y,0},
			{0,0,0},
			{setup.size.x,0,0},
		};
		whitgl_sys_draw_buffer_pane(1, pane_verts, WHITGL_SHADER_TEXTURE, whitgl_fmat_identity, whitgl_fmat_identity, ortho);

		whitgl_sys_enable_depth(false);

		if(game_mode == MODE_NORMAL || menu_pointer.lerp > 0.5)
			ld41_menu_draw(menu, &menu_pointer, setup.size);

		whitgl_ivec mid = {setup.size.x/2, setup.size.y-32};
		whitgl_ivec size = {setup.size.x/2, 16};
		whitgl_float off = whitgl_fsmoothstep(progress_bar_lerp, 0, 1)*setup.size.y;
		whitgl_iaabb progress_bar = {{mid.x-size.x/2, mid.y-size.y/2+off}, {mid.x+size.x/2, mid.y+size.y/2+off}};
		whitgl_sys_draw_hollow_iaabb(progress_bar, 1, whitgl_sys_color_white);
		progress_bar.b.x = progress_bar.a.x+size.x*(1-frames_remaining/128.0);
		whitgl_sys_draw_iaabb(progress_bar, whitgl_sys_color_white);

		if(game_mode == MODE_POST_RECORDING)
		{
			whitgl_sprite font = {1, {0,0}, {6,12}};
			whitgl_ivec saved_pos = {mid.x-4.5*font.size.x, progress_bar.a.y-font.size.y*2-4};
			whitgl_sys_draw_text(font, "gif saved", saved_pos);
			whitgl_ivec press_pos = {mid.x-7*font.size.x, progress_bar.a.y-font.size.y*1-4};
			whitgl_sys_draw_text(font, "press anything", press_pos);
		}

		whitgl_sys_draw_finish();

		if(game_mode == MODE_RECORDING && menu_pointer.lerp <= 0)
		{
			gif_add_frame(&gif, capture_data, 4);
			frames_remaining--;
			if(frames_remaining == 0)
			{
				gif_finalize(&gif);
				camera.rot_y = 0;
				game_mode = MODE_POST_RECORDING;
			}
		}

#if defined _WIN32
		if(!whitgl_sys_window_focused())
			Sleep(50);
#else
		if(!whitgl_sys_window_focused())
			usleep(50*1000);
#endif
	}

	free(capture_data);

	free(menu);

	ld41_island_shutdown();

	WHITGL_LOG("Shutting down input");
	whitgl_input_shutdown();
	WHITGL_LOG("Shutting down sound");
	whitgl_sound_shutdown();

	whitgl_sys_close();

	return 0;
}
