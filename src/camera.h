#ifndef LD41_CAMERA_H_
#define LD41_CAMERA_H_

#include <whitgl/math.h>

typedef struct
{
	whitgl_fvec3 pos;
	whitgl_fvec3 look_at;
	whitgl_float ui_lerp;
	whitgl_float rot_y;
	whitgl_float rot_y_speed;
	whitgl_float rot_x;
	whitgl_float rot_x_speed;
	whitgl_float time_since_input_y;

	whitgl_ivec last_mouse;
} ld41_camera;
static const ld41_camera ld41_camera_zero = {{0,0.2,-1.2}, {0,0,0}, 0, 0, 0.25, 0.2, 0, 1, {0,0}};

void ld41_camera_update(ld41_camera* camera, whitgl_bool in_ui, whitgl_bool recording);
whitgl_fmat ld41_camera_view(const ld41_camera* camera);

#endif
