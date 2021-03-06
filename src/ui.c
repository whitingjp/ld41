#include "ui.h"

#include <stdio.h>
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

void _ld41_menu_add_submenu(ld41_menu* menu, ld41_menu_group group, const char* name, ld41_menu_group submenu)
{
	_ld41_menu_common(menu, group, name);
	menu->items[menu->num_items].type = TYPE_SUBMENU;
	menu->items[menu->num_items].submenu = submenu;
	menu->num_items++;
}
void _ld41_menu_add_slider(ld41_menu* menu, ld41_menu_group group, const char* name, whitgl_float* val, whitgl_float min, whitgl_float max, whitgl_bool wrap)
{
	_ld41_menu_common(menu, group, name);
	menu->items[menu->num_items].type = TYPE_SLIDER;
	menu->items[menu->num_items].slider.value = val;
	menu->items[menu->num_items].slider.min = min;
	menu->items[menu->num_items].slider.max = max;
	menu->items[menu->num_items].slider.wrap = wrap;
	menu->num_items++;
}
void _ld41_menu_add_button(ld41_menu* menu, ld41_menu_group group, const char* name, whitgl_bool* value)
{
	_ld41_menu_common(menu, group, name);
	menu->items[menu->num_items].type = TYPE_BUTTON;
	menu->items[menu->num_items].button.value = value;
	menu->num_items++;
}
void _ld41_menu_add_list(ld41_menu* menu, ld41_menu_group group, const char* name, whitgl_int* value, whitgl_int max, char** name_array)
{
	_ld41_menu_common(menu, group, name);
	menu->items[menu->num_items].type = TYPE_LIST;
	menu->items[menu->num_items].list.value = value;
	menu->items[menu->num_items].list.max = max;
	menu->items[menu->num_items].list.name_array = name_array;
	menu->num_items++;
}

void ld41_menu_zero(ld41_menu* menu, ld41_island* island)
{
	menu->num_items = 0;
	_ld41_menu_add_button(menu, GROUP_ROOT, "randomize", &island->button_randomize);
	_ld41_menu_add_button(menu, GROUP_ROOT, "save gif", &island->button_save_gif);
	_ld41_menu_add_submenu(menu, GROUP_ROOT, "bumps", GROUP_BUMPS);

	whitgl_int i;
	for(i=0; i<NUM_BLOBS; i++)
	{
		char buffer[MAX_NAME_LENGTH];
		snprintf(buffer, MAX_NAME_LENGTH, "bump %lld", i+1);
		_ld41_menu_add_submenu(menu, GROUP_BUMPS, buffer, GROUP_BUMPS_1+i);
		_ld41_menu_add_list(menu, GROUP_BUMPS_1+i, "type", &island->blobs[i].type, TYPE_MAX, blob_type_name_array);
		_ld41_menu_add_slider(menu, GROUP_BUMPS_1+i, "radius", &island->blobs[i].size, 0, 1, false);
		_ld41_menu_add_slider(menu, GROUP_BUMPS_1+i, "height", &island->blobs[i].height, 0, MAX_BUMP_HEIGHT, false);
		_ld41_menu_add_slider(menu, GROUP_BUMPS_1+i, "angle", &island->blobs[i].angle, 0, whitgl_tau, true);
		_ld41_menu_add_slider(menu, GROUP_BUMPS_1+i, "dist", &island->blobs[i].dist, -0.9, 0.9, false);
	}

	_ld41_menu_add_submenu(menu, GROUP_ROOT, "colors", GROUP_COLORS);
	_ld41_menu_add_submenu(menu, GROUP_COLORS, "bottom", GROUP_COLORS_BOTTOM);
	_ld41_menu_add_slider(menu, GROUP_COLORS_BOTTOM, "red", &island->color_ramp.src.r, 0, 1, false);
	_ld41_menu_add_slider(menu, GROUP_COLORS_BOTTOM, "green", &island->color_ramp.src.g, 0, 1, false);
	_ld41_menu_add_slider(menu, GROUP_COLORS_BOTTOM, "blue", &island->color_ramp.src.b, 0, 1, false);
	_ld41_menu_add_submenu(menu, GROUP_COLORS, "middle", GROUP_COLORS_MID);
	_ld41_menu_add_slider(menu, GROUP_COLORS_MID, "red", &island->color_ramp.ctrl.r, 0, 1, false);
	_ld41_menu_add_slider(menu, GROUP_COLORS_MID, "green", &island->color_ramp.ctrl.g, 0, 1, false);
	_ld41_menu_add_slider(menu, GROUP_COLORS_MID, "blue", &island->color_ramp.ctrl.b, 0, 1, false);
	_ld41_menu_add_submenu(menu, GROUP_COLORS, "top", GROUP_COLORS_TOP);
	_ld41_menu_add_slider(menu, GROUP_COLORS_TOP, "red", &island->color_ramp.dest.r, 0, 1, false);
	_ld41_menu_add_slider(menu, GROUP_COLORS_TOP, "green", &island->color_ramp.dest.g, 0, 1, false);
	_ld41_menu_add_slider(menu, GROUP_COLORS_TOP, "blue", &island->color_ramp.dest.b, 0, 1, false);
	_ld41_menu_add_submenu(menu, GROUP_COLORS, "sky", GROUP_COLORS_SKY);
	_ld41_menu_add_slider(menu, GROUP_COLORS_SKY, "red", &island->sky_ramp.dest.r, 0, 1, false);
	_ld41_menu_add_slider(menu, GROUP_COLORS_SKY, "green", &island->sky_ramp.dest.g, 0, 1, false);
	_ld41_menu_add_slider(menu, GROUP_COLORS_SKY, "blue", &island->sky_ramp.dest.b, 0, 1, false);

	_ld41_menu_add_submenu(menu, GROUP_ROOT, "moon", GROUP_MOON);
	_ld41_menu_add_slider(menu, GROUP_MOON, "size", &island->moon.size, 0, 1, false);
	_ld41_menu_add_slider(menu, GROUP_MOON, "height", &island->moon.height, 0, 1, false);
	_ld41_menu_add_slider(menu, GROUP_MOON, "angle", &island->moon.rotate, 0, 1, true);
	_ld41_menu_add_button(menu, GROUP_ROOT, "quit", &island->button_quit);
}
bool ld41_menu_update(const ld41_menu* menu, ld41_menu_pointer* pointer, whitgl_ivec setup_size)
{
	whitgl_bool update_required = false;

	whitgl_float offset = whitgl_fsmoothstep(1-pointer->lerp,0,1)*setup_size.x*0.5;
	whitgl_ivec draw_pos = {8+16+8-offset, 8+2+14};
	whitgl_sprite sprite = {1, {0,0}, {6,12}};


	whitgl_ivec mouse_pos = whitgl_input_mouse_pos(2);
	whitgl_ivec mouse_diff = whitgl_ivec_sub(mouse_pos, pointer->last_mouse);
	whitgl_bool mouse_moved = !whitgl_ivec_eq(mouse_diff, whitgl_ivec_zero);
	if(!whitgl_input_held(WHITGL_INPUT_MOUSE_LEFT))
		pointer->mouse_interacting = false;
	whitgl_int move_dir = 0;
	whitgl_iaabb mouse_box = {{mouse_pos.x-6,mouse_pos.y}, {mouse_pos.x+7, mouse_pos.y+1}};

	whitgl_ivec closed_draw_pos = {8+offset-setup_size.x*0.5, 8};
	whitgl_sprite open_sprite = {1, {0, 96}, {10,14}};
	whitgl_iaabb open_sprite_iaabb = {{closed_draw_pos.x, closed_draw_pos.y-6}, {closed_draw_pos.x+sprite.size.x*16+1+8, closed_draw_pos.y+open_sprite.size.y+6}};
	if(whitgl_input_pressed(WHITGL_INPUT_MOUSE_LEFT) && whitgl_iaabb_intersects(mouse_box, open_sprite_iaabb))
		pointer->up = true;


	whitgl_ivec close_sprite_pos = {8-offset,8};
	whitgl_iaabb close_sprite_iaabb = {{close_sprite_pos.x, close_sprite_pos.y-6}, {close_sprite_pos.x+sprite.size.x*16+1+8, closed_draw_pos.y+open_sprite.size.y+1}};
	if(whitgl_input_pressed(WHITGL_INPUT_MOUSE_LEFT) && whitgl_iaabb_intersects(mouse_box, close_sprite_iaabb))
		pointer->up = false;

	whitgl_bool old_pointer_up = pointer->up;
	if(whitgl_input_pressed(WHITGL_INPUT_START) ||whitgl_input_pressed(WHITGL_INPUT_ESC))
		pointer->up = !pointer->up;
	if(whitgl_input_pressed(WHITGL_INPUT_A) && !pointer->up)
		pointer->up = true;
 	if(whitgl_input_pressed(WHITGL_INPUT_B) && pointer->up && pointer->depth == 1)
 		pointer->up = false;
	if(pointer->up)
		pointer->ever_opened = true;
	if(old_pointer_up == false && pointer->up)
		return update_required;
	if(pointer->up)
		pointer->lerp = whitgl_fclamp(pointer->lerp+0.05, 0, 1);
	else
		pointer->lerp = whitgl_fclamp(pointer->lerp-0.05, 0, 1);

	pointer->idle_bounce = whitgl_fwrap(pointer->idle_bounce+1/240.0,0,1);
	if(!pointer->up)
		return update_required;

	if(mouse_moved && !pointer->mouse_interacting)
	{
		if(pointer->highlighted != -1)
			pointer->last_valid = pointer->highlighted;
		pointer->highlighted = -1;
	}
	whitgl_int i;
	for(i=0; i<menu->num_items; i++)
	{
		whitgl_int depth = -1;
		whitgl_int j;
		for(j=0; j<pointer->depth; j++)
			if(menu->items[i].group == pointer->group[j])
				depth = j;
		if(depth == -1)
			continue;

		whitgl_ivec p = draw_pos;
		p.x += 6*2*depth;
		whitgl_iaabb select_box = {{p.x-2-8, p.y-1}, {p.x+sprite.size.x*16+1+8, p.y+sprite.size.y}};

		if(!pointer->mouse_interacting && whitgl_iaabb_intersects(mouse_box, select_box) && mouse_moved)
			pointer->highlighted = i;

		if(i != pointer->highlighted)
		{
			draw_pos.y += 12;
			continue;
		}

		if(whitgl_input_pressed(WHITGL_INPUT_B) && pointer->depth > 1)
		{
			if(menu->items[i].group == pointer->group[pointer->depth-1])
				move_dir--;
			pointer->depth=depth+1;
		}
		if(menu->items[i].type == TYPE_SLIDER)
		{
			whitgl_fvec joystick = whitgl_input_joystick();
			whitgl_float old_value = *menu->items[i].slider.value;
			whitgl_float dist = menu->items[i].slider.max-menu->items[i].slider.min;
			whitgl_float speed = joystick.x*dist*0.015;
			if(whitgl_input_held(WHITGL_INPUT_A))
				speed *= 4;
			if(menu->items[i].slider.wrap)
				*menu->items[i].slider.value = whitgl_fwrap(*menu->items[i].slider.value+speed, menu->items[i].slider.min,  menu->items[i].slider.max);
			else
				*menu->items[i].slider.value = whitgl_fclamp(*menu->items[i].slider.value+speed, menu->items[i].slider.min,  menu->items[i].slider.max);

			whitgl_iaabb slider_box = {{p.x+sprite.size.x*8, p.y}, {p.x+sprite.size.x*16+1+8, p.y+sprite.size.y-1}};
			if((whitgl_iaabb_intersects(mouse_box, slider_box) && whitgl_input_pressed(WHITGL_INPUT_MOUSE_LEFT)) || pointer->mouse_interacting)
			{
				whitgl_float slider_box_width = slider_box.b.x-slider_box.a.x;
				whitgl_float pos0to1 = (mouse_pos.x+1-slider_box.a.x)/slider_box_width;
				whitgl_float dest_width = menu->items[i].slider.max-menu->items[i].slider.min;
				*menu->items[i].slider.value = whitgl_fclamp(pos0to1*dest_width+menu->items[i].slider.min, menu->items[i].slider.min,  menu->items[i].slider.max);;
				pointer->mouse_interacting = true;
			}
			if(old_value != *menu->items[i].slider.value)
				update_required = true;
		}
		if(menu->items[i].type == TYPE_SUBMENU)
		{
			if(whitgl_input_pressed(WHITGL_INPUT_A) || whitgl_input_pressed(WHITGL_INPUT_MOUSE_LEFT))
			{
				whitgl_bool found = false;
				for(j=0; j<pointer->depth; j++)
				{
					if(pointer->group[j] == menu->items[i].submenu)
					{
						pointer->depth=j;
						found = true;
					}
				}
				if(!found)
				{
					pointer->depth = depth+1;
					pointer->group[pointer->depth++] = menu->items[i].submenu;
				}
			}
		}
		if(menu->items[i].type == TYPE_BUTTON)
		{
			if(whitgl_input_pressed(WHITGL_INPUT_A) || whitgl_input_pressed(WHITGL_INPUT_MOUSE_LEFT))
			{
				*menu->items[i].button.value = true;
				update_required = true;
			}
		}
		if(menu->items[i].type == TYPE_LIST)
		{
			whitgl_int dir = 0;
			if(whitgl_input_pressed(WHITGL_INPUT_LEFT))
				dir--;
			if(whitgl_input_pressed(WHITGL_INPUT_RIGHT))
				dir++;

			whitgl_int box_width = sprite.size.x*8+1+8;
			whitgl_int mid_x = p.x+sprite.size.x*8+box_width/2;
			if(whitgl_input_pressed(WHITGL_INPUT_MOUSE_LEFT))
			{
				if(mouse_pos.x < mid_x)
					dir--;
				else
					dir++;
			}

			*menu->items[i].list.value = whitgl_iclamp(*menu->items[i].list.value+dir, 0, menu->items[i].list.max-1);
			update_required = true;
		}
		draw_pos.y += 12;
	}

	if(pointer->highlighted == -1 && (whitgl_input_pressed(WHITGL_INPUT_DOWN) || whitgl_input_pressed(WHITGL_INPUT_UP)))
	{
		pointer->highlighted = pointer->last_valid;
	}
	else
	{
		if(whitgl_input_pressed(WHITGL_INPUT_DOWN))
			move_dir++;
		if(whitgl_input_pressed(WHITGL_INPUT_UP))
			move_dir--;
	}
	whitgl_int trial = pointer->highlighted+move_dir;
	while(trial >= 0 && trial < menu->num_items)
	{
		whitgl_int depth = -1;
		whitgl_int j;
		for(j=0; j<pointer->depth; j++)
			if(menu->items[trial].group == pointer->group[j])
				depth = j;
		if(depth != -1)
		{
			pointer->highlighted = trial;
			break;
		}
		trial += move_dir;
	}

	pointer->last_mouse = mouse_pos;
	return update_required;
}
void ld41_menu_draw(const ld41_menu* menu, const ld41_menu_pointer* pointer, whitgl_ivec setup_size)
{
	whitgl_float offset = whitgl_fsmoothstep(1-pointer->lerp,0,1)*setup_size.x*0.5;
	whitgl_ivec draw_pos = {8+16+8-offset, 8+2+14};
	whitgl_sprite sprite = {1, {0,0}, {6,12}};

	whitgl_ivec closed_draw_pos = {8+offset-setup_size.x*0.5, 8};
	whitgl_sprite open_sprite = {1, {0, 96}, {10,14}};
	whitgl_ivec open_sprite_pos = closed_draw_pos;
	whitgl_float bounce = (whitgl_fclamp(pointer->idle_bounce-0.9,0,0.1))*10*2;
	if(bounce > 1)
		bounce = 2-bounce;
	bounce = whitgl_fsmoothstep(bounce, 0, 1);
	if(!pointer->ever_opened)
		open_sprite_pos.x += bounce*6;
	whitgl_sys_draw_sprite(open_sprite, whitgl_ivec_zero, open_sprite_pos);

	whitgl_ivec close_sprite_pos = {8-offset,8};
	whitgl_ivec close_sprite_frame = {1,0};
	whitgl_sys_draw_sprite(open_sprite, close_sprite_frame, close_sprite_pos);
	close_sprite_pos.x += 16;
	whitgl_sys_draw_text(sprite, "lofoten", close_sprite_pos);

	whitgl_int i;
	for(i=0; i<menu->num_items; i++)
	{
		whitgl_ivec p = draw_pos;
		whitgl_sys_color highlight_color = {0xff,0xff,0xff,0xff/2};
		whitgl_sys_color ui_color = {0xff,0xff,0xff,0xff};

		whitgl_int depth = -1;
		whitgl_int j;
		for(j=0; j<pointer->depth; j++)
			if(menu->items[i].group == pointer->group[j])
				depth = j;
		if(depth == -1)
			continue;

		p.x += sprite.size.x*2*depth;

		if(i == pointer->highlighted)
		{
			whitgl_iaabb left_box = {{p.x-2-8, p.y-1}, {p.x-2, p.y+sprite.size.y}};
			whitgl_sys_draw_iaabb(left_box, ui_color);
		} else
		{
			whitgl_iaabb left_box = {{p.x-2-7, p.y-1}, {p.x-3, p.y+sprite.size.y}};
			whitgl_sys_draw_iaabb(left_box, highlight_color);
		}
		whitgl_sys_draw_text(sprite, menu->items[i].name, p);
		if(menu->items[i].type == TYPE_SUBMENU)
		{
			whitgl_ivec marker_pos = {p.x+sprite.size.x*(strlen(menu->items[i].name)+1), p.y};

			whitgl_bool opened = false;
			for(j=0; j<pointer->depth; j++)
				if(menu->items[i].submenu == pointer->group[j])
					opened = true;
			if(!opened)
				whitgl_sys_draw_text(sprite, ">", marker_pos);
			else
				whitgl_sys_draw_text(sprite, "<", marker_pos);
		}

		whitgl_int box_width = sprite.size.x*8+1+8;
		if(menu->items[i].type == TYPE_SLIDER)
		{
			whitgl_iaabb slider_box = {{p.x+sprite.size.x*8, p.y+2}, {p.x+sprite.size.x*8+box_width, p.y+sprite.size.y-3}};
			whitgl_sys_draw_hollow_iaabb(slider_box, 1, ui_color);
			whitgl_int total_width = slider_box.b.x-slider_box.a.x-2;
			whitgl_float dist = menu->items[i].slider.max-menu->items[i].slider.min;
			whitgl_float display_value = ((*menu->items[i].slider.value)-menu->items[i].slider.min)/dist;
			slider_box.b.x = slider_box.a.x+(1+total_width)*display_value;
			if(i == pointer->highlighted)
				whitgl_sys_draw_iaabb(slider_box, ui_color);
			else
				whitgl_sys_draw_iaabb(slider_box, highlight_color);
		}

		if(menu->items[i].type == TYPE_LIST)
		{
			const char* label = menu->items[i].list.name_array[*menu->items[i].list.value];
			whitgl_int string_size = sprite.size.x*strlen(label);
			whitgl_ivec label_pos = {p.x+sprite.size.x*8+(box_width-string_size)/2, p.y};
			whitgl_sys_draw_text(sprite, label, label_pos);
			if(i == pointer->highlighted)
			{
				whitgl_ivec left_pos = {label_pos.x-sprite.size.x*1.5, label_pos.y};
				if(*menu->items[i].list.value > 0)
					whitgl_sys_draw_text(sprite, "<", left_pos);
				whitgl_ivec right_pos = {label_pos.x+string_size+sprite.size.x*0.5+1, label_pos.y};
				if(*menu->items[i].list.value < menu->items[i].list.max-1)
					whitgl_sys_draw_text(sprite, ">", right_pos);
			}
		}
		draw_pos.y += sprite.size.y;
	}
}
