#include "Screen.hpp"

#include "../Chipset/MMURegion.hpp"
#include "../Data/HardwareId.hpp"
#include "../Data/SpriteInfo.hpp"
#include "../Data/ColourInfo.hpp"
#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

#include <vector>

namespace casioemu
{
	struct SpriteBitmap
	{
		const char *name;
		uint8_t mask, offset;
	};

	template <HardwareId hardware_id>
	class Screen : public Peripheral
	{
		static int const N_ROW, // excluding the 1 row used for status line
			ROW_SIZE, // bytes
			OFFSET, // bytes
			ROW_SIZE_DISP; // bytes used to display

		MMURegion region_buffer, region_buffer1, region_contrast, region_mode, region_range;
		uint8_t *screen_buffer, *screen_buffer1, screen_contrast, screen_mode, screen_range;

	    SDL_Renderer *renderer;
	    SDL_Texture *interface_texture;

		enum Sprite : unsigned {
		};

		static const SpriteBitmap sprite_bitmap[];
		std::vector<SpriteInfo> sprite_info;
		ColourInfo ink_colour;

		/**
		 * Similar to MMURegion::DefaultRead, but takes the pointer to the Screen
		 * object as the userdata instead of the uint8_t member.
		 */
		template<typename value_type, value_type mask = (value_type)-1,
			value_type Screen:: *member_ptr>
		static uint8_t DefaultRead(MMURegion *region, size_t offset)
		{
			auto this_obj = (Screen *)(region->userdata);
			value_type value = this_obj->*member_ptr;
			return (value & mask) >> ((offset - region->base) * 8);
		}

		/**
		 * Similar to MMURegion::DefaultWrite, except this also set the
		 * (require_frame) flag of (Peripheral) class.
		 * If (only_on_change) is true, (require_frame) is not set if the new value
		 * is the same as the old value.
		 * (region->userdata) should be a pointer to a (Screen) instance.
		 *
		 * TODO: Probably this should be a member of Peripheral class instead.
		 * (in that case (Screen) class needs to be parameterized)
		 */
		template<typename value_type, value_type mask = (value_type)-1,
			value_type Screen:: *member_ptr, bool only_on_change = true>
		static void SetRequireFrameWrite(MMURegion *region, size_t offset, uint8_t data)
		{
			auto this_obj = (Screen *)(region->userdata);
			value_type &value = this_obj->*member_ptr;

			value_type old_value;
			if (only_on_change)
				old_value = value;

			// This part is identical to MMURegion::DefaultWrite.
			// * TODO Try to avoid duplication?
			value &= ~(((value_type)0xFF) << ((offset - region->base) * 8));
			value |= ((value_type)data) << ((offset - region->base) * 8);
			value &= mask;

			if (only_on_change && old_value == value)
				return;
			this_obj->require_frame = true;
		}

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Uninitialise();
		void Frame();
	};

	template <> const int Screen<HW_CLASSWIZ_II>::N_ROW = 63;
	template <> const int Screen<HW_CLASSWIZ_II>::ROW_SIZE = 32;
	template <> const int Screen<HW_CLASSWIZ_II>::OFFSET = 32;
	template <> const int Screen<HW_CLASSWIZ_II>::ROW_SIZE_DISP = 24;

	template <> const int Screen<HW_CLASSWIZ>::N_ROW = 63;
	template <> const int Screen<HW_CLASSWIZ>::ROW_SIZE = 32;
	template <> const int Screen<HW_CLASSWIZ>::OFFSET = 32;
	template <> const int Screen<HW_CLASSWIZ>::ROW_SIZE_DISP = 24;

	template <> const int Screen<HW_ES_PLUS>::N_ROW = 31;
	template <> const int Screen<HW_ES_PLUS>::ROW_SIZE = 16;
	template <> const int Screen<HW_ES_PLUS>::OFFSET = 16;
	template <> const int Screen<HW_ES_PLUS>::ROW_SIZE_DISP = 12;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic" // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61491
	// Note: SPR_PIXEL must be the first enum member and SPR_MAX must be the last one.
	template <> enum Screen<HW_CLASSWIZ_II>::Sprite :unsigned
	{
		SPR_PIXEL,
		SPR_S,
		SPR_MATH,
		SPR_D,
		SPR_R,
		SPR_G,
		SPR_FIX,
		SPR_SCI,
		SPR_E,
		SPR_CMPLX,
		SPR_ANGLE,
		SPR_WDOWN,
		SPR_VERIFY,
		SPR_LEFT,
		SPR_DOWN,
		SPR_UP,
		SPR_RIGHT,
		SPR_PAUSE,
		SPR_SUN,
		SPR_MAX
	};

	template <> enum Screen<HW_CLASSWIZ>::Sprite : unsigned
	{
		SPR_PIXEL,
		SPR_S,
		SPR_A,
		SPR_M,
		SPR_STO,
		SPR_MATH,
		SPR_D,
		SPR_R,
		SPR_G,
		SPR_FIX,
		SPR_SCI,
		SPR_E,
		SPR_CMPLX,
		SPR_ANGLE,
		SPR_WDOWN,
		SPR_LEFT,
		SPR_DOWN,
		SPR_UP,
		SPR_RIGHT,
		SPR_PAUSE,
		SPR_SUN,
		SPR_MAX
	};

	template <> enum Screen<HW_ES_PLUS>::Sprite : unsigned
	{
		SPR_PIXEL,
		SPR_S,
		SPR_A,
		SPR_M,
		SPR_STO,
		SPR_RCL,
		SPR_STAT,
		SPR_CMPLX,
		SPR_MAT,
		SPR_VCT,
		SPR_D,
		SPR_R,
		SPR_G,
		SPR_FIX,
		SPR_SCI,
		SPR_MATH,
		SPR_DOWN,
		SPR_UP,
		SPR_DISP,
		SPR_MAX
	};

#pragma GCC diagnostic pop


	template<> const SpriteBitmap Screen<HW_CLASSWIZ_II>::sprite_bitmap[SPR_MAX] = {
		{"rsd_pixel",    0,    0},
		{"rsd_s",     0x01, 0x01},
		{"rsd_math",  0x01, 0x03},
		{"rsd_d",     0x01, 0x04},
		{"rsd_r",     0x01, 0x05},
		{"rsd_g",     0x01, 0x06},
		{"rsd_fix",   0x01, 0x07},
		{"rsd_sci",   0x01, 0x08},
		{"rsd_e",     0x01, 0x0A},
		{"rsd_cmplx", 0x01, 0x0B},
		{"rsd_angle", 0x01, 0x0C},
		{"rsd_wdown", 0x01, 0x0D},
		{"rsd_verify",0x01, 0x0E},
		{"rsd_left",  0x01, 0x10},
		{"rsd_down",  0x01, 0x11},
		{"rsd_up",    0x01, 0x12},
		{"rsd_right", 0x01, 0x13},
		{"rsd_pause", 0x01, 0x15},
		{"rsd_sun",   0x01, 0x16}
	};

	template<> const SpriteBitmap Screen<HW_CLASSWIZ>::sprite_bitmap[SPR_MAX] = {
		{"rsd_pixel",    0,    0},
		{"rsd_s",     0x01, 0x00},
		{"rsd_a",     0x01, 0x01},
		{"rsd_m",     0x01, 0x02},
		{"rsd_sto",   0x01, 0x03},
		{"rsd_math",  0x01, 0x05},
		{"rsd_d",     0x01, 0x06},
		{"rsd_r",     0x01, 0x07},
		{"rsd_g",     0x01, 0x08},
		{"rsd_fix",   0x01, 0x09},
		{"rsd_sci",   0x01, 0x0A},
		{"rsd_e",     0x01, 0x0B},
		{"rsd_cmplx", 0x01, 0x0C},
		{"rsd_angle", 0x01, 0x0D},
		{"rsd_wdown", 0x01, 0x0F},
		{"rsd_left",  0x01, 0x10},
		{"rsd_down",  0x01, 0x11},
		{"rsd_up",    0x01, 0x12},
		{"rsd_right", 0x01, 0x13},
		{"rsd_pause", 0x01, 0x15},
		{"rsd_sun",   0x01, 0x16}
	};

	template<> const SpriteBitmap Screen<HW_ES_PLUS>::sprite_bitmap[SPR_MAX] = {
		{"rsd_pixel",    0,    0},
		{"rsd_s",     0x10, 0x00},
		{"rsd_a",     0x04, 0x00},
		{"rsd_m",     0x10, 0x01},
		{"rsd_sto",   0x02, 0x01},
		{"rsd_rcl",   0x40, 0x02},
		{"rsd_stat",  0x40, 0x03},
		{"rsd_cmplx", 0x80, 0x04},
		{"rsd_mat",   0x40, 0x05},
		{"rsd_vct",   0x01, 0x05},
		{"rsd_d",     0x20, 0x07},
		{"rsd_r",     0x02, 0x07},
		{"rsd_g",     0x10, 0x08},
		{"rsd_fix",   0x01, 0x08},
		{"rsd_sci",   0x20, 0x09},
		{"rsd_math",  0x40, 0x0A},
		{"rsd_down",  0x08, 0x0A},
		{"rsd_up",    0x80, 0x0B},
		{"rsd_disp",  0x10, 0x0B}
	};

	template <HardwareId hardware_id> void Screen<hardware_id>::Initialise()
	{
		auto constexpr SPR_MAX = Sprite::SPR_MAX;

		static_assert(SPR_MAX == (sizeof(sprite_bitmap) / sizeof(sprite_bitmap[0])), "SPR_MAX and sizeof(sprite_bitmap) don't match");

	    renderer = emulator.GetRenderer();
	    interface_texture = emulator.GetInterfaceTexture();
		sprite_info.resize(SPR_MAX);
		for (int ix = 0; ix != SPR_MAX; ++ix)
			sprite_info[ix] = emulator.GetModelInfo(sprite_bitmap[ix].name);
		
		ink_colour = emulator.GetModelInfo("ink_colour");
		require_frame = true;

		screen_buffer = new uint8_t[(N_ROW + 1) * ROW_SIZE];

		region_buffer.Setup(0xF800, (N_ROW + 1) * ROW_SIZE, "Screen/Buffer", this, [](MMURegion *region, size_t offset) {
			offset -= region->base;
			if (offset % ROW_SIZE >= ROW_SIZE_DISP)
				return (uint8_t)0;
			return ((Screen *)region->userdata)->screen_buffer[offset];
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			offset -= region->base;
			if (offset % ROW_SIZE >= ROW_SIZE_DISP)
				return;

			auto this_obj = (Screen *)region->userdata;
			// * Set require_frame to true only if the value changed.
			this_obj->require_frame |= this_obj->screen_buffer[offset] != data;
			this_obj->screen_buffer[offset] = data;
		}, emulator);

		if (emulator.hardware_id == HW_CLASSWIZ_II) {
			screen_buffer1 = new uint8_t[(N_ROW + 1) * ROW_SIZE];
			region_buffer1.Setup(0x89000, (N_ROW + 1) * ROW_SIZE, "Screen/Buffer1", this, [](MMURegion* region, size_t offset) {
				offset -= region->base;
				if (offset % ROW_SIZE >= ROW_SIZE_DISP)
					return (uint8_t)0;
				return ((Screen*)region->userdata)->screen_buffer1[offset];
				}, [](MMURegion* region, size_t offset, uint8_t data) {
					offset -= region->base;
					if (offset % ROW_SIZE >= ROW_SIZE_DISP)
						return;

					auto this_obj = (Screen*)region->userdata;
					// * Set require_frame to true only if the value changed.
					this_obj->require_frame |= this_obj->screen_buffer1[offset] != data;
					this_obj->screen_buffer1[offset] = data;
				}, emulator);
		}

		region_range.Setup(0xF030, 1, "Screen/Range", this, DefaultRead<uint8_t, 0x07, &Screen::screen_range>,
				SetRequireFrameWrite<uint8_t, 0x07, &Screen::screen_range>, emulator);

		region_mode.Setup(0xF031, 1, "Screen/Mode", this, DefaultRead<uint8_t, 0x07, &Screen::screen_mode>,
				SetRequireFrameWrite<uint8_t, 0x07, &Screen::screen_mode>, emulator);

		region_contrast.Setup(0xF032, 1, "Screen/Contrast", this, DefaultRead<uint8_t, 0x3F, &Screen::screen_contrast>,
				SetRequireFrameWrite<uint8_t, 0x3F, &Screen::screen_contrast>, emulator);
	}

	template<HardwareId hardware_id> void Screen<hardware_id>::Uninitialise()
	{
		delete[] screen_buffer;
		delete[] screen_buffer1;
	}

	template<HardwareId hardware_id> void Screen<hardware_id>::Frame()
	{
		require_frame = false;

		int ink_alpha_on = 20 + screen_contrast * 16;
		if (ink_alpha_on > 255)
			ink_alpha_on = 255;
		int ink_alpha_off = (screen_contrast - 8) * 7;
		if (ink_alpha_off < 0)
			ink_alpha_off = 0;

		bool enable_status, enable_dotmatrix, clear_dots;

		switch (screen_mode)
		{
		case 4:
			enable_dotmatrix = true;
			clear_dots = true;
			enable_status = false;
			break;

		case 5:
			enable_dotmatrix = true;
			clear_dots = false;
			enable_status = true;
			break;

		case 6:
			enable_dotmatrix = true;
			clear_dots = true;
			enable_status = true;
			ink_alpha_on = 80;
			ink_alpha_off = 20;
			break;

		default:
			return;
		}

		SDL_SetTextureColorMod(interface_texture, ink_colour.r, ink_colour.g, ink_colour.b);

		if (enable_status)
		{
			for (int ix = Sprite::SPR_PIXEL + 1; ix != Sprite::SPR_MAX; ++ix)
			{
				if (screen_buffer[sprite_bitmap[ix].offset] & sprite_bitmap[ix].mask)
					SDL_SetTextureAlphaMod(interface_texture, ink_alpha_on);
				else
					SDL_SetTextureAlphaMod(interface_texture, ink_alpha_off);
				SDL_RenderCopy(renderer, interface_texture, &sprite_info[ix].src, &sprite_info[ix].dest);
			}
		}

		if (enable_dotmatrix)
		{
			static constexpr auto SPR_PIXEL = Sprite::SPR_PIXEL;
			SDL_Rect dest = Screen<hardware_id>::sprite_info[SPR_PIXEL].dest;
			int ink_alpha = ink_alpha_off;
			if (emulator.hardware_id == HW_CLASSWIZ_II) {
				for (int iy = 0; iy != N_ROW; ++iy)
				{
					dest.x = sprite_info[SPR_PIXEL].dest.x;
					dest.y = sprite_info[SPR_PIXEL].dest.y + iy * sprite_info[SPR_PIXEL].src.h;
					for (int ix = 0; ix != ROW_SIZE_DISP; ++ix)
					{
						for (uint8_t mask = 0x80; mask; mask >>= 1, dest.x += sprite_info[SPR_PIXEL].src.w)
						{
							ink_alpha = ink_alpha_off;
							if (!clear_dots && screen_buffer[iy * ROW_SIZE + OFFSET + ix] & mask)
								ink_alpha += (ink_alpha_on - ink_alpha_off) * 0.3;
							if (!clear_dots && screen_buffer1[iy * ROW_SIZE + OFFSET + ix] & mask)
								ink_alpha += (ink_alpha_on - ink_alpha_off) * 0.7;
							SDL_SetTextureAlphaMod(interface_texture, ink_alpha);
							SDL_RenderCopy(renderer, interface_texture, &sprite_info[SPR_PIXEL].src, &dest);
						}
					}
				}
			}
			else {
				for (int iy = 0; iy != N_ROW; ++iy)
				{
					dest.x = sprite_info[SPR_PIXEL].dest.x;
					dest.y = sprite_info[SPR_PIXEL].dest.y + iy * sprite_info[SPR_PIXEL].src.h;
					for (int ix = 0; ix != ROW_SIZE_DISP; ++ix)
					{
						for (uint8_t mask = 0x80; mask; mask >>= 1, dest.x += sprite_info[SPR_PIXEL].src.w)
						{
							if (!clear_dots && screen_buffer[iy * ROW_SIZE + OFFSET + ix] & mask)
								SDL_SetTextureAlphaMod(interface_texture, ink_alpha_on);
							else
								SDL_SetTextureAlphaMod(interface_texture, ink_alpha_off);
							SDL_RenderCopy(renderer, interface_texture, &sprite_info[SPR_PIXEL].src, &dest);
						}
					}
				}
			}
		}
	}

	Peripheral *CreateScreen(Emulator& emulator)
	{
		switch (emulator.hardware_id)
		{
		case HW_ES_PLUS:
			return new Screen<HW_ES_PLUS>(emulator);

		case HW_CLASSWIZ:
			return new Screen<HW_CLASSWIZ>(emulator);

		case HW_CLASSWIZ_II:
			return new Screen<HW_CLASSWIZ_II>(emulator);
		default:
			PANIC("Unknown hardware id\n");
		}
	}
}
