#include "MMU.hpp"

#include <cstdio>
#include <cstring>
#include "../Emulator.hpp"
#include "Chipset.hpp"
#include "../Logger.hpp"

namespace casioemu
{
	MMU::MMU(Emulator &_emulator) : emulator(_emulator)
	{
		segment_dispatch = new MemoryByte *[0x100];
		for (size_t ix = 0; ix != 0x100; ++ix)
			segment_dispatch[ix] = nullptr;
	}

	MMU::~MMU()
	{
		for (size_t ix = 0; ix != 0x100; ++ix)
			if (segment_dispatch[ix])
				delete[] segment_dispatch[ix];

		delete[] segment_dispatch;
	}

	void MMU::GenerateSegmentDispatch(size_t segment_index)
	{
		segment_dispatch[segment_index] = new MemoryByte[0x10000];
		for (size_t ix = 0; ix != 0x10000; ++ix)
		{
			segment_dispatch[segment_index][ix].region = nullptr;
			segment_dispatch[segment_index][ix].on_read = LUA_REFNIL;
			segment_dispatch[segment_index][ix].on_write = LUA_REFNIL;
		}
	}

	void MMU::SetupInternals()
	{
		*(MMU **)lua_newuserdata(emulator.lua_state, sizeof(MMU *)) = this;
		lua_newtable(emulator.lua_state);
		lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
			lua_pushinteger(lua_state, mmu->ReadCode(lua_tointeger(lua_state, 2)));
			return 1;
		});
		lua_setfield(emulator.lua_state, -2, "__index");
		lua_pushcfunction(emulator.lua_state, [](lua_State *) {
			return 0;
		});
		lua_setfield(emulator.lua_state, -2, "__newindex");
		lua_setmetatable(emulator.lua_state, -2);
		lua_setglobal(emulator.lua_state, "code");

		*(MMU **)lua_newuserdata(emulator.lua_state, sizeof(MMU *)) = this;
		lua_newtable(emulator.lua_state);
		lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
			int isnum;
			size_t offset = lua_tointegerx(lua_state, 2, &isnum);
			if (isnum)
			{
				lua_pushinteger(lua_state, mmu->ReadData(offset));
				return 1;
			}
			const char *key = lua_tostring(lua_state, 2);
			if (key == nullptr)  // the key is not a string
				return 0;
			if (std::strcmp(key, "rwatch") == 0)
			{
				// execute Lua function whenever address is read from
				lua_pushcfunction(lua_state, [](lua_State *lua_state) {
					if (lua_gettop(lua_state) != 3)
						return luaL_error(lua_state, "rwatch function called with incorrect number of arguments");

					MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
					size_t offset = lua_tointeger(lua_state, 2);
					int on_read = luaL_ref(lua_state, LUA_REGISTRYINDEX);

					size_t segment_index = offset >> 16;
					size_t segment_offset = offset & 0xFFFF;

					MemoryByte *segment = mmu->segment_dispatch[segment_index];
					if (!segment)
					{
						logger::Info("attempt to set rwatch from offset %04zX of unmapped segment %02zX\n",
								segment_offset, segment_index);
						return 0;
					}

					MemoryByte &byte = segment[segment_offset];
					luaL_unref(lua_state, LUA_REGISTRYINDEX, byte.on_read);
					byte.on_read = on_read;
					return 0;
				});
				return 1;
			}
			else if (std::strcmp(key, "watch") == 0)
			{
				// execute Lua function whenever address is written to
				lua_pushcfunction(lua_state, [](lua_State *lua_state) {
					if (lua_gettop(lua_state) != 3)
						return luaL_error(lua_state, "watch function called with incorrect number of arguments");

					MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
					size_t offset = lua_tointeger(lua_state, 2);
					int on_write = luaL_ref(lua_state, LUA_REGISTRYINDEX);

					size_t segment_index = offset >> 16;
					size_t segment_offset = offset & 0xFFFF;

					MemoryByte *segment = mmu->segment_dispatch[segment_index];
					if (!segment)
					{
						logger::Info("attempt to set watch from offset %04zX of unmapped segment %02zX\n",
								segment_offset, segment_index);
						return 0;
					}

					MemoryByte &byte = segment[segment_offset];
					luaL_unref(lua_state, LUA_REGISTRYINDEX, byte.on_write);
					byte.on_write = on_write;
					return 0;
				});
				return 1;
			}
			else
			{
				return 0;
			}
		});
		lua_setfield(emulator.lua_state, -2, "__index");
		lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
			mmu->WriteData(lua_tointeger(lua_state, 2), lua_tointeger(lua_state, 3));
			return 0;
		});
		lua_setfield(emulator.lua_state, -2, "__newindex");
		lua_setmetatable(emulator.lua_state, -2);
		lua_setglobal(emulator.lua_state, "data");
	}

	uint16_t MMU::ReadCode(size_t offset)
	{
		if (offset >= (1 << 20))
			PANIC("offset doesn't fit 20 bits\n");
		if (offset & 1)
			PANIC("offset has LSB set\n");

		size_t segment_index = offset >> 16;
		size_t segment_offset = offset & 0xFFFF;

		if (!segment_index)
			return (((uint16_t)emulator.chipset.rom_data[segment_offset + 1]) << 8) | emulator.chipset.rom_data[segment_offset];

		MemoryByte *segment = segment_dispatch[segment_index];
		if (!segment)
		{
			emulator.HandleMemoryError();
			return 0;
		}

		MMURegion *region = segment[segment_offset].region;
		if (!region)
		{
			emulator.HandleMemoryError();
			return 0;
		}

		return (((uint16_t)region->read(region, offset + 1)) << 8) | region->read(region, offset);
	}

	uint8_t MMU::ReadData(size_t offset)
	{
		if (offset >= (1 << 24))
			PANIC("offset doesn't fit 24 bits\n");

		size_t segment_index = offset >> 16;
		size_t segment_offset = offset & 0xFFFF;

		MemoryByte *segment = segment_dispatch[segment_index];
		if (!segment)
		{
			//logger::Info("read from offset %04zX of unmapped segment %02zX\n", segment_offset, segment_index);
			emulator.HandleMemoryError();
			return 0;
		}

		MemoryByte &byte = segment[segment_offset];
		MMURegion *region = byte.region;
		if (byte.on_read != LUA_REFNIL)
		{
			lua_geti(emulator.lua_state, LUA_REGISTRYINDEX, byte.on_read);
			if (lua_pcall(emulator.lua_state, 0, 0, 0) != LUA_OK)
			{
				//logger::Info("calling commands on rwatch at %06zX failed: %s\n",
						//offset, lua_tostring(emulator.lua_state, -1));
				lua_pop(emulator.lua_state, 1);
			}
		}
		if (!region)
		{
			//logger::Info("read from unmapped offset %04zX of segment %02zX\n", segment_offset, segment_index);
			emulator.HandleMemoryError();
			return 0;
		}

		return region->read(region, offset);
	}

	void MMU::WriteData(size_t offset, uint8_t data)
	{
		if (offset >= (1 << 24))
			PANIC("offset doesn't fit 24 bits\n");

		size_t segment_index = offset >> 16;
		size_t segment_offset = offset & 0xFFFF;

		MemoryByte *segment = segment_dispatch[segment_index];
		if (!segment)
		{
			//logger::Info("write to offset %04zX of unmapped segment %02zX (%02zX)\n", segment_offset, segment_index, data);
			emulator.HandleMemoryError();
			return;
		}

		MemoryByte &byte = segment[segment_offset];
		MMURegion *region = byte.region;
		if (byte.on_write != LUA_REFNIL)
		{
			lua_geti(emulator.lua_state, LUA_REGISTRYINDEX, byte.on_write);
			if (lua_pcall(emulator.lua_state, 0, 0, 0) != LUA_OK)
			{
				logger::Info("calling commands on watch at %06zX failed: %s\n",
						offset, lua_tostring(emulator.lua_state, -1));
				lua_pop(emulator.lua_state, 1);
			}
		}
		if (!region)
		{
			//logger::Info("write to unmapped offset %04zX of segment %02zX (%02zX)\n", segment_offset, segment_index, data);
			emulator.HandleMemoryError();
			return;
		}

		region->write(region, offset, data);
	}

	void MMU::RegisterRegion(MMURegion *region)
	{
		for (size_t ix = region->base; ix != region->base + region->size; ++ix)
		{
			if(ix == 561152){
				printf("s");
			}
			if (segment_dispatch[ix >> 16][ix & 0xFFFF].region)
				PANIC("MMU region overlap at %06zX\n", ix);
			segment_dispatch[ix >> 16][ix & 0xFFFF].region = region;
		}
	}

	void MMU::UnregisterRegion(MMURegion *region)
	{
		for (size_t ix = region->base; ix != region->base + region->size; ++ix)
		{
			if (!segment_dispatch[ix >> 16][ix & 0xFFFF].region)
				PANIC("MMU region double-hole at %06zX\n", ix);
			segment_dispatch[ix >> 16][ix & 0xFFFF].region = nullptr;
		}
	}
}
