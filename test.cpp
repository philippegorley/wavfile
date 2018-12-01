#include "wav.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

#ifdef WF_AVFRAME
extern "C" {
#include <libavutil/channel_layout.h>
}
#endif

template<typename T>
T get_sample(float t)
{
    if (std::is_unsigned<T>::value)
        // For unsigned, sin(t) must be above or equal to 0
        // Multiply by half of max to avoid overflowing
        return std::numeric_limits<T>::max() / 2 * (sin(t) + 1.0); // [0, max]
    else if (std::is_integral<T>::value)
        return std::numeric_limits<T>::max() * sin(t); // [min, max]
    else
        return sin(t); // [-1.0, 1.0]
}

template<typename T>
std::vector<T> get_vector(int channels, int rate)
{
    const float tincr = 2 * M_PI * 440.0 / rate;
    float t = 0;
    std::vector<T> v;
    for (int i = 0; i < rate; ++i) {
        for (int j = 0; j < channels; ++j)
            v.push_back(get_sample<T>(t));
        t += tincr;
    }
    return v;
}

template<typename T>
void test_sample(std::string filename, int channels, int rate)
{
    auto wav = std::unique_ptr<wav::WavFile<T>>(new wav::WavFile<T>(filename, channels, rate));
    auto v = get_vector<T>(channels, rate);
    for (T sample : v)
        wav->write(sample);
}

template<typename T>
void test_vector(std::string filename, int channels, int rate)
{
    auto wav = std::unique_ptr<wav::WavFile<T>>(new wav::WavFile<T>(filename, channels, rate));
    auto v = get_vector<T>(channels, rate);
    wav->write(v);
}

template<typename T>
void test_vector_vector(std::string filename, int channels, int rate)
{
    auto wav = std::unique_ptr<wav::WavFile<T>>(new wav::WavFile<T>(filename, channels, rate));
    auto v = std::vector<std::vector<T>>(std::max(1, channels));
    for (auto& c : v)
        c = get_vector<T>(1, rate);
    wav->write(v);
}

#ifdef WF_AVFRAME
template<typename T>
void test_frame(std::string filename, int channels, int rate, bool planar)
{
    auto wav = std::unique_ptr<wav::WavFile<T>>(new wav::WavFile<T>(filename, channels, rate));
    AVFrame *frame = av_frame_alloc();
    AVSampleFormat format;
    if (std::is_same<T, uint8_t>::value)
        format = AV_SAMPLE_FMT_U8;
    else if (std::is_same<T, int16_t>::value)
        format = AV_SAMPLE_FMT_S16;
    else if (std::is_same<T, int32_t>::value)
        format = AV_SAMPLE_FMT_S32;
    else if (std::is_same<T, int64_t>::value)
        format = AV_SAMPLE_FMT_S64;
    else if (std::is_same<T, float>::value)
        format = AV_SAMPLE_FMT_FLT;
    else if (std::is_same<T, double>::value)
        format = AV_SAMPLE_FMT_DBL;
    if (planar)
        format = av_get_planar_sample_fmt(format);
    for (int i = 0; i < 50; ++i) {
        frame->format = format;
        frame->channels = channels;
        frame->channel_layout = av_get_default_channel_layout(channels);
        frame->sample_rate = rate;
        frame->nb_samples = rate / 50;
        av_frame_get_buffer(frame, 0);
        av_samples_set_silence(frame->extended_data, 0, frame->nb_samples, channels, format);
        wav->write(frame);
        av_frame_unref(frame);
    }
    av_frame_free(&frame);
}
#endif

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <output directory>\n";
        exit(1);
    }

    auto output_dir = std::string(argv[1]);
    int channels = 2;
    int sampling_rate = 44100;

    test_sample<uint8_t>(output_dir + "u8.wav", channels, sampling_rate);
    test_sample<int16_t>(output_dir + "s16.wav", channels, sampling_rate);
    test_sample<int32_t>(output_dir + "s32.wav", channels, sampling_rate);
    test_sample<int64_t>(output_dir + "s64.wav", channels, sampling_rate);
    test_sample<float>(output_dir + "f32.wav", channels, sampling_rate);
    test_sample<double>(output_dir + "f64.wav", channels, sampling_rate);

    test_vector<uint8_t>(output_dir + "vu8.wav", channels, sampling_rate);
    test_vector<int16_t>(output_dir + "vs16.wav", channels, sampling_rate);
    test_vector<int32_t>(output_dir + "vs32.wav", channels, sampling_rate);
    test_vector<int64_t>(output_dir + "vs64.wav", channels, sampling_rate);
    test_vector<float>(output_dir + "vf32.wav", channels, sampling_rate);
    test_vector<double>(output_dir + "vf64.wav", channels, sampling_rate);

    test_vector_vector<uint8_t>(output_dir + "vvu8.wav", channels, sampling_rate);
    test_vector_vector<int16_t>(output_dir + "vvs16.wav", channels, sampling_rate);
    test_vector_vector<int32_t>(output_dir + "vvs32.wav", channels, sampling_rate);
    test_vector_vector<int64_t>(output_dir + "vvs64.wav", channels, sampling_rate);
    test_vector_vector<float>(output_dir + "vvf32.wav", channels, sampling_rate);
    test_vector_vector<double>(output_dir + "vvf64.wav", channels, sampling_rate);

#ifdef WF_AVFRAME
    test_frame<uint8_t>(output_dir + "fu8.wav", channels, sampling_rate, false);
    test_frame<int16_t>(output_dir + "fs16.wav", channels, sampling_rate, false);
    test_frame<int32_t>(output_dir + "fs32.wav", channels, sampling_rate, false);
    test_frame<int64_t>(output_dir + "fs64.wav", channels, sampling_rate, false);
    test_frame<float>(output_dir + "ff32.wav", channels, sampling_rate, false);
    test_frame<double>(output_dir + "ff64.wav", channels, sampling_rate, false);

    test_frame<uint8_t>(output_dir + "fu8p.wav", channels, sampling_rate, true);
    test_frame<int16_t>(output_dir + "fs16p.wav", channels, sampling_rate, true);
    test_frame<int32_t>(output_dir + "fs32p.wav", channels, sampling_rate, true);
    test_frame<int64_t>(output_dir + "fs64p.wav", channels, sampling_rate, true);
    test_frame<float>(output_dir + "ff32p.wav", channels, sampling_rate, true);
    test_frame<double>(output_dir + "ff64p.wav", channels, sampling_rate, true);
#endif

    return 0;
}
