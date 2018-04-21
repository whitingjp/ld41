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
	menu->num_items++;
}
void _ld41_menu_add_slider(ld41_menu* menu, ld41_menu_group group, const char* name, whitgl_float* val, whitgl_float min, whitgl_float max)
{
	_ld41_menu_common(menu, group, name);
	menu->items[menu->num_items].slider.value = val;
	menu->items[menu->num_items].slider.min = min;
	menu->items[menu->num_items].slider.max = max;
	menu->num_items++;
}
void ld41_menu_zero(ld41_menu* menu, ld41_island* island)
{
	menu->num_items = 0;
	_ld41_menu_add_title(menu, GROUP_ROOT, "hello");
	_ld41_menu_add_title(menu, GROUP_ROOT, "world");
	_ld41_menu_add_slider(menu, GROUP_ROOT, "red", &island->color_ramp.src.r, 0, 1);
}
void ld41_menu_update(const ld41_menu* menu, ld41_menu_pointer* pointer)
{
	ld41_menu_group current_group = pointer->group[pointer->depth];
	whitgl_int num_active_items = 0;
	whitgl_int i;
	for(i=0; i<menu->num_items; i++)
	{
		if(menu->items[i].group != current_group)
			continue;
		num_active_items++;
	}
	if(whitgl_input_pressed(WHITGL_INPUT_DOWN))
		pointer->highlighted = whitgl_iclamp(pointer->highlighted+1, 0, num_active_items-1);
	if(whitgl_input_pressed(WHITGL_INPUT_UP))
		pointer->highlighted = whitgl_iclamp(pointer->highlighted-1, 0, num_active_items-1);
}
void ld41_menu_draw(const ld41_menu* menu, const ld41_menu_pointer* pointer, whitgl_ivec draw_pos)
{
	whitgl_sprite sprite = {1, {0,0}, {6,12}};

	whitgl_int i;
	whitgl_int num_active_items = 0;
	ld41_menu_group current_group = pointer->group[pointer->depth];
	for(i=0; i<menu->num_items; i++)
	{
		if(menu->items[i].group != current_group)
			continue;
		if(num_active_items == pointer->highlighted)
		{
			whitgl_sys_color highlight_color = {0xff,0xff,0xff,0xff};
			whitgl_iaabb left_box = {{draw_pos.x-2-8, draw_pos.y-1}, {draw_pos.x-2, draw_pos.y+sprite.size.y}};
			whitgl_sys_draw_iaabb(left_box, highlight_color);
			whitgl_iaabb right_box = {{draw_pos.x+sprite.size.x*strlen(menu->items[i].name)+1, draw_pos.y-1}, {draw_pos.x+sprite.size.x*strlen(menu->items[i].name)+1+8, draw_pos.y+sprite.size.y}};
			whitgl_sys_draw_iaabb(right_box, highlight_color);
		}
		whitgl_sys_draw_text(sprite, menu->items[i].name, draw_pos);
		draw_pos.y += sprite.size.y;
		num_active_items++;
	}
}
