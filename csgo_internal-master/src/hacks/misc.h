
#ifndef HACKS_MISC_H
#define HACKS_MISC_H

#include <sdk/hl_client.h>
#include <sdk/input.h>
#include <sdk/engine_sound.h>
#include <sdk/datamap.h>
#include <sdk/glow_manager.h>

namespace hacks
{
namespace misc
{
	void on_post_data();
	void on_exit();
	void on_frame_stage_notify(sdk::framestage stage);
	void on_override_view();
	void on_before_move();
	void on_create_move(sdk::user_cmd* cmd);
	void on_sound(sdk::start_sound_params* params, const char* name);

	extern std::vector<sdk::typedescription_t> player_types, grenade_types;
	extern sdk::typedescription_t* old_player_types, *old_grenade_types;
	extern uint32_t old_player_num_field, old_grenade_num_field;
}
}

#endif // HACKS_MISC_H
