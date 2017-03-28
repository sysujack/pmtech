#include "audio.h"
#include "fmod.hpp"
#include <math.h>

#if 0
namespace pen
{
#define MAX_SOUNDS 32
#define INVALID_SOUND (u32)-1
#define MAX_CHANNEL_GROUPS 32

	FMOD::System*			g_sound_system;

	FMOD::Sound*			g_sounds[ MAX_SOUNDS ];
	FMOD::Channel*			g_channels[ MAX_SOUNDS ];

	FMOD::ChannelGroup*		g_channel_groups[ MAX_CHANNEL_GROUPS ];

	u32 get_free_sound( )
	{
		for( u32 i = 0; i < MAX_SOUNDS; ++i )
		{
			if( g_sounds[ i ] == NULL )
			{
				return i;
			}
		}

		return INVALID_SOUND;
	}

	u32 get_free_channel_group( )
	{
		for (u32 i = 0; i < MAX_SOUNDS; ++i)
		{
			if (g_channel_groups[i] == NULL)
			{
				return i;
			}
		}

		return INVALID_SOUND;
	}

	void audio_system_initialise( )
	{
		FMOD_RESULT result; 

		FMOD::System_Create(&g_sound_system);

		result = g_sound_system->init( 32, FMOD_INIT_NORMAL, NULL );

		//initialize the data
		for( u32 i = 0; i < MAX_SOUNDS; ++i )
		{
			g_sounds[ i ] = NULL;
			g_channels[ i ] = NULL;
		}

		for (u32 i = 0; i < MAX_CHANNEL_GROUPS; ++i)
		{
			g_channel_groups[ i ] = NULL;
		}

		PEN_ASSERT( result == FMOD_OK );
	}

	void audio_system_update( )
	{
		g_sound_system->update();
	}

	u32 audio_create_stream( const c8* filename )
	{
		u32 sound_slot = get_free_sound( );

		if( sound_slot != INVALID_SOUND )
		{
			FMOD_RESULT result;

			result = g_sound_system->createStream( filename, FMOD_SOFTWARE, NULL, &g_sounds[sound_slot] );

			PEN_ASSERT( result == FMOD_OK );

			return sound_slot;
		}

		return INVALID_SOUND;
	}

	u32 audio_create_sound( const c8* filename )
	{
		u32 sound_slot = get_free_sound( );

		if (sound_slot != INVALID_SOUND)
		{
			FMOD_RESULT result;

			result = g_sound_system->createSound( filename, FMOD_SOFTWARE, NULL, &g_sounds[sound_slot] );

			PEN_ASSERT( result == FMOD_OK );

			return sound_slot;
		}

		return INVALID_SOUND;
	}


	void audio_create_sound_channel( const u32 &snd )
	{
		FMOD_RESULT result;

		result = g_sound_system->playSound( FMOD_CHANNEL_FREE, g_sounds[ snd ], FALSE, &g_channels[ snd ] );

		PEN_ASSERT( result == FMOD_OK );
	}

	u32 audio_create_channel_group( )
	{
		u32 group = get_free_channel_group( );

		if( group != INVALID_SOUND )
		{
			FMOD_RESULT result;

			result = g_sound_system->createChannelGroup(NULL, &g_channel_groups[ group ] );

			PEN_ASSERT( result == FMOD_OK );

			return group;
		}

		return INVALID_SOUND;
	}

	void audio_add_sound_to_channel_group( const u32 &snd, const u32 &channel_group )
	{
		FMOD_RESULT result;

		result = g_channels[ snd ]->setChannelGroup( g_channel_groups[ channel_group ] );

		PEN_ASSERT( result == FMOD_OK );
	}

	void audio_channel_set_pitch( const u32 &channel_group, const f32 &pitch )
	{
		f32 abs_pitch = fabs( pitch );
	
		s32 num_channels = 0;
		g_channel_groups[channel_group]->getNumChannels( &num_channels );
		for (s32 i = 0; i < num_channels; ++i)
		{
			FMOD::Channel* chan;
			g_channel_groups[channel_group]->getChannel( i, &chan );

			f32 freq = 0.0f;
			chan->getFrequency( &freq );

			f32 abs_freq = fabs( freq );

			chan->setFrequency( pitch < 0.0f ? -abs_freq : abs_freq );
		}

		g_channel_groups[ channel_group ]->setPitch( abs_pitch );

		//EQ TEST
		/*
		FMOD::DSP* dspeqlow;
		g_sound_system->createDSPByType( FMOD_DSP_TYPE_PARAMEQ, &dspeqlow );

		dspeqlow->setParameter( 0, 30.0f );
		dspeqlow->setParameter( 1, 0.2f );
		dspeqlow->setParameter( 2, 2.0f );

		FMOD::DSPConnection* dspcon;
		g_channel_groups[ channel_group ]->addDSP( dspeqlow, &dspcon );
		*/
	}

	void audio_channel_set_volume( const u32 &channel_group, const f32 &volume )
	{
		g_channel_groups[ channel_group ]->setVolume( volume );
	}

	void audio_channel_pause( const u32 &channel_group, const u32 &val )
	{
		g_channel_groups[channel_group]->setPaused( val == 0 ? false : true );
	}

	void audio_sound_set_frequency( const u32 &snd )
	{
		f32 freq = 0.0f;
		g_channels[ snd ]->getFrequency( &freq );
		g_channels[ snd ]->setFrequency( -freq );
	}

	void audio_channel_get_spectrum( const u32 &channel_group, float *spectrum_array, u32 sample_size, u32 channel_offset )
	{
		g_channel_groups[channel_group]->getSpectrum( spectrum_array, sample_size, channel_offset, FMOD_DSP_FFT_WINDOW_BLACKMANHARRIS );
	}

	void audio_channel_set_position( const u32 &channel_group, u32 pos_ms )
	{
		s32 num_channels = 0;
		g_channel_groups[channel_group]->getNumChannels( &num_channels );
		for (s32 i = 0; i < num_channels; ++i)
		{
			FMOD::Channel* chan;
			g_channel_groups[channel_group]->getChannel( i, &chan );

			chan->setPosition( pos_ms, FMOD_TIMEUNIT_MS );
		}
	}

	u32 audio_channel_get_position( const u32 &channel_group )
	{
		s32 num_channels = 0;
		g_channel_groups[channel_group]->getNumChannels( &num_channels );

		for (s32 i = 0; i < num_channels; ++i)
		{
			FMOD::Channel* chan;
			g_channel_groups[channel_group]->getChannel( i, &chan );

			u32 pos_out;
			chan->getPosition( &pos_out, FMOD_TIMEUNIT_MS );

			return pos_out;
		}

		return 0;
	}

	u32 audio_get_sound_length( const u32 &snd )
	{
		u32 length_out = 0;
		g_sounds[ snd ]->getLength( &length_out, FMOD_TIMEUNIT_MS );

		return length_out;
	}

	u32 audio_is_channel_playing( const u32 &chan )
	{
		bool ip;
		g_channels[ chan ]->isPlaying( &ip );

		return (u32)ip;
	}
}
#endif