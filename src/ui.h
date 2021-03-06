#ifndef LD41_UI_H_
#define LD41_UI_H_

#include <whitgl/math.h>
#include <island.h>

typedef enum
{
	GROUP_ROOT,
	GROUP_BUMPS,
	GROUP_BUMPS_1,
	GROUP_BUMPS_2,
	GROUP_BUMPS_3,
	GROUP_BUMPS_4,
	GROUP_BUMPS_5,
	GROUP_COLORS,
	GROUP_COLORS_BOTTOM,
	GROUP_COLORS_MID,
	GROUP_COLORS_TOP,
	GROUP_COLORS_SKY,
	GROUP_MOON,
	GROUP_MAX,
} ld41_menu_group;

typedef enum
{
	TYPE_SUBMENU,
	TYPE_SLIDER,
	TYPE_BUTTON,
	TYPE_LIST,
} ld41_menu_type;

typedef struct
{
	whitgl_float* value;
	whitgl_float min;
	whitgl_float max;
	whitgl_bool wrap;
} ld41_slider_data;

typedef struct
{
	whitgl_bool* value;
} ld41_button_data;

typedef struct
{
	whitgl_int* value;
	whitgl_int max;
	char** name_array;
} ld41_list_data;

#define MAX_NAME_LENGTH (16)
typedef struct
{
	char name[MAX_NAME_LENGTH];
	ld41_menu_group group;
	ld41_menu_type type;
	ld41_slider_data slider;
	ld41_menu_group submenu;
	ld41_button_data button;
	ld41_list_data list;
} ld41_menu_item;

#define MAX_ITEMS (64)
typedef struct
{
	ld41_menu_item items[MAX_ITEMS];
	whitgl_int num_items;
} ld41_menu;

#define MAX_DEPTH (4)
typedef struct
{
	ld41_menu_group group[MAX_DEPTH];
	whitgl_int depth;
	whitgl_int highlighted;
	whitgl_ivec last_mouse;
	whitgl_bool mouse_interacting;

	whitgl_float idle_bounce;
	whitgl_bool ever_opened;

	whitgl_bool up;
	whitgl_float lerp;

	whitgl_int last_valid;
} ld41_menu_pointer;
static const ld41_menu_pointer ld41_menu_pointer_zero = {{GROUP_ROOT}, 1, 0, {0,0}, false, 0, false, false, 0, 0};

void ld41_menu_zero(ld41_menu* menu, ld41_island* island);
bool ld41_menu_update(const ld41_menu* menu, ld41_menu_pointer* pointer, whitgl_ivec setup_size);
void ld41_menu_draw(const ld41_menu* menu, const ld41_menu_pointer* pointer, whitgl_ivec setup_size);

#endif // LD41_UI_H_
