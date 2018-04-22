#include "camera.h"

#include <whitgl/logging.h>
#include <whitgl/input.h>

void ld41_camera_update(ld41_camera* camera, whitgl_bool in_ui, whitgl_bool recording)
{
	if(in_ui)
		camera->ui_lerp = whitgl_fclamp(camera->ui_lerp+0.05, 0, 1);
	else
		camera->ui_lerp = whitgl_fclamp(camera->ui_lerp-0.05, 0, 1);

	if(!in_ui)
	{
		whitgl_fvec joystick = whitgl_input_joystick();

		if(whitgl_fabs(joystick.x) > 0.001)
		{
			camera->time_since_input = 0;
			camera->rot_y_speed = whitgl_fclamp(camera->rot_y_speed-joystick.x*0.01, -1, 1);
		}
	}

	if(camera->time_since_input >= 1 && camera->rot_y_speed < 0.25)
		camera->rot_y_speed = camera->rot_y_speed+0.0002;
	else if(camera->time_since_input > 0)
		camera->rot_y_speed = camera->rot_y_speed*0.97;
	camera->time_since_input = whitgl_fclamp(camera->time_since_input+1/480.0, 0, 1);

	if(recording)
		camera->rot_y_speed = 0;

	whitgl_float speed = (1.0/120.0)*camera->rot_y_speed;
	camera->rot_y = whitgl_fwrap(camera->rot_y+speed, 0, 1);

	const whitgl_fvec3 ui_camera_pos = {0,0.3,-1.6};
	const whitgl_fvec3 ui_camera_look_at = {0.5,0,0};

	whitgl_float ui_lerp_smooth = whitgl_fsmoothstep(camera->ui_lerp, 0, 1);
	camera->pos = whitgl_fvec3_interpolate(ld41_camera_zero.pos, ui_camera_pos, ui_lerp_smooth);
	camera->look_at = whitgl_fvec3_interpolate(ld41_camera_zero.look_at, ui_camera_look_at, ui_lerp_smooth);
}
whitgl_fmat ld41_camera_view(const ld41_camera* camera)
{
	whitgl_fvec3 up = {0,1,0};
	whitgl_fmat rotate = whitgl_fmat_rot_y(camera->rot_y*whitgl_tau);
	whitgl_fvec3 spun_camera_pos = whitgl_fvec3_apply_fmat(camera->pos, rotate);
	whitgl_fvec3 spun_camera_look_at = whitgl_fvec3_apply_fmat(camera->look_at, rotate);
	whitgl_fmat view = whitgl_fmat_lookAt(spun_camera_pos, spun_camera_look_at, up);
	return view;
}
