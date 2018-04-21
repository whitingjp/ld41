#include "ui.h"

#include <string.h>

#include <whitgl/input.h>
#include <whitgl/logging.h>
#include <whitgl/sys.h>

void _ld41_menu_common(ld41_menu* menu, ld41_menu_group group, const char* name)
{
	if(strlen(name) >= MAX_NAME_LENGTH)
		WHITGL_PANIC("Name too long");
	if(menu->num_items >= MAX_ITEMS)
		WHITGL_PANIC("Too many items");
	menu->items[menu->num_items].group = group;
	strncpy(menu->items[menu->num_items].name, name, MAX_NAME_LENGTH);
}

void _ld41_menu_add_title(ld41_menu* menu, ld41_menu_group group, const char* name)
{
	_ld41_menu_common(menu, group, name);
	menu->items[menu->num_items].type = TYPE_TITLE;
	menu->num_items++;
}
void _ld41_menu_add_slider(ld41_menu* menu, ld41_menu_group group, const char* name, whitgl_float* val, whitgl_float min, whitgl_float max)
{
	_ld41_menu_common(menu, group, name);
	menu->items[menu->num_items].type = TYPE_SLIDER;
	menu->items[menu->num_items].slider.value = val;
	menu->items[menu->num_items].slider.min = min;
	menu->items[menu->num_items].slider.max = max;
	menu->num_items++;
}
void ld41_menu_zero(ld41_menu* menu, ld41_island* island)
{
	menu->num_items = 0;
	_ld41_menu_add_title(menu, GROUP_ROOT, "bump 1");
	_ld41_menu_add_slider(menu, GROUP_ROOT, "angle", &island->blobs[0].angle, 0, whitgl_tau);
	_ld41_menu_add_slider(menu, GROUP_ROOT, "dist", &island->blobs[0].dist, -1, 1);
	_ld41_menu_add_slider(menu, GROUP_ROOT, "height", &island->blobs[0].height, 0, 1);
	_ld41_menu_add_title(menu, GROUP_ROOT, "bump 2");
	_ld41_menu_add_slider(menu, GROUP_ROOT, "angle", &island->blobs[1].angle, 0, whitgl_tau);
	_ld41_menu_add_slider(menu, GROUP_ROOT, "dist", &island->blobs[1].dist, -1, 1);
	_ld41_menu_add_slider(menu, GROUP_ROOT, "height", &island->blobs[1].height, 0, 1);
	_ld41_menu_add_title(menu, GROUP_ROOT, "bump 3");
	_ld41_menu_add_slider(menu, GROUP_ROOT, "angle", &island->blobs[2].angle, 0, whitgl_tau);
	_ld41_menu_add_slider(menu, GROUP_ROOT, "dist", &island->blobs[2].dist, -1, 1);
	_ld41_menu_add_slider(menu, GROUP_ROOT, "height", &island->blobs[2].height, 0, 1);
	_ld41_menu_add_title(menu, GROUP_ROOT, "bump 4");
	_ld41_menu_add_slider(menu, GROUP_ROOT, "angle", &island->blobs[3].angle, 0, whitgl_tau);
	_ld41_menu_add_slider(menu, GROUP_ROOT, "dist", &island->blobs[3].dist, -1, 1);
	_ld41_menu_add_slider(menu, GROUP_ROOT, "height", &island->blobs[3].height, 0, 1);

	_ld41_menu_add_title(menu, GROUP_COLORS, "bottom");
	_ld41_menu_add_slider(menu, GROUP_COLORS, "red", &island->color_ramp.src.r, 0, 1);
	_ld41_menu_add_slider(menu, GROUP_COLORS, "green", &island->color_ramp.src.g, 0, 1);
	_ld41_menu_add_slider(menu, GROUP_COLORS, "blue", &island->color_ramp.src.b, 0, 1);
	_ld41_menu_add_title(menu, GROUP_COLORS, "middle");
	_ld41_menu_add_slider(menu, GROUP_COLORS, "red", &island->color_ramp.ctrl.r, 0, 1);
	_ld41_menu_add_slider(menu, GROUP_COLORS, "green", &island->color_ramp.ctrl.g, 0, 1);
	_ld41_menu_add_slider(menu, GROUP_COLORS, "blue", &island->color_ramp.ctrl.b, 0, 1);
	_ld41_menu_add_title(menu, GROUP_COLORS, "top");
	_ld41_menu_add_slider(menu, GROUP_COLORS, "red", &island->color_ramp.dest.r, 0, 1);
	_ld41_menu_add_slider(menu, GROUP_COLORS, "green", &island->color_ramp.dest.g, 0, 1);
	_ld41_menu_add_slider(menu, GROUP_COLORS, "blue", &island->color_ramp.dest.b, 0, 1);
	_ld41_menu_add_title(menu, GROUP_COLORS, "sky");
	_ld41_menu_add_slider(menu, GROUP_COLORS, "red", &island->sky_ramp.dest.r, 0, 1);
	_ld41_menu_add_slider(menu, GROUP_COLORS, "green", &island->sky_ramp.dest.g, 0, 1);
	_ld41_menu_add_slider(menu, GROUP_COLORS, "blue", &island->sky_ramp.dest.b, 0, 1);
}
void ld41_menu_update(const ld41_menu* menu, ld41_menu_pointer* pointer)
{
	ld41_menu_group current_group = pointer->group[pointer->depth];
	whitgl_int i;
	whitgl_int num_active_items = -1;
	for(i=0; i<menu->num_items; i++)
	{
		if(menu->items[i].group != current_group)
			continue;
		num_active_items++;
		if(num_active_items != pointer->highlighted)
			continue;
		if(menu->items[i].type == TYPE_SLIDER)
		{
			whitgl_fvec joystick = whitgl_input_joystick();
			whitgl_float dist = menu->items[i].slider.max-menu->items[i].slider.min;
			*menu->items[i].slider.value = whitgl_fclamp(*menu->items[i].slider.value+joystick.x*dist*0.02, menu->items[i].slider.min,  menu->items[i].slider.max);
		}
	}
	num_active_items++;
	whitgl_int move_dir = 0;
	if(whitgl_input_pressed(WHITGL_INPUT_DOWN))
		move_dir++;
	if(whitgl_input_pressed(WHITGL_INPUT_UP))
		move_dir--;
	if(!move_dir)
		return;
	whitgl_int trial = pointer->highlighted;
	while(true)
	{
		trial += move_dir;
		num_active_items = -1;
		for(i=0; i<menu->num_items; i++)
		{
			if(menu->items[i].group != current_group)
				continue;
			num_active_items++;
			if(num_active_items != trial)
				continue;
			if(menu->items[i].type != TYPE_TITLE)
			{
				pointer->highlighted = trial;
				return;
			}
		}
		num_active_items++;
		if(trial < 0 || trial >= num_active_items)
			return;
	}
}
void ld41_menu_draw(const ld41_menu* menu, const ld41_menu_pointer* pointer, whitgl_ivec draw_pos)
{
	whitgl_sprite sprite = {1, {0,0}, {6,12}};

	whitgl_int i;
	whitgl_int num_active_items = 0;
	ld41_menu_group current_group = pointer->group[pointer->depth];
	for(i=0; i<menu->num_items; i++)
	{
		whitgl_ivec p = draw_pos;
		whitgl_sys_color highlight_color = {0xff,0xff,0xff,0xff/2};
		whitgl_sys_color ui_color = {0xff,0xff,0xff,0xff};
		if(menu->items[i].group != current_group)
			continue;
		if(menu->items[i].type != TYPE_TITLE)
			p.x += sprite.size.x*2;
		if(num_active_items == pointer->highlighted)
		{
			whitgl_iaabb left_box = {{p.x-2-8, p.y-1}, {p.x-2, p.y+sprite.size.y}};
			whitgl_sys_draw_iaabb(left_box, highlight_color);
			whitgl_iaabb right_box = {{p.x+sprite.size.x*strlen(menu->items[i].name)+1, p.y-1}, {p.x+sprite.size.x*strlen(menu->items[i].name)+1+8, p.y+sprite.size.y}};
			whitgl_sys_draw_iaabb(right_box, highlight_color);
		}
		whitgl_sys_draw_text(sprite, menu->items[i].name, p);
		if(menu->items[i].type == TYPE_SLIDER)
		{
			whitgl_iaabb slider_box = {{p.x+sprite.size.x*8, p.y}, {p.x+sprite.size.x*16+1+8, p.y+sprite.size.y-1}};
			whitgl_sys_draw_hollow_iaabb(slider_box, 1, ui_color);
			whitgl_int total_width = slider_box.b.x-slider_box.a.x-2;
			whitgl_float dist = menu->items[i].slider.max-menu->items[i].slider.min;
			whitgl_float display_value = ((*menu->items[i].slider.value)-menu->items[i].slider.min)/dist;
			slider_box.b.x = slider_box.a.x+1+total_width*display_value;
			whitgl_sys_draw_iaabb(slider_box, ui_color);
		}
		draw_pos.y += sprite.size.y;
		num_active_items++;
	}
}
