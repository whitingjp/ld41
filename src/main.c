#include <stdbool.h>
#include <stddef.h>

#include <whitgl/input.h>
#include <whitgl/logging.h>
#include <whitgl/math.h>
#include <whitgl/sound.h>
#include <whitgl/sys.h>
#include <whitgl/timer.h>

int main()
{
	WHITGL_LOG("Starting main.");

	whitgl_sys_setup setup = whitgl_sys_setup_zero;
	setup.size.x = 320;
	setup.size.y = 180;
	setup.pixel_size = 2;
	setup.name = "game";

	WHITGL_LOG("Initiating sys");
	if(!whitgl_sys_init(&setup))
	{
		WHITGL_LOG("Failed to initiate sys");
		return 1;
	}

	WHITGL_LOG("Initiating sound");
	whitgl_sound_init();
	WHITGL_LOG("Initiating input");
	whitgl_input_init();
	WHITGL_LOG("Initiating timer");
	whitgl_timer_init();

	whitgl_load_model(0, "data/model/land.wmd");

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
		whitgl_sys_draw_init(0);

		whitgl_float fov = whitgl_tau/4;
		whitgl_fmat perspective = whitgl_fmat_perspective(fov, (float)setup.size.x/(float)setup.size.y, 0.1f, 20.0f);
		whitgl_fvec3 up = {0,1,0};
		whitgl_fvec3 camera_pos = {0,3,-10};
		whitgl_fvec3 camera_to = {0,0,0};
		whitgl_fmat view = whitgl_fmat_lookAt(camera_pos, camera_to, up);

		// whitgl_fmat model_matrix = whitgl_fmat_rot_y(time);
		// model_matrix = whitgl_fmat_multiply(model_matrix, whitgl_fmat_rot_z(time*3));

		whitgl_sys_draw_model(0, WHITGL_SHADER_MODEL, whitgl_fmat_identity, view, perspective);

		whitgl_sys_draw_finish();
	}

	WHITGL_LOG("Shutting down input");
	whitgl_input_shutdown();
	WHITGL_LOG("Shutting down sound");
	whitgl_sound_shutdown();

	whitgl_sys_close();

	return 0;
}
