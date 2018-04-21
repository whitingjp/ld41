#include "ui.h"

#include <string.h>

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
void ld41_menu_zero(ld41_menu* menu)
{
	menu->num_items = 0;
	_ld41_menu_add_title(menu, GROUP_ROOT, "hello");
	_ld41_menu_add_title(menu, GROUP_ROOT, "world");
}
void ld41_menu_update(const ld41_menu* menu, ld41_menu_pointer* pointer)
{
	(void)menu;
	(void)pointer;
}
void ld41_menu_draw(const ld41_menu* menu, const ld41_menu_pointer* pointer, whitgl_ivec draw_pos)
{
	whitgl_sprite sprite = {1, {0,0}, {6,12}};

	whitgl_int i;
	ld41_menu_group current_group = pointer->group[pointer->depth];
	for(i=0; i<menu->num_items; i++)
	{
		if(menu->items[i].group != current_group)
			continue;
		whitgl_sys_draw_text(sprite, menu->items[i].name, draw_pos);
		draw_pos.y += sprite.size.y;
	}
}
