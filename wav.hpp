#include <fstream>
#include <locale>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace wav {

template<typename SampleFormat>
class WavFile
{
private:
    std::ofstream os;
    size_t factChunk {0};
    size_t dataChunk {0};
    size_t length {0};

public:
    WavFile(std::string filename, int channels, int sampling_rate)
    {
        static_assert(std::is_arithmetic<SampleFormat>::value, "Sample type must be integral or floating point");
        os = std::ofstream(filename, std::ios_base::out | std::ios_base::binary);
        os.imbue(std::locale::classic());
        os << "RIFF----WAVEfmt ";
        if (std::is_integral<SampleFormat>::value) {
            write(16, 4);
            write(1, 2);
            write(channels, 2);
            write(sampling_rate, 4);
            write(sampling_rate * sizeof(SampleFormat) * channels, 4);
            write(sizeof(SampleFormat) * channels, 2);
            write(8 * sizeof(SampleFormat), 2);
            os << "data";
            dataChunk = os.tellp();
            os << "----";
        } else if (std::is_floating_point<SampleFormat>::value) {
            write(18, 4);
            write(3, 2);
            write(channels, 2);
            write(sampling_rate, 4);
            write(sampling_rate * sizeof(SampleFormat) * channels, 4);
            write(sizeof(SampleFormat) * channels, 2);
            write(8 * sizeof(SampleFormat), 2);
            write(0, 2);
            os << "fact";
            write(4, 4);
            factChunk = os.tellp();
            os << "----";
            os << "data";
            dataChunk = os.tellp();
            os << "----";
        }
    }

    ~WavFile()
    {
        length = os.tellp();
        os.seekp(dataChunk);
        write(length - dataChunk + 4, 4); // bytes_per_sample * channels * nb_samples
        os.seekp(4);
        write(length - 8, 4);
        if (factChunk) {
            os.seekp(factChunk);
            write((length - dataChunk + 4) / sizeof(SampleFormat), 4); // channels * nb_samples
        }
    }

    template<typename Word>
    void write(Word value, unsigned size = sizeof(Word))
    {
        auto p = reinterpret_cast<unsigned char const *>(&value);
        for (int i = 0; size; --size, ++i)
            os.put(p[i]);
    }

    template<typename Word>
    void write(std::vector<Word> samples, unsigned sizePerSample = sizeof(Word))
    {
        for (auto sample : samples)
            write(sample, sizePerSample);
    }

    template<typename Word>
    void write(std::vector<std::vector<Word>> samples, unsigned sizePerSample = sizeof(Word))
    {
        for (int i = 0; i < samples[0].size(); ++i)
            for (int c = 0; c < samples.size(); ++c)
                write(samples[c][i], sizePerSample);
    }
};

}
