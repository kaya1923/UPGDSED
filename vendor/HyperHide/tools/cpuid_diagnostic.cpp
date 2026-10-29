#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <intrin.h>
#include <vector>

namespace
{
    constexpr const char* ToolVersion = "1.0.0-20261029";
    constexpr std::size_t TimingSamples = 4096;

    struct CpuidResult
    {
        std::uint32_t eax;
        std::uint32_t ebx;
        std::uint32_t ecx;
        std::uint32_t edx;
    };

    struct LeafRequest
    {
        std::uint32_t leaf;
        std::uint32_t subleaf;
        const char* label;
    };

    CpuidResult cpuid(std::uint32_t leaf, std::uint32_t subleaf = 0)
    {
        int registers[4]{};
        __cpuidex(registers, static_cast<int>(leaf), static_cast<int>(subleaf));
        return {
            static_cast<std::uint32_t>(registers[0]),
            static_cast<std::uint32_t>(registers[1]),
            static_cast<std::uint32_t>(registers[2]),
            static_cast<std::uint32_t>(registers[3])
        };
    }

    void print_leaf(const LeafRequest& request)
    {
        const auto result = cpuid(request.leaf, request.subleaf);
        std::printf(
            "leaf=%08X subleaf=%08X eax=%08X ebx=%08X ecx=%08X edx=%08X label=%s\n",
            request.leaf, request.subleaf, result.eax, result.ebx, result.ecx, result.edx,
            request.label);
    }

    std::uint64_t timed_cpuid(std::uint32_t leaf, std::uint32_t subleaf)
    {
        int registers[4]{};
        unsigned aux = 0;
        _mm_lfence();
        const auto begin = __rdtsc();
        __cpuidex(registers, static_cast<int>(leaf), static_cast<int>(subleaf));
        const auto end = __rdtscp(&aux);
        _mm_lfence();
        return end - begin;
    }

    void print_timing(std::uint32_t leaf, std::uint32_t subleaf)
    {
        std::vector<std::uint64_t> samples;
        samples.reserve(TimingSamples);
        for (std::size_t index = 0; index < TimingSamples; ++index)
            samples.push_back(timed_cpuid(leaf, subleaf));

        std::sort(samples.begin(), samples.end());
        long double sum = 0;
        for (const auto sample : samples)
            sum += sample;
        const long double mean = sum / samples.size();

        long double squaredDistance = 0;
        for (const auto sample : samples)
        {
            const long double distance = sample - mean;
            squaredDistance += distance * distance;
        }
        const long double standardDeviation = std::sqrt(squaredDistance / samples.size());

        std::printf(
            "timing leaf=%08X subleaf=%08X samples=%zu min=%llu median=%llu p95=%llu "
            "p99=%llu max=%llu mean=%.2Lf stdev=%.2Lf cycles\n",
            leaf, subleaf, samples.size(),
            static_cast<unsigned long long>(samples.front()),
            static_cast<unsigned long long>(samples[samples.size() / 2]),
            static_cast<unsigned long long>(samples[(samples.size() * 95) / 100]),
            static_cast<unsigned long long>(samples[(samples.size() * 99) / 100]),
            static_cast<unsigned long long>(samples.back()), mean, standardDeviation);
    }

    void print_vendor(const char* name, const CpuidResult& value, bool standardOrder)
    {
        char text[13]{};
        std::memcpy(text + 0, &value.ebx, sizeof(value.ebx));
        if (standardOrder)
        {
            std::memcpy(text + 4, &value.edx, sizeof(value.edx));
            std::memcpy(text + 8, &value.ecx, sizeof(value.ecx));
        }
        else
        {
            std::memcpy(text + 4, &value.ecx, sizeof(value.ecx));
            std::memcpy(text + 8, &value.edx, sizeof(value.edx));
        }
        for (auto& character : text)
        {
            const auto byte = static_cast<unsigned char>(character);
            if (character != '\0' && (byte < 0x20 || byte > 0x7E))
                character = '.';
        }
        std::printf("%s='%s'\n", name, text);
    }
}

int main()
{
#if defined(_M_X64)
    constexpr const char* Architecture = "x64";
#elif defined(_M_IX86)
    constexpr const char* Architecture = "x86";
#else
    constexpr const char* Architecture = "unknown";
#endif

    const auto basic = cpuid(0);
    const auto features = cpuid(1);
    const auto structured = cpuid(7, 0);
    const auto extended = cpuid(0x80000000);
    const auto hypervisor = cpuid(0x40000000);
    const std::uint32_t maximumBasic = basic.eax;
    const std::uint32_t maximumExtended = extended.eax;
    const std::uint32_t maximumLeaf7Subleaf = structured.eax;

    std::printf("cpuid_diagnostic version=%s architecture=%s\n", ToolVersion, Architecture);
    std::printf("maximum_basic=%08X maximum_extended=%08X leaf7_max_subleaf=%08X\n",
        maximumBasic, maximumExtended, maximumLeaf7Subleaf);
    std::printf("cpuid_1_ecx=%08X hypervisor_present=%u\n",
        features.ecx, (features.ecx >> 31) & 1U);
    print_vendor("cpu_vendor", basic, true);
    print_vendor("hypervisor_vendor_bytes", hypervisor, false);

    std::vector<LeafRequest> requests = {
        {0x00000000, 0, "basic-root"},
        {0x00000001, 0, "feature-bits"},
        {0x00000007, 0, "structured-features"},
        {0x40000000, 0, "hypervisor-root"},
        {0x40000001, 0, "hypervisor-interface"},
        {0x40000010, 0, "hypervisor-boundary-probe"},
        {0x4FFFFFFF, 0, "hypervisor-range-end"},
        {0x80000000, 0, "extended-root"},
        {0x8FFFFFFF, 0, "extended-range-end"},
        {maximumBasic, 0, "maximum-basic"},
        {maximumExtended, 0, "maximum-extended"},
        {0x00000007, maximumLeaf7Subleaf, "leaf7-maximum-subleaf"}
    };
    if (maximumBasic < 0x3FFFFFFF)
        requests.push_back({maximumBasic + 1, 0, "first-invalid-basic"});
    if (maximumExtended >= 0x80000000 && maximumExtended < 0x8FFFFFFF)
        requests.push_back({maximumExtended + 1, 0, "first-invalid-extended"});
    if (maximumLeaf7Subleaf != 0xFFFFFFFF)
        requests.push_back({0x00000007, maximumLeaf7Subleaf + 1, "first-invalid-leaf7-subleaf"});

    std::sort(requests.begin(), requests.end(), [](const auto& left, const auto& right) {
        if (left.leaf != right.leaf)
            return left.leaf < right.leaf;
        return left.subleaf < right.subleaf;
    });
    requests.erase(std::unique(requests.begin(), requests.end(), [](const auto& left, const auto& right) {
        return left.leaf == right.leaf && left.subleaf == right.subleaf;
    }), requests.end());

    std::puts("-- cpuid leaves --");
    for (const auto& request : requests)
        print_leaf(request);

    std::puts("-- timing distributions --");
    print_timing(0x00000000, 0);
    print_timing(0x00000001, 0);
    print_timing(0x40000000, 0);
    std::puts("Timing distributions are comparative evidence only; no single value proves virtualization.");
    return 0;
}
