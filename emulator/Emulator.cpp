#include "Emulator.hpp"

#include "Chipset/Chipset.hpp"
#include "Logger.hpp"
#include "Data/EventCode.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <cassert>

namespace casioemu
{
	Emulator::Emulator(std::map<std::string, std::string> &_argv_map, bool _paused) : paused(_paused), argv_map(_argv_map), chipset(*new Chipset(*this))
	{
		std::lock_guard<decltype(access_mx)> access_lock(access_mx);
		running = true;
		model_path = argv_map["model"];

		lua_state = luaL_newstate();
		luaL_openlibs(lua_state);

		SetupLuaAPI();
		LoadModelDefition();

		int hardware_id = GetModelInfo("hardware_id");
		if (hardware_id != HW_ES_PLUS && hardware_id != HW_CLASSWIZ && hardware_id != HW_CLASSWIZ_II)
			PANIC("Unknown hardware id %d\n", hardware_id);
		this->hardware_id = (HardwareId)hardware_id;

		unsigned int cycles_per_second = hardware_id == HW_ES_PLUS ? 128 * 1024 : hardware_id == HW_CLASSWIZ ? 1024 * 1024 : 2048 * 1024;
		timer_interval = hardware_id == HW_CLASSWIZ_II ? 10 : 20;

		cycles.Setup(cycles_per_second, timer_interval);
		chipset.Setup();

		interface_background = GetModelInfo("rsd_interface");
		if (interface_background.dest.x != 0 || interface_background.dest.y != 0)
			PANIC("rsd_interface must have dest x and y coordinate zero\n");

		width = interface_background.dest.w;
		height = interface_background.dest.h;
		try
		{
			std::size_t pos;

			auto width_iter = argv_map.find("width");
			if (width_iter != argv_map.end())
			{
				width = std::stoi(width_iter->second, &pos, 0);
				if (pos != width_iter->second.size())
					PANIC("width parameter has extraneous trailing characters\n");
			}

			auto height_iter = argv_map.find("height");
			if (height_iter != argv_map.end())
			{
				height = std::stoi(height_iter->second, &pos, 0);
				if (pos != height_iter->second.size())
					PANIC("height parameter has extraneous trailing characters\n");
			}
		}
		catch (std::invalid_argument const&)
		{
			PANIC("invalid width/height parameter\n");
		}
		catch (std::out_of_range const&)
		{
			PANIC("out of range width/height parameter\n");
		}

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
		window = SDL_CreateWindow(
			std::string(GetModelInfo("model_name")).c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width, height,
			SDL_WINDOW_SHOWN |
			(SDL_WINDOW_RESIZABLE)
		);
		if (!window)
			PANIC("SDL_CreateWindow failed: %s\n", SDL_GetError());
		renderer = SDL_CreateRenderer(window, -1, 0);
		if (!renderer)
			PANIC("SDL_CreateRenderer failed: %s\n", SDL_GetError());

		SDL_Surface *loaded_surface = IMG_Load(GetModelFilePath(GetModelInfo("interface_image_path")).c_str());
		if (!loaded_surface)
			PANIC("IMG_Load failed: %s\n", IMG_GetError());
		interface_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
		SDL_FreeSurface(loaded_surface);

		SetupInternals();
		cycles.Reset();

		tick_thread = new std::thread([this] {
			auto iteration_end = std::chrono::steady_clock::now();
			while (1)
			{
				{
					std::lock_guard<decltype(access_mx)> access_lock(access_mx);
					if (!Running())
						break;
					TimerCallback();
				}

				iteration_end += std::chrono::milliseconds(timer_interval);
				auto now = std::chrono::steady_clock::now();
				if (iteration_end > now)
					std::this_thread::sleep_until(iteration_end);
				else // in case the computer is not fast enough or paused
					iteration_end = now;
			}
		});

		RunStartupScript();

		chipset.Reset();

		if (argv_map.find("paused") != argv_map.end())
			SetPaused(true);

		pause_on_mem_error = argv_map.find("pause_on_mem_error") != argv_map.end();
	}

	Emulator::~Emulator()
	{
		if (tick_thread->joinable())
			tick_thread->join();
		delete tick_thread;
		
		std::lock_guard<decltype(access_mx)> access_lock(access_mx);

		SDL_DestroyTexture(interface_texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		luaL_unref(lua_state, LUA_REGISTRYINDEX, lua_model_ref);
		lua_close(lua_state);
		delete &chipset;
	}

	void Emulator::HandleMemoryError()
	{
		if (pause_on_mem_error)
		{
			logger::Info("execution paused due to memory error\n");
			SetPaused(true);
		}
	}

	void Emulator::UIEvent(SDL_Event &event)
	{
		std::lock_guard<decltype(access_mx)> access_lock(access_mx);

		// For mouse events, rescale the coordinates from window size to original size.
		switch (event.type)
		{
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			event.button.x *= (float) interface_background.dest.w / width;
			event.button.y *= (float) interface_background.dest.h / height;
			break;
		case SDL_MOUSEMOTION:
			event.motion.x *= (float) interface_background.dest.w / width;
			event.motion.y *= (float) interface_background.dest.h / height;
			event.motion.xrel *= (float) interface_background.dest.w / width;
			event.motion.yrel *= (float) interface_background.dest.h / height;
			break;
		case SDL_MOUSEWHEEL:
			event.wheel.x *= (float) interface_background.dest.w / width;
			event.wheel.y *= (float) interface_background.dest.h / height;
			break;
		}
		chipset.UIEvent(event);
	}

	void Emulator::RunStartupScript()
	{
		if (argv_map.find("script") == argv_map.end())
			return;

		if (luaL_loadfile(lua_state, argv_map["script"].c_str()) != LUA_OK)
		{
			logger::Info("%s\n", lua_tostring(lua_state, -1));
			lua_pop(lua_state, 1);
			return;
		}

		if (lua_pcall(lua_state, 0, 1, 0) != LUA_OK)
		{
			logger::Info("%s\n", lua_tostring(lua_state, -1));
			lua_pop(lua_state, 1);
			return;
		}
	}

	void Emulator::SetupLuaAPI()
	{
		*(Emulator **)lua_newuserdata(lua_state, sizeof(Emulator *)) = this;
		lua_newtable(lua_state);
		lua_newtable(lua_state);
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			Emulator *emu = *(Emulator **)lua_topointer(lua_state, 1);
			emu->Tick();
			return 0;
		});
		lua_setfield(lua_state, -2, "tick");
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			Emulator *emu = *(Emulator **)lua_topointer(lua_state, 1);
			emu->Shutdown();
			return 0;
		});
		lua_setfield(lua_state, -2, "shutdown");
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			Emulator *emu = *(Emulator **)lua_topointer(lua_state, 1);
			emu->SetPaused(lua_toboolean(lua_state, 2));
			return 0;
		});
		lua_setfield(lua_state, -2, "set_paused");
		lua_model_ref = LUA_REFNIL;
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			Emulator *emu = *(Emulator **)lua_topointer(lua_state, 1);
			switch (lua_gettop(lua_state))
			{
			case 1:
				// emu:model() returns the model table
				lua_geti(lua_state, LUA_REGISTRYINDEX, emu->lua_model_ref);
				return 1;

			case 2:
				// emu:model(t) sets the model table
				if (emu->lua_model_ref != LUA_REFNIL)
					PANIC("emu.model invoked twice\n");
				emu->lua_model_ref = luaL_ref(lua_state, LUA_REGISTRYINDEX);
				return 0;

			default:
				PANIC("Invalid number of arguments (%d)\n", lua_gettop(lua_state));
			}
		});
		lua_setfield(lua_state, -2, "model");
		lua_pre_tick_ref = LUA_REFNIL;
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			Emulator *emu = *(Emulator **)lua_topointer(lua_state, 1);
			luaL_unref(lua_state, LUA_REGISTRYINDEX, emu->lua_pre_tick_ref);
			emu->lua_pre_tick_ref = luaL_ref(lua_state, LUA_REGISTRYINDEX);
			return 0;
		});
		lua_setfield(lua_state, -2, "pre_tick");
		lua_post_tick_ref = LUA_REFNIL;
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			Emulator *emu = *(Emulator **)lua_topointer(lua_state, 1);
			luaL_unref(lua_state, LUA_REGISTRYINDEX, emu->lua_post_tick_ref);
			emu->lua_post_tick_ref = luaL_ref(lua_state, LUA_REGISTRYINDEX);
			return 0;
		});
		lua_setfield(lua_state, -2, "post_tick");
		lua_setfield(lua_state, -2, "__index");
		lua_pushcfunction(lua_state, [](lua_State *) {
			return 0;
		});
		lua_setfield(lua_state, -2, "__newindex");
		lua_setmetatable(lua_state, -2);
		lua_setglobal(lua_state, "emu");
	}

	void Emulator::SetupInternals()
	{
		chipset.SetupInternals();
	}

	void Emulator::LoadModelDefition()
	{
		if (luaL_loadfile(lua_state, (model_path + "/model.lua").c_str()) != LUA_OK)
			PANIC("LoadModelDefition failed: %s\n", lua_tostring(lua_state, -1));

		if (lua_pcall(lua_state, 0, 0, 0) != LUA_OK)
			PANIC("LoadModelDefition failed: %s\n", lua_tostring(lua_state, -1));

		if (lua_model_ref == LUA_REFNIL)
			PANIC("LoadModelDefition failed: model failed to call emu.model\n");
	}

	std::string Emulator::GetModelFilePath(std::string relative_path)
	{
		std::cout<<model_path<<"/"<<relative_path;
		return model_path + "/" + relative_path;
	}

	void Emulator::TimerCallback()
	{
		std::lock_guard<decltype(access_mx)> access_lock(access_mx);

		Uint64 cycles_to_emulate = cycles.GetDelta();
		for (Uint64 ix = 0; ix != cycles_to_emulate; ++ix)
			if (!paused)
				Tick();

		if (chipset.GetRequireFrame())
		{
			SDL_Event event;
			SDL_zero(event);
			event.type = SDL_USEREVENT;
			event.user.code = CE_FRAME_REQUEST;
			SDL_PushEvent(&event);
		}
	}

	void Emulator::Repaint()
	{
		SDL_RenderPresent(renderer);
	}

	void Emulator::Frame()
	{
		std::lock_guard<decltype(access_mx)> access_lock(access_mx);

		// create texture `tx` with the same format as `interface_texture`
		Uint32 format;
		SDL_QueryTexture(interface_texture, &format, nullptr, nullptr, nullptr);
		SDL_Texture* tx = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_TARGET, interface_background.dest.w, interface_background.dest.h);

		// render on `tx`
		SDL_SetRenderTarget(renderer, tx);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);
		SDL_SetTextureColorMod(interface_texture, 255, 255, 255);
		SDL_SetTextureAlphaMod(interface_texture, 255);
		SDL_RenderCopy(renderer, interface_texture, &interface_background.src, nullptr);
		chipset.Frame();

		// resize and copy `tx` to screen
		SDL_SetRenderTarget(renderer, nullptr);
		SDL_Rect dest {0, 0, width, height};
		SDL_RenderCopy(renderer, tx, nullptr, &dest);
		SDL_DestroyTexture(tx);
		Repaint();
	}

	void Emulator::WindowResize(int _width, int _height)
	{
		std::lock_guard<decltype(access_mx)> access_lock(access_mx);
		width = _width;
		height = _height;
		Frame();
	}

	void Emulator::Tick()
	{
		if (lua_pre_tick_ref != LUA_REFNIL)
		{
			lua_geti(lua_state, LUA_REGISTRYINDEX, lua_pre_tick_ref);
			if (lua_pcall(lua_state, 0, 0, 0) != LUA_OK)
			{
				logger::Info("pre-tick hook failed: %s\n", lua_tostring(lua_state, -1));
				lua_pop(lua_state, 1);
				luaL_unref(lua_state, LUA_REGISTRYINDEX, lua_pre_tick_ref);
				lua_pre_tick_ref = LUA_REFNIL;
				logger::Info("  pre-tick hook unregistered\n");
			}
		}

		chipset.Tick();

		if (lua_post_tick_ref != LUA_REFNIL)
		{
			lua_geti(lua_state, LUA_REGISTRYINDEX, lua_post_tick_ref);
			if (lua_pcall(lua_state, 0, 0, 0) != LUA_OK)
			{
				logger::Info("post-tick hook failed: %s\n", lua_tostring(lua_state, -1));
				lua_pop(lua_state, 1);
				luaL_unref(lua_state, LUA_REGISTRYINDEX, lua_post_tick_ref);
				lua_post_tick_ref = LUA_REFNIL;
				logger::Info("  post-tick hook unregistered\n");
			}
		}
	}

	bool Emulator::Running()
	{
		return running;
	}

	bool Emulator::GetPaused()
	{
		return paused;
	}

	void Emulator::Shutdown()
	{
		std::lock_guard<decltype(access_mx)> access_lock(access_mx);

		running = false;
	}

	void Emulator::ExecuteCommand(std::string command)
	{
		std::lock_guard<decltype(access_mx)> access_lock(access_mx);

		lua_State *thread = lua_newthread(lua_state);

		// load command to thread's stack
		const char *ugly_string_data_ptr = command.c_str();
		if (lua_load(thread, [](lua_State *, void *data, size_t *size) {
			char **ugly_string_data_ptr_ptr = (char **)data;
			if (!*ugly_string_data_ptr_ptr)
				return (const char *)nullptr;
			const char *result = *ugly_string_data_ptr_ptr;
			*size = strlen(result);
			*ugly_string_data_ptr_ptr = nullptr;
			return result;
		}, &ugly_string_data_ptr, "stdin", "t") != LUA_OK)
		{
			logger::Info("%s\n", lua_tostring(thread, -1));
		}
		else
		{
			int status = lua_resume(thread, nullptr, 0);
			// * TODO Is it necessary to clear the stack?
			if (status != LUA_OK && status != LUA_YIELD)
				logger::Info("%s\n", lua_tostring(thread, -1));
		}

		lua_pop(lua_state, 1); // thread
	}

	void Emulator::SetPaused(bool _paused)
	{
		paused = _paused;
	}

	void Emulator::Cycles::Setup(Uint64 _cycles_per_second, unsigned int _timer_interval)
	{
		ticks_now = 0;
		cycles_emulated = 0;
		cycles_per_second = _cycles_per_second;
		timer_interval = _timer_interval;
	}

	void Emulator::Cycles::Reset()
	{
		ticks_now = 0;
		cycles_emulated = 0;
	}

	Uint64 Emulator::Cycles::GetDelta()
	{
		ticks_now += timer_interval;
		Uint64 cycles_to_have_been_emulated_by_now = ticks_now * cycles_per_second / 1000;
		Uint64 diff = cycles_to_have_been_emulated_by_now - cycles_emulated;
		cycles_emulated = cycles_to_have_been_emulated_by_now;
		return diff;
	}

	SDL_Renderer *Emulator::GetRenderer()
	{
		return renderer;
	}

	SDL_Texture *Emulator::GetInterfaceTexture()
	{
		return interface_texture;
	}

	unsigned int Emulator::GetCyclesPerSecond()
	{
		return cycles.cycles_per_second;
	}

	FairRecursiveMutex::FairRecursiveMutex() : holding{}, recursive_count{}
	{
	}

	FairRecursiveMutex::~FairRecursiveMutex()
	{
		assert(0 == recursive_count);
	}

	void FairRecursiveMutex::lock()
	{
		std::unique_lock<std::mutex> lock(m);
		assert((holding == std::thread::id{}) == (recursive_count == 0));
		if (holding == std::this_thread::get_id())
		{
			++recursive_count;
			return;
		}
		if (holding != std::thread::id{} or not waiting.empty())
		{
			waiting.emplace();
			auto& c = waiting.back();
			c.wait(lock, [&]{
				assert(not waiting.empty());
				assert((holding == std::thread::id{}) == (recursive_count == 0));
				return recursive_count == 0 && &waiting.front() == &c;
			});
			waiting.pop();
		}
		assert(holding == std::thread::id{});
		assert(recursive_count == 0);
		holding = std::this_thread::get_id();
		recursive_count = 1;
	}

	void FairRecursiveMutex::unlock()
	{
		std::lock_guard<std::mutex> lock(m);
		assert(holding == std::this_thread::get_id());
		assert(recursive_count > 0);
		--recursive_count;
		if (recursive_count == 0)
		{
			holding = {};
			if (not waiting.empty())
				waiting.front().notify_one(); // the notify_one must be called while m is locked, otherwise the condition variable might be destroyed (as noted on https://en.cppreference.com/w/cpp/thread/condition_variable/notify_one)
		}
	}
}
