// psx/device.cc

#include <audio/device.hp>
#include <audio/buffer.hp>
#include <hal/hal.h>
#include <pigsys/pigsys.hp>

SoundDevice::SoundDevice()
{
}


SoundDevice::~SoundDevice()
{
}


SoundBuffer*
SoundDevice::CreateSoundBuffer( binistream& binis )
{
	return new (HALLmalloc) SoundBuffer( binis );
}
