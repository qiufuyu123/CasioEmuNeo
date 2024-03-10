#include "ModelInfo.hpp"

#include "../Emulator.hpp"
#include "SpriteInfo.hpp"
#include "ColourInfo.hpp"



namespace casioemu
{
	ModelInfo Emulator::GetModelInfo(std::string key)
	{
		return ModelInfo(*this, key);
	}

	ModelInfo::ModelInfo(Emulator &_emulator, std::string _key) : emulator(_emulator)
	{
		key = _key;
	}

	ModelInfo::operator std::string()
	{
		lua_geti(emulator.lua_state, LUA_REGISTRYINDEX, emulator.lua_model_ref);
		if (lua_getfield(emulator.lua_state, -1, key.c_str()) != LUA_TSTRING)
			PANIC("key '%s' is not a string\n", key.c_str());
		const char *value = lua_tostring(emulator.lua_state, -1);
		lua_pop(emulator.lua_state, 2);
		return std::string(value);
	}

	ModelInfo::operator int()
	{
		lua_geti(emulator.lua_state, LUA_REGISTRYINDEX, emulator.lua_model_ref);
		if (lua_getfield(emulator.lua_state, -1, key.c_str()) != LUA_TNUMBER)
			PANIC("key '%s' is not a number\n", key.c_str());
		int value = lua_tointeger(emulator.lua_state, -1);
		lua_pop(emulator.lua_state, 2);
		return value;
	}

	ModelInfo::operator SpriteInfo()
	{
		lua_geti(emulator.lua_state, LUA_REGISTRYINDEX, emulator.lua_model_ref);
		if (lua_getfield(emulator.lua_state, -1, key.c_str()) != LUA_TTABLE)
			PANIC("key '%s' is not a table\n", key.c_str());

		for (int ix = 0; ix != 6; ++ix)
			if (lua_geti(emulator.lua_state, -1 - ix, ix + 1) != LUA_TNUMBER)
				PANIC("key '%s'[%i] is not a number\n", key.c_str(), ix + 1);

		SpriteInfo sprite_info;
		sprite_info.src.x = lua_tointeger(emulator.lua_state, -6);
		sprite_info.src.y = lua_tointeger(emulator.lua_state, -5);
		sprite_info.src.w = lua_tointeger(emulator.lua_state, -4);
		sprite_info.src.h = lua_tointeger(emulator.lua_state, -3);
		sprite_info.dest.x = lua_tointeger(emulator.lua_state, -2);
		sprite_info.dest.y = lua_tointeger(emulator.lua_state, -1);
		sprite_info.dest.w = sprite_info.src.w;
		sprite_info.dest.h = sprite_info.src.h;

		lua_pop(emulator.lua_state, 7);
		return sprite_info;
	}

	ModelInfo::operator ColourInfo()
	{
		lua_geti(emulator.lua_state, LUA_REGISTRYINDEX, emulator.lua_model_ref);
		if (lua_getfield(emulator.lua_state, -1, key.c_str()) != LUA_TTABLE)
			PANIC("key '%s' is not a table\n", key.c_str());

		for (int ix = 0; ix != 3; ++ix)
			if (lua_geti(emulator.lua_state, -1 - ix, ix + 1) != LUA_TNUMBER)
				PANIC("key '%s'[%i] is not a number\n", key.c_str(), ix + 1);

		ColourInfo colour_info;
		colour_info.r = lua_tointeger(emulator.lua_state, -3);
		colour_info.g = lua_tointeger(emulator.lua_state, -2);
		colour_info.b = lua_tointeger(emulator.lua_state, -1);

		lua_pop(emulator.lua_state, 4);
		return colour_info;
	}
}

