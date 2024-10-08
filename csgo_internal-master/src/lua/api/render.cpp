#include <lua/api_def.h>
#include <ren/renderer.h>
#include <lua/engine.h>
#include <ren/adapter.h>
#include <ren/misc.h>
#include <detail/dx_adapter.h>
#include <gui/renderer/bitfont.h>

using namespace evo;
using namespace lua;
using namespace detail;

namespace lua
{
	ren::color extract_color(state& s, int pos = 5)
	{
		return {
			s.get_field_integer(XOR_STR("r"), pos),
			s.get_field_integer(XOR_STR("g"), pos),
			s.get_field_integer(XOR_STR("b"), pos),
			s.get_field_integer(XOR_STR("a"), pos)
		};
	}
}

int api_def::render::create_font(lua_State *l)
{
	runtime_state s(l);

	const auto r = s.check_arguments({
		{ ltd::string },
		{ ltd::number },
		{ ltd::number, true },
		{ ltd::number, true },
		{ ltd::number, true },
	});

	if (!r)
	{
		s.error(XOR_STR("usage: create_font(font_path, size, flags = none, from = 0, to = 255): number"));
		return 0;
	}

	const auto me = api.find_by_state(l);
	if (!me)
	{
		s.error(XOR_STR("FATAL: could not find the script. Did it escape the sandbox?"));
		return 0;
	}

	const auto flags = s.get_stack_top() >= 3 ? s.get_integer(3) : 0;
	const auto from = s.get_stack_top() >= 4 ? s.get_integer(4) : 0;
	const auto to = s.get_stack_top() >= 5 ? s.get_integer(5) : 255;

	const auto next_free_id = ren::draw.get_free_font_slot();
	ren::draw.manage(next_free_id, std::make_shared<ren::font>(s.get_string(1), s.get_integer(2), flags, from, to));
	me->add_font(next_free_id);

	s.push(static_cast<int>(next_free_id));
	return 1;
}

int api_def::render::create_font_gdi(lua_State *l)
{
	runtime_state s(l);

	const auto r = s.check_arguments({
		{ ltd::string },
		{ ltd::number },
		{ ltd::number, true },
		{ ltd::number, true },
		{ ltd::number, true },
	});

	if (!r)
	{
		s.error(XOR_STR("usage: create_font_gdi(name, size, flags = none, from = 0, to = 255): number"));
		return 0;
	}

	const auto me = api.find_by_state(l);
	if (!me)
	{
		s.error(XOR_STR("FATAL: could not find the script. Did it escape the sandbox?"));
		return 0;
	}

	const auto flags = s.get_stack_top() >= 3 ? s.get_integer(3) : 0;
	const auto from = s.get_stack_top() >= 4 ? s.get_integer(4) : 0;
	const auto to = s.get_stack_top() >= 5 ? s.get_integer(5) : 255;

	const auto next_free_id = ren::draw.get_free_font_slot();
	ren::draw.manage(next_free_id, std::make_shared<::gui::bitfont>(s.get_string(1), s.get_integer(2), flags, from, to));
	me->add_font(next_free_id);

	s.push(static_cast<int>(next_free_id));
	return 1;
}

int api_def::render::create_texture(lua_State *l)
{
	runtime_state s(l);
	
	const auto r = s.check_arguments({
		{ ltd::string },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: create_texture(tex_path): number"));
		return 0;
	}
	
	const auto me = api.find_by_state(l);
	if (!me)
	{
		s.error(XOR_STR("FATAL: could not find the script. Did it escape the sandbox?"));
		return 0;
	}
	
	const auto next_free_id = ren::draw.get_free_texture_slot();
	ren::draw.manage(next_free_id, std::make_shared<ren::texture>(s.get_string(1)));
	me->add_texture(next_free_id);
	
	s.push(static_cast<int>(next_free_id));
	return 1;
}

int api_def::render::create_texture_bytes(lua_State *l)
{
	runtime_state s(l);

	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
	});

	if (!r)
	{
		s.error(XOR_STR("usage: create_texture_bytes(bytes, size): number"));
		return 0;
	}

	const auto me = api.find_by_state(l);
	if (!me)
	{
		s.error(XOR_STR("FATAL: could not find the script. Did it escape the sandbox?"));
		return 0;
	}

	const auto next_free_id = ren::draw.get_free_texture_slot();
	ren::draw.manage(next_free_id, std::make_shared<ren::texture>((void*)s.get_integer(1), s.get_integer(2)));
	me->add_texture(next_free_id);

	s.push(static_cast<int>(next_free_id));
	return 1;
}

int api_def::render::create_texture_rgba(lua_State *l)
{
	runtime_state s(l);

	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
	});

	if (!r)
	{
		s.error(XOR_STR("usage: create_texture_rgba(bytes, w, h, stride): number"));
		return 0;
	}

	const auto me = api.find_by_state(l);
	if (!me)
	{
		s.error(XOR_STR("FATAL: could not find the script. Did it escape the sandbox?"));
		return 0;
	}

	const auto next_free_id = ren::draw.get_free_texture_slot();
	ren::draw.manage(next_free_id, std::make_shared<ren::texture>((void*)s.get_integer(1), s.get_integer(2), s.get_integer(3), s.get_integer(4)));
	me->add_texture(next_free_id);

	s.push(static_cast<int>(next_free_id));
	return 1;
}

int api_def::render::push_texture(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}
	
	const auto r = s.check_arguments({
		{ ltd::number },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: push_texture(tex)"));
		return 0;
	}
	
	if (!ren::draw.has_texture(s.get_integer(1)))
	{
		s.error(XOR_STR("the requested texture is not loaded"));
		return 0;
	}

	buf.push_texture(ren::draw.get_texture(s.get_integer(1)));
	return 0;
}

int api_def::render::pop_texture(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}

	buf.pop_texture();
	return 0;
}

int api_def::render::push_clip_rect(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}
	
	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::boolean, true },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: push_clip_rect(x1, y1, x2, y2, intersect = true)"));
		return 0;
	}

	const auto intersect = s.get_stack_top() == 5 ? s.get_boolean(5) : true;

	buf.push_clip({ s.get_float(1), s.get_float(2), s.get_float(3), s.get_float(4) }, intersect);
	return 0;
}

int api_def::render::pop_clip_rect(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}

	buf.pop_clip();
	return 0;
}

int api_def::render::push_uv(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}
	
	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: push_uv(x1, y1, x2, y2)"));
		return 0;
	}

	buf.push_uv({ s.get_float(1), s.get_float(2), s.get_float(3), s.get_float(4) });
	return 0;
}

int api_def::render::pop_uv(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}

	buf.pop_uv();
	return 0;
}

int api_def::render::create_animator_color(lua_State *l)
{
	runtime_state s(l);
	
	const auto r = s.check_arguments({
		{ ltd::table },
		{ ltd::number },
		{ ltd::number, true },
		{ ltd::boolean, true },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: create_animator_color(initial, time, interpolation = linear, interp_hue = false): anim_color"));
		return 0;
	}
	
	const auto me = api.find_by_state(l);
	if (!me)
	{
		s.error(XOR_STR("FATAL: could not find the script. Did it escape the sandbox?"));
		return 0;
	}
	
	const auto interp = s.get_stack_top() >= 3 ? s.get_integer(3) : ren::i_linear;
	const auto is_hue = s.get_stack_top() >= 4 ? s.get_boolean(4) : false;
	
	const auto anim = std::make_shared<ren::anim_color>(extract_color(s, 1), s.get_float(2), interp);
	anim->type = is_hue ? ren::act_hsva : ren::act_rgba;
	
	const auto next_free_id = ren::draw.get_free_anim_slot();
	ren::draw.manage(next_free_id, anim);
	me->add_animator(next_free_id);
	
	std::weak_ptr<ren::anim_color> obj{anim};
	s.create_user_object<decltype(obj)>(XOR_STR("render.anim_color"), &obj);
	
	return 1;
}

int api_def::render::anim_color_direct(lua_State *l)
{
	runtime_state s(l);
	
	const auto r = s.check_arguments({
		{ ltd::user_data },
		{ ltd::table },
		{ ltd::table, true },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: obj:direct(to) or obj:direct(from, to)"));
		return 0;
	}
	
	const auto obj = *reinterpret_cast<std::weak_ptr<ren::anim_color>*>(s.get_user_data(1));
	const auto anim = obj.lock();
	if (!anim)
	{
		s.error(XOR_STR("invalid animator"));
		return 0;
	}
	
	if (s.get_stack_top() == 3)
		anim->direct(extract_color(s, 2), extract_color(s, 3));
	else
		anim->direct(extract_color(s, 2));
	
	return 0;
}

int api_def::render::anim_color_get_value(lua_State *l)
{
	runtime_state s(l);
	
	const auto r = s.check_arguments({
	{ ltd::user_data },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: obj:get_value()"));
		return 0;
	}
	
	const auto obj = *reinterpret_cast<std::weak_ptr<ren::anim_color>*>(s.get_user_data(1));
	const auto anim = obj.lock();
	if (!anim)
	{
		s.error(XOR_STR("invalid animator"));
		return 0;
	}
	
	s.create_table();
	s.set_field(XOR_STR("r"), anim->value.r());
	s.set_field(XOR_STR("g"), anim->value.g());
	s.set_field(XOR_STR("b"), anim->value.b());
	s.set_field(XOR_STR("a"), anim->value.a());
	
	return 1;
}

int api_def::render::create_animator_float(lua_State *l)
{
	runtime_state s(l);
	
	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number, true },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: create_animator_float(initial, time, interpolation = linear): anim_float"));
		return 0;
	}
	
	const auto me = api.find_by_state(l);
	if (!me)
	{
		s.error(XOR_STR("FATAL: could not find the script. Did it escape the sandbox?"));
		return 0;
	}
	
	const auto interp = s.get_stack_top() == 3 ? s.get_integer(3) : ren::i_linear;
	const auto anim = std::make_shared<ren::anim_float>(s.get_float(1), s.get_float(2), interp);
	
	const auto next_free_id = ren::draw.get_free_anim_slot();
	ren::draw.manage(next_free_id, anim);
	me->add_animator(next_free_id);
	
	std::weak_ptr<ren::anim_float> obj{anim};
	s.create_user_object<decltype(obj)>(XOR_STR("render.anim_float"), &obj);
	
	return 1;
}

int api_def::render::anim_float_direct(lua_State *l)
{
	runtime_state s(l);
	
	const auto r = s.check_arguments({
		{ ltd::user_data },
		{ ltd::number },
		{ ltd::number, true },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: obj:direct(to) or obj:direct(from, to)"));
		return 0;
	}
	
	const auto obj = *reinterpret_cast<std::weak_ptr<ren::anim_float>*>(s.get_user_data(1));
	const auto anim = obj.lock();
	if (!anim)
	{
		s.error(XOR_STR("invalid animator"));
		return 0;
	}
	
	if (s.get_stack_top() == 3)
		anim->direct(s.get_float(2), s.get_float(3));
	else
		anim->direct(s.get_float(2));
	
	return 0;
}

int api_def::render::anim_float_get_value(lua_State *l)
{
	runtime_state s(l);
	
	const auto r = s.check_arguments({
		{ ltd::user_data },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: obj:get_value()"));
		return 0;
	}
	
	const auto obj = *reinterpret_cast<std::weak_ptr<ren::anim_float>*>(s.get_user_data(1));
	const auto anim = obj.lock();
	if (!anim)
	{
		s.error(XOR_STR("invalid animator"));
		return 0;
	}
	
	s.push(anim->value);
	return 1;
}

int api_def::render::get_texture_size(lua_State *l)
{
	runtime_state s(l);
	
	const auto r = s.check_arguments({
		{ ltd::number },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: get_texture_size(tex): number, number"));
		return 0;
	}
	
	if (!ren::draw.has_texture(s.get_integer(1)))
	{
		s.error(XOR_STR("the requested texture is not loaded"));
		return 0;
	}
	
	const auto tex = ren::draw.get_texture(s.get_integer(1));
	const auto size = tex->size();
	
	s.push(size.x);
	s.push(size.y);
	
	return 2;
}

int api_def::render::get_text_size(lua_State *l)
{
	runtime_state s(l);

	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::string }
    });

	if (!r)
	{
		s.error(XOR_STR("usage: get_text_size(font, text): number, number"));
		return 0;
	}

	if (!ren::draw.has_font(s.get_integer(1)))
	{
		s.error(XOR_STR("the requested font is not loaded"));
		return 0;
	}

	const auto font = ren::draw.get_font(s.get_integer(1));
	const auto size = ren::text_size(font, s.get_string(2));
	
	s.push(size.x);
	s.push(size.y);
	
	return 2;
}

int api_def::render::color(lua_State *l)
{
	runtime_state s(l);

	auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number, true }
	});

	int cr, cg, cb, ca;
	if (!r)
	{
		r = s.check_arguments({
			{ ltd::string }
		});

		if (!r)
		{
			s.error(XOR_STR("usage: color(r, g, b, a = 255) or color(hex)"));
			return 0;
		}

		const auto parse_c = sscanf_s(s.get_string(1), XOR_STR("#%02x%02x%02x%02x"), &cr, &cg, &cb, &ca);
		if (parse_c < 3)
		{
			s.error(XOR_STR("invalid hex color (example: #00acf5, format: #RRGGBBAA)"));
			return 0;
		}

		if (parse_c != 4)
			ca = 255;
	} else
	{
		cr = s.get_integer(1);
		cg = s.get_integer(2);
		cb = s.get_integer(3);
		ca = s.get_stack_top() == 4 ? s.get_integer(4) : 255;
	}

	s.create_table();
	s.set_field(XOR_STR("r"), cr);
	s.set_field(XOR_STR("g"), cg);
	s.set_field(XOR_STR("b"), cb);
	s.set_field(XOR_STR("a"), ca);

	return 1;
}

int api_def::render::rect_filled(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}

	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::table }
	});

	if (!r)
	{
		s.error(XOR_STR("usage: rect_filled(x1, y1, x2, y2, color: { r, g, b, a })"));
		return 0;
	}

	buf.add_rect_filled(
	ren::rect(s.get_float(1), s.get_float(2), s.get_float(3), s.get_float(4)),
		extract_color(s)
	);

	return 0;
}

int api_def::render::rect_filled_multicolor(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}
	
	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::table },
		{ ltd::table },
		{ ltd::table },
		{ ltd::table },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: rect_filled_multicolor(x1, y1, x2, y2, tl, tr, br, bl)"));
		return 0;
	}
	
	buf.add_rect_filled_multicolor(ren::rect{s.get_float(1), s.get_float(2), s.get_float(3), s.get_float(4)}, {
		extract_color(s, 5),
		extract_color(s, 6),
		extract_color(s, 7),
		extract_color(s, 8),
	});
	
	return 0;
}

int api_def::render::line(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}
	
	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::table },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: line(x1, y1, x2, y2, color)"));
		return 0;
	}
	
	buf.add_line({ s.get_float(1), s.get_float(2) }, { s.get_float(3), s.get_float(4) }, extract_color(s));
	return 0;
}

int api_def::render::line_multicolor(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}
	
	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::table },
		{ ltd::table },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: line_multicolor(x1, y1, x2, y2, col1, col2)"));
		return 0;
	}
	
	buf.add_line_multicolor({ s.get_float(1), s.get_float(2) }, { s.get_float(3), s.get_float(4) }, {
		extract_color(s, 5),
		extract_color(s, 6),
	});
	return 0;
}

int api_def::render::triangle(lua_State* l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}
	
	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::table },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: triangle(x1, y1, x2, y2, x3, y3, color)"));
		return 0;
	}
	
	buf.add_triangle({ s.get_float(1), s.get_float(2) }, { s.get_float(3), s.get_float(4) }, { s.get_float(5), s.get_float(6) }, extract_color(s, 7));
	return 0;
}

int api_def::render::triangle_filled(lua_State* l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}
	
	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::table },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: triangle_filled(x1, y1, x2, y2, x3, y3, color)"));
		return 0;
	}
	
	buf.add_triangle_filled({ s.get_float(1), s.get_float(2) }, { s.get_float(3), s.get_float(4) }, { s.get_float(5), s.get_float(6) }, extract_color(s, 7));
	return 0;
}

int api_def::render::triangle_filled_multicolor(lua_State* l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}
	
	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::table },
		{ ltd::table },
		{ ltd::table },
	});
	
	if (!r)
	{
		s.error(XOR_STR("usage: triangle_filled(x1, y1, x2, y2, x3, y3, col1, col2, col3)"));
		return 0;
	}
	
	buf.add_triangle_filled_multicolor({ s.get_float(1), s.get_float(2) }, { s.get_float(3), s.get_float(4) }, { s.get_float(5), s.get_float(6) }, {
		extract_color(s, 7),
		extract_color(s, 8),
		extract_color(s, 9),
	});
	
	return 0;
}

int api_def::render::rect(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}

	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::table }
	});

	if (!r)
	{
		s.error(XOR_STR("usage: rect(x1, y1, x2, y2, color: { r, g, b, a })"));
		return 0;
	}

	buf.add_rect(
		ren::rect(s.get_float(1), s.get_float(2), s.get_float(3), s.get_float(4)),
		extract_color(s)
	);

	return 0;
}

int api_def::render::rect_filled_rounded(lua_State* l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}

	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::table },
		{ ltd::number },
		{ ltd::number, true }
	});

	if (!r)
	{
		s.error(XOR_STR("usage: rect_filled_rounded(x1, y1, x2, y2, color: { r, g, b, a }, rounding, sides = all)"));
		return 0;
	}

	const auto rounding = static_cast<char>(s.get_stack_top() == 7 ? s.get_integer(7) : ren::layer::rnd_all);

	buf.push_aa(true);
	buf.add_rect_filled_rounded(
		ren::rect(s.get_float(1), s.get_float(2), s.get_float(3), s.get_float(4)),
		extract_color(s), s.get_float(6), rounding
	);
	buf.pop_aa();

	return 0;
}

int api_def::render::text(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}

	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::string },
		{ ltd::table },
		{ ltd::number, true },
		{ ltd::number, true },
	});

	if (!r)
	{
		s.error(XOR_STR("usage: text(font, x, y, text, color: { r, g, b, a }, align_h = left, align_v = top)"));
		return 0;
	}

	if (!ren::draw.has_font(s.get_integer(1)))
	{
		s.error(XOR_STR("the requested font is not loaded"));
		return 0;
	}

	const auto font = ren::draw.get_font(s.get_integer(1));

	char ah, av;
	if (s.get_stack_top() > 5)
	{
		if (s.get_stack_top() == 6)
		{
			ah = static_cast<char>(s.get_integer(6));
			av = ren::layer::align_top;
		}
		else
		{
			ah = static_cast<char>(s.get_integer(6));
			av = static_cast<char>(s.get_integer(7));
		}
	} else
	{
		ah = ren::layer::align_left;
		av = ren::layer::align_top;
	}

	buf.add_text(font, { s.get_float(2), s.get_float(3) },
			   s.get_string(4), extract_color(s), ah, av);

	return 0;
}

int lua::api_def::render::get_screen_size(lua_State *l)
{
	runtime_state s(l);

	const auto sz = ren::adapter->display;
	s.push(sz.x);
	s.push(sz.y);

	return 2;
}

int lua::api_def::render::circle(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}

	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::table },
		{ ltd::number, true },
		{ ltd::number, true },
		{ ltd::number, true },
		{ ltd::number, true },
	});

	if (!r)
	{
		s.error(XOR_STR("usage: circle(x, y, radius, color: { r, g, b, a }, segments = 12, fill = 1.0, rot = 0.0)"));
		return 0;
	}
	
	const auto thickness = s.get_stack_top() >= 5 ? s.get_integer(5) : 1.f;
	const auto segments = s.get_stack_top() >= 6 ? s.get_integer(6) : 12;
	const auto fill = s.get_stack_top() >= 7 ? s.get_float(7) : 1.f;
	const auto rot = s.get_stack_top() >= 8 ? s.get_float(8) : 0.f;

	buf.push_aa(true);
	buf.add_circle({ s.get_float(1), s.get_float(2) }, s.get_float(3),
		extract_color(s, 4), thickness, segments, fill, rot);
	buf.pop_aa();
	
	return 0;
}

int lua::api_def::render::circle_filled(lua_State *l)
{
	runtime_state s(l);
	if (!api.in_render)
	{
		s.error(XOR_STR("this function can only be called from on_paint()!"));
		return 0;
	}

	const auto r = s.check_arguments({
		{ ltd::number },
		{ ltd::number },
		{ ltd::number },
		{ ltd::table },
		{ ltd::number, true },
		{ ltd::number, true },
		{ ltd::number, true },
	});

	if (!r)
	{
		s.error(XOR_STR("usage: circle_filled(x, y, radius, color: { r, g, b, a }, segments = 12, fill = 1.0, rot = 0.0)"));
		return 0;
	}

	const auto segments = s.get_stack_top() >= 5 ? s.get_integer(5) : 12;
	const auto fill = s.get_stack_top() >= 6 ? s.get_float(6) : 1.f;
	const auto rot = s.get_stack_top() >= 7 ? s.get_float(7) : 0.f;

	buf.push_aa(true);
	buf.add_circle_filled({ s.get_float(1), s.get_float(2) }, s.get_float(3),
		extract_color(s, 4), segments, fill, rot);
	buf.pop_aa();

	return 0;
}