#include "StandbyControl.hpp"

#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

namespace casioemu
{
	void StandbyControl::Initialise()
	{
		region_stpacp.Setup(0xF008, 1, "StandbyControl/STPACP", this, MMURegion::IgnoreRead<0x00>, [](MMURegion *region, size_t, uint8_t data) {
			StandbyControl *self = (StandbyControl *)(region->userdata);
			if ((data & 0xF0) == 0xA0 && (self->stpacp_last & 0xF0) == 0x50)
				self->stop_acceptor_enabled = true;
			self->stpacp_last = data;
		}, emulator);

		region_sbycon.Setup(0xF009, 1, "StandbyControl/SBYCON", this, MMURegion::IgnoreRead<0x00>, [](MMURegion *region, size_t, uint8_t data) {
			StandbyControl *self = (StandbyControl *)(region->userdata);

			if (data & 0x01)
			{
				self->emulator.chipset.Halt();
				return;
			}

			if (data & 0x02 && self->stop_acceptor_enabled)
			{
				self->stop_acceptor_enabled = false;
				self->emulator.chipset.Stop();
				return;
			}
		}, emulator);
	}

	void StandbyControl::Reset()
	{
		stpacp_last = 0;
		stop_acceptor_enabled = false;
	}
}
