#include "../UpdateList.h"

#include <cstring>

#define DR_MP3_IMPLEMENTATION
#include "../../include/dr_libs/dr_mp3.h"//

#define MUSICERROR "Failed to load music file"

drmp3 mp3;
bool backgroundMusic = false;
float *bufferIn = NULL;
float volume = 1.0f;

//Audio systems
void UpdateList::setVolume(int _volume) {
    volume = _volume / 100.0f;
}

//Background music
void UpdateList::musicStream(std::string filename, int _volume) {
	if(!drmp3_init_file(&mp3, filename.c_str(), NULL))
        throw new std::invalid_argument(MUSICERROR);

    backgroundMusic = true;
    volume = _volume / 100.0f;
}

void stream_cb(float* bufferOut, int num_frames, int num_channels) {
    const unsigned int num_samples = num_frames * num_channels;
    if(bufferIn == NULL && backgroundMusic)
        bufferIn = (float*)malloc(num_samples);

    //Load music from file
    if(backgroundMusic)
        drmp3_read_pcm_frames_f32(&mp3, num_frames, bufferIn);

    //Copy music to output
    if(bufferIn == NULL)
        std::memset(bufferOut, 0, num_samples);
    else {
        for(unsigned int i = 0; i < num_samples; i++)
            bufferOut[i] = bufferIn[i] * volume;
    }
}