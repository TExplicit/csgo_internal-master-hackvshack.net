#include <lua/engine.h>
#include <util/fnv1a.h>
#include <filesystem>
#include <sdk/surface.h>
#include <gui/controls.h>
#include <util/misc.h>
#include <base/game.h>

#include <fstream>

using namespace evo;

void lua::engine::refresh_scripts()
{
	if (!std::filesystem::exists(XOR_STR("ev0lve/scripts")))
		std::filesystem::create_directories(XOR_STR("ev0lve/scripts"));
	if (!std::filesystem::exists(XOR_STR("ev0lve/scripts/remote")))
		std::filesystem::create_directories(XOR_STR("ev0lve/scripts/remote"));
	if (!std::filesystem::exists(XOR_STR("ev0lve/scripts/lib")))
		std::filesystem::create_directories(XOR_STR("ev0lve/scripts/lib"));
	
	script_files.clear();
	
	// TODO: template this?
	for (auto& f : std::filesystem::directory_iterator(XOR_STR("ev0lve/scripts")))
	{
		if (f.is_directory() || f.path().extension() != XOR_STR(".lua"))
			continue;
		
		script_file file{
			st_script,
			f.path().filename().replace_extension(XOR_STR("")).string()
		};
		
		file.parse_metadata();
		script_files.emplace_back(file);
	}
	
	for (auto& f : std::filesystem::directory_iterator(XOR_STR("ev0lve/scripts/remote")))
	{
		if (f.is_directory() || f.path().extension() != XOR_STR(".lua"))
			continue;
		
		script_file file{
			st_remote,
			f.path().filename().replace_extension(XOR_STR("")).string()
		};
		
		file.parse_metadata();
		script_files.emplace_back(file);
	}
	
	for (auto& f : std::filesystem::directory_iterator(XOR_STR("ev0lve/scripts/lib")))
	{
		if (f.is_directory() || f.path().extension() != XOR_STR(".lua"))
			continue;
		
		script_file file{
			st_library,
			f.path().filename().replace_extension(XOR_STR("")).string()
		};
		
		file.parse_metadata();
		script_files.emplace_back(file);
	}
	
	if (!std::filesystem::exists(XOR_STR("ev0lve/autoload")))
		return;
	
	std::ifstream al_file(XOR_STR("ev0lve/autoload"), std::ios::binary);
	if (!al_file.is_open())
		return;
	
	while (!al_file.eof())
	{
		uint32_t alh{};
		al_file.read(reinterpret_cast<char*>(&alh), sizeof(alh));
		autoload.emplace_back(alh);
	}
}

void lua::engine::run_autoload()
{
	for (const auto& f_id : autoload)
	{
		const auto file = std::find_if(script_files.begin(), script_files.end(), [f_id](const script_file& f) {
			return f.make_id() == f_id;
		});
		
		if (file != script_files.end())
			run_script(*file, false);
	}

	gui::ctx->find(GUI_HASH("scripts.general.list"))->as<gui::list>()->for_each_control([&](std::shared_ptr<gui::control>& c){
		const auto sel = c->as<gui::selectable>();
		if (lua::api.exists(sel->id))
			sel->is_loaded = true;
		else
			sel->is_loaded = false;

		sel->reset();
	});
}

void lua::engine::enable_autoload(const script_file &file)
{
	if (std::find(autoload.begin(), autoload.end(), file.make_id()) != autoload.end())
		return;
	
	autoload.emplace_back(file.make_id());
	write_autoload();
}

void lua::engine::disable_autoload(const script_file &file)
{
	if (const auto f = std::find(autoload.begin(), autoload.end(), file.make_id()); f != autoload.end())
		autoload.erase(f);
	
	write_autoload();
}

bool lua::engine::is_autoload_enabled(const script_file &file)
{
	return std::find(autoload.begin(), autoload.end(), file.make_id()) != autoload.end();
}

void lua::engine::write_autoload()
{
	if (std::filesystem::exists(XOR_STR("ev0lve/autoload")))
		std::filesystem::remove(XOR_STR("ev0lve/autoload"));
	
	std::ofstream al_file(XOR_STR("ev0lve/autoload"), std::ios::binary);
	if (!al_file.is_open())
		return;

    // eliminate same scripts from the list
    std::vector<uint32_t> fixed_list{};
    for (const auto& alh : autoload)
    {
        if (std::find(fixed_list.begin(), fixed_list.end(), alh) == fixed_list.end())
            fixed_list.emplace_back(alh);
    }

    autoload = fixed_list;
	for (const auto& alh : autoload)
		al_file.write(reinterpret_cast<const char*>(&alh), sizeof(alh));
}

bool lua::engine::run_script(const script_file &file, bool sounds)
{
	if (file.type == st_library)
		return false;
	
	std::lock_guard _lock(access_mutex);

	// make sure file still exists
	if (!std::filesystem::exists(file.get_file_path()))
	{
		if (sounds)
			EMIT_ERROR_SOUND();

		// show the error message to user
		const auto msg = std::make_shared<gui::message_box>(FNV1A("script.error"), XOR_STR("Script error"), XOR_STR("File not found."));
		msg->open();

		return false;
	}

	// delete previously running script
	const auto id = file.make_id();
	if (scripts.find(id) != scripts.end())
		scripts.erase(id);

	// create script (lua will auto-initialize)
	const auto s = std::make_shared<script>();
	s->name = file.name;
	s->id = id;
	s->type = file.type;
	s->file = file.get_file_path();

	// add to the list
	scripts[s->id] = s;

	// init the script
	try {
		s->initialize();
		s->call_main();
	} catch (const std::exception& ex) {
		if (sounds)
			EMIT_ERROR_SOUND();

		// show the error message to user
		const auto msg = std::make_shared<gui::message_box>(FNV1A("script.error"), XOR_STR("Script error"), ex.what());
		msg->open();

		return false;
	}

	// mark as running
	s->is_running = true;

	if (sounds)
		EMIT_SUCCESS_SOUND();

	return true;
}

void lua::engine::stop_script(const script_file &file)
{
	std::lock_guard _lock(access_mutex);

	// "unload" script (lua state will be auto-erased)
	const auto id = file.make_id();
	if (const auto s = scripts.find(id); s != scripts.end())
	{
		s->second->call_forward(FNV1A("on_shutdown"));
		scripts.erase(id);
	}
}

void lua::engine::for_each_script_name(const std::function<void(const script_file&)> &fn)
{
	for (const auto& f : script_files)
	{
		if (f.type != st_library)
			fn(f);
	}
}

bool lua::engine::run_library(const script_file &file)
{
	if (file.type != st_library)
		return false;
	
	std::lock_guard _lock(library_mutex);
	
	// avoid re-running libraries
	const auto id = file.make_id();
	if (libraries.find(id) != libraries.end())
	{
		const auto& lib = libraries[id];
		if (lib->is_running)
			return true;
		else
			libraries.erase(id);
	}
	
	// create lib (lua will auto-initialize)
	const auto s = std::make_shared<library>();
	s->name = file.name;
	s->id = id;
	s->type = file.type;
	s->file = file.get_file_path();
	
	// add to the list
	libraries[s->id] = s;
	
	// init the script
	try {
		s->initialize();
		s->call_main();
	} catch (const std::exception& ex) {
		return false;
	}
	
	// mark as running
	s->is_running = true;
	
	return true;
}

void lua::engine::stop_library(uint32_t id)
{
	std::lock_guard _lock(library_mutex);
	
	// "unload" lib (lua state will be auto-erased)
	if (libraries.find(id) != libraries.end())
		libraries.erase(id);
}

void lua::engine::callback(uint32_t id, const std::function<int(state &)> &arg_callback)
{
	std::lock_guard _lock(access_mutex);
	for (const auto& [_, s] : scripts)
	{
		if (s->is_running)
			s->call_forward(id, arg_callback);
	}
	
	std::lock_guard _lib(library_mutex);
	for (const auto& [_, s] : libraries)
	{
		if (s->is_running)
			s->call_forward(id, arg_callback);
	}
}

void lua::engine::create_callback(const char *n)
{
	std::lock_guard _lock(access_mutex);
	for (const auto& [_, s] : scripts)
	{
		if (!s->has_forward(util::fnv1a(n)))
			s->create_forward(n);
	}
	
	std::lock_guard _lib(library_mutex);
	for (const auto& [_, s] : libraries)
	{
		if (!s->has_forward(util::fnv1a(n)))
			s->create_forward(n);
	}
}

void lua::engine::stop_all()
{
	std::lock_guard _lock(access_mutex);
	scripts.clear();
	
	std::lock_guard _lib(library_mutex);
	libraries.clear();
}

void lua::engine::run_timers()
{
	std::lock_guard _lock(access_mutex);
	for (const auto& [_, s] : scripts)
	{
		if (s->is_running)
			s->run_timers();
	}

	std::lock_guard _lib(library_mutex);
	for (const auto& [_, s] : libraries)
	{
		if (s->is_running)
			s->run_timers();
	}
}

lua::script_t lua::engine::find_by_state(lua_State *state) {
	for (const auto& [_, s] : scripts)
	{
		if (s->l.get_state() == state)
			return s;
	}
	
	for (const auto& [_, s] : libraries)
	{
		if (s->l.get_state() == state)
			return s;
	}
	
	return nullptr;
}

lua::script_t lua::engine::find_by_id(uint32_t id)
{
	for (const auto& [_, s] : scripts)
	{
		if (_ == id)
			return s;
	}
	
	for (const auto& [_, s] : libraries)
	{
		if (_ == id)
			return s;
	}
	
	return nullptr;
}

uint32_t lua::script_file::make_id() const
{
	return util::fnv1a(name.c_str()) ^ type;
}

std::string lua::script_file::get_file_path() const
{
	std::string base_path{XOR_STR("ev0lve/")};
	switch (type)
	{
		case st_script:
			base_path += XOR_STR("scripts/");
			break;
		case st_remote:
			base_path += XOR_STR("scripts/remote/");
			break;
		case st_library:
			base_path += XOR_STR("scripts/lib/");
			break;
		default:
			return XOR_STR("");
	}
	
	return base_path + name + XOR_STR(".lua");
}

void lua::script_file::parse_metadata()
{
	std::ifstream file(get_file_path());
	if (!file.is_open())
		return;
	
	std::string line{};
	while (std::getline(file, line))
	{
		// check our own shebang notation
		if (line.find(XOR_STR("--.")) != 0)
			continue;
		
		// remove shebang
		line = line.erase(0, 3);
		
		// split in parts
		const auto parts = util::split(line, XOR_STR(" "));
		if (parts.empty())
			continue;
		
		const auto item = util::fnv1a(parts[0].c_str());
		switch (item)
		{
			case FNV1A("name"):
				metadata.name = line.substr(5);
				break;
			case FNV1A("description"):
				metadata.description = line.substr(12);
				break;
			case FNV1A("author"):
				metadata.author = line.substr(7);
				break;
			case FNV1A("use_state"):
				metadata.use_state = true;
				break;
			default: break;
		}
	}
}