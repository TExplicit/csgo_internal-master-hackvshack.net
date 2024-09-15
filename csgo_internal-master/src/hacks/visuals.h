
#ifndef HACKS_VISUALS_H
#define HACKS_VISUALS_H

#include <sdk/cs_player.h>
#include <sdk/weapon.h>
#include <sdk/model_render.h>
#include <base/draw_manager.h>
#include <map>
#include <utility>
#include <atomic>

namespace sdk
{
	class game_event;
	class view_setup;
}

namespace hacks
{
	class visuals : public gui_legacy::drawable
	{
		inline static constexpr auto cam_hull_offset = 9.f;
		inline static const std::array<std::pair<std::string, char>, 4> smoke_materials =
		{
			XOR_STR_STORE("particle/vistasmokev1/vistasmokev1_fire"),
			XOR_STR_STORE("particle/vistasmokev1/vistasmokev1_smokegrenade"),
			XOR_STR_STORE("particle/vistasmokev1/vistasmokev1_emods"),
			XOR_STR_STORE("particle/vistasmokev1/vistasmokev1_emods_impactdust")
		};

	public:
		void draw(const gui_legacy::draw_adapter& draw) final;
		void draw_glow();

		void on_frame_render_start(const bool pre = true) const;
		void on_post_data_update() const;
		void set_alpha_for_third_person(const sdk::model_render_info_t& info) const;

		void on_player_hurt(sdk::game_event* event);
		void on_override_view(sdk::view_setup* setup, bool pre_original = false);

		void reset();

	private:
		void third_person(sdk::view_setup* setup);
		void indicators(const gui_legacy::draw_adapter& draw);
		void hitmarker(const gui_legacy::draw_adapter& draw);
		void scope(const gui_legacy::draw_adapter& draw) const;
		void pen_crosshair(const gui_legacy::draw_adapter& draw) const;
		void output_eventlog(const gui_legacy::draw_adapter& draw) const;

		float hitmarker_alpha{}, hitmarker_offset{}, fps{}, choke{}, desync{}, third_person_dist{};
		sdk::color hitmarker_color = sdk::color::white();
		bool last_no_smoke{};
		std::array<bool, 64> glow_enabled{};
	};

	extern std::shared_ptr<visuals> vis;
}

#endif // HACKS_VISUALS_H
