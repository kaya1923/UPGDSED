#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <intrin.h>
#include <vector>

struct CpuidResult
{
    int eax;
    int ebx;
    int ecx;
    int edx;
};

static CpuidResult cpuid(int leaf, int subleaf = 0)
{
    CpuidResult result{};
    __cpuidex(&result.eax, leaf, subleaf);
    return result;
}

static void print_leaf(unsigned leaf, const CpuidResult& result)
{
    char vendor[13]{};
    std::memcpy(vendor + 0, &result.ebx, 4);
    std::memcpy(vendor + 4, &result.ecx, 4);
    std::memcpy(vendor + 8, &result.edx, 4);
    for (char& value : vendor)
    {
        if (value != 0 && !std::isprint(static_cast<unsigned char>(value)))
            value = '.';
    }

    std::printf("CPUID %08X: EAX=%08X EBX=%08X ECX=%08X EDX=%08X  text='%s'\n",
        leaf,
        static_cast<unsigned>(result.eax),
        static_cast<unsigned>(result.ebx),
        static_cast<unsigned>(result.ecx),
        static_cast<unsigned>(result.edx),
        vendor);
}

static std::uint64_t timed_cpuid()
{
    int registers[4]{};
    unsigned aux = 0;
    _mm_lfence();
    const auto begin = __rdtsc();
    __cpuidex(registers, 1, 0);
    const auto end = __rdtscp(&aux);
    _mm_lfence();
    return end - begin;
}

int main()
{
    const auto basic = cpuid(0);
    const auto features = cpuid(1);

    std::printf("CPUID visibility check\n");
    std::printf("Highest basic leaf: %08X\n", static_cast<unsigned>(basic.eax));
    std::printf("CPUID.1 ECX:       %08X\n", static_cast<unsigned>(features.ecx));
    std::printf("Hypervisor bit:    %u\n",
        (static_cast<unsigned>(features.ecx) >> 31) & 1U);

    for (unsigned leaf = 0x40000000; leaf <= 0x40000003; ++leaf)
        print_leaf(leaf, cpuid(static_cast<int>(leaf)));

    constexpr std::size_t sample_count = 2000;
    std::vector<std::uint64_t> samples;
    samples.reserve(sample_count);
    for (std::size_t index = 0; index < sample_count; ++index)
        samples.push_back(timed_cpuid());
    std::sort(samples.begin(), samples.end());

    std::printf("CPUID timing (%zu samples): median=%llu cycles, p95=%llu cycles, p99=%llu cycles\n",
        sample_count,
        static_cast<unsigned long long>(samples[sample_count / 2]),
        static_cast<unsigned long long>(samples[(sample_count * 95) / 100]),
        static_cast<unsigned long long>(samples[(sample_count * 99) / 100]));
    std::printf("Compare the full output before and after loading airhv; timing alone is not conclusive.\n");
    return 0;
}
