#include <stdio.h>
#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <sndfile.h>
#include "audio.h"

static ALCdevice *device = NULL;
static ALCcontext *context = NULL;

static ALuint buffer = 0;
static ALuint source = 0;

void iniciarAudio(const char *arquivo)
{
    SF_INFO info = {0};
    SNDFILE *file;
    short *samples;
    sf_count_t framesLidos;
    ALenum formato;

    // Inicializa dispositivo e contexto OpenAL
    device = alcOpenDevice(NULL);

    if (!device) {
        printf("Erro ao inicializar dispositivo de audio.\n");
        return;
    }

    context = alcCreateContext(device, NULL);

    if (!context || !alcMakeContextCurrent(context)) {
        printf("Erro ao criar contexto de audio.\n");
        pararAudio();
        return;
    }

    // Abre arquivo WAV
    file = sf_open(arquivo, SFM_READ, &info);

    if (!file) {
        printf("Erro ao abrir musica: %s\n", sf_strerror(NULL));
        pararAudio();
        return;
    }

    if (info.channels != 1 && info.channels != 2) {
        printf("O audio precisa ser mono ou stereo.\n");
        sf_close(file);
        pararAudio();
        return;
    }

    samples = malloc(info.frames * info.channels * sizeof(short));

    if (!samples) {
        sf_close(file);
        pararAudio();
        return;
    }

    framesLidos = sf_readf_short(file, samples, info.frames);
    sf_close(file);

    if (framesLidos <= 0) {
        free(samples);
        pararAudio();
        return;
    }

    formato = (info.channels == 1)
        ? AL_FORMAT_MONO16
        : AL_FORMAT_STEREO16;

    // Cria buffer de áudio
    alGenBuffers(1, &buffer);

    alBufferData(
        buffer,
        formato,
        samples,
        (ALsizei)(framesLidos * info.channels * sizeof(short)),
        info.samplerate
    );

    free(samples);

    // Cria fonte de reprodução
    alGenSources(1, &source);

    alSourcei(source, AL_BUFFER, buffer);

    // Configura reprodução infinita
    alSourcei(source, AL_LOOPING, AL_TRUE);

    // Volume em 40%
    alSourcef(source, AL_GAIN, 0.4f);

    // Inicia música
    alSourcePlay(source);

    printf("Musica de fundo iniciada!\n");
}

void pararAudio(void)
{
    if (source != 0) {
        alSourceStop(source);
        alDeleteSources(1, &source);
        source = 0;
    }

    if (buffer != 0) {
        alDeleteBuffers(1, &buffer);
        buffer = 0;
    }

    if (context) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        context = NULL;
    }

    if (device) {
        alcCloseDevice(device);
        device = NULL;
    }
}