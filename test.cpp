#include "wav.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

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

    return 0;
}
