#include <cstdint>
#include <cstddef>
#include <iostream>
#include "../HyperHideDrv/NtQueryObjectSafety.h"

namespace
{
	struct NativeUnicodeString64
	{
		std::uint16_t Length;
		std::uint16_t MaximumLength;
		std::uint32_t Padding;
		std::uint64_t Buffer;
	};

	static_assert(sizeof(NativeUnicodeString64) == 16);
	static_assert(offsetof(NativeUnicodeString64, Buffer) == 8);

	int Failures = 0;

	void Check(bool Condition, const char* Name)
	{
		if (!Condition)
		{
			std::cerr << "FAIL: " << Name << '\n';
			++Failures;
		}
	}

	bool CanPostProcess(std::uint64_t Output, std::size_t OutputLength,
		std::uint64_t ReturnLength)
	{
		return ReturnLength == 0 ||
			!NtQueryObjectSafety::RangesOverlap(Output, OutputLength,
				ReturnLength, sizeof(std::uint32_t));
	}

	bool ValidTypeName(std::uint64_t Output, std::size_t OutputLength,
		const NativeUnicodeString64& Name)
	{
		return Name.Length <= Name.MaximumLength &&
			(Name.Length & 1) == 0 && (Name.MaximumLength & 1) == 0 &&
			Name.Buffer != 0 &&
			NtQueryObjectSafety::RangeContains(Output, OutputLength,
				Name.Buffer, Name.MaximumLength);
	}
}

int main()
{
	using namespace NtQueryObjectSafety;
	constexpr Address Output = 0x0528FA70;

	// Exact crash-dump argument layout: ReturnLength at output+8 overlaps the
	// native x64 UNICODE_STRING.Buffer field and must disable post-processing.
	Check(RangesOverlap(Output, 0x100, Output + 8, sizeof(std::uint32_t)),
		"dump overlap at ObjectInformation+8");
	Check(!CanPostProcess(Output, 0x100, Output + 8),
		"class 2 dump layout skips all post-processing");
	Check(CanPostProcess(Output, 0x100, 0),
		"ReturnLength NULL remains supported");
	Check(CanPostProcess(Output, 0x100, Output + 0x200),
		"separate ReturnLength remains supported");
	Check(!RangesOverlap(Output, 0x100, Output + 0x100, sizeof(std::uint32_t)),
		"adjacent ReturnLength is not overlap");
	Check(RangesOverlap(~Address{0} - 1, 8, Output, 4),
		"overflowing range fails closed");

	Check(RangeContains(Output, 0x100, Output + 0x20, 0x10),
		"valid embedded type name");
	Check(!RangeContains(Output, 0x100, Output + 0xF8, 0x10),
		"truncated type name rejected");
	Check(!RangeContains(Output, 0x100, 0x80, 0x10),
		"external type-name pointer rejected");

	NativeUnicodeString64 NormalName{22, 24, 0, Output + 0x40};
	Check(ValidTypeName(Output, 0x100, NormalName),
		"normal native x64 class 2 descriptor accepted");
	NormalName.Buffer = Output + 0xF8;
	Check(!ValidTypeName(Output, 0x100, NormalName),
		"class 2 truncated MaximumLength rejected");
	NormalName = {23, 24, 0, Output + 0x40};
	Check(!ValidTypeName(Output, 0x100, NormalName),
		"odd UTF-16 Length rejected");
	NormalName = {22, 20, 0, Output + 0x40};
	Check(!ValidTypeName(Output, 0x100, NormalName),
		"Length greater than MaximumLength rejected");
	NormalName = {22, 24, 0, 0x80};
	Check(!ValidTypeName(Output, 0x100, NormalName),
		"changed external string pointer fails closed");

	Address Aligned = 0;
	Check(AlignUp(Output + 0x23, 8, &Aligned) && Aligned == Output + 0x28,
		"native x64 entry alignment");
	Check(RangeContains(Output, 0x100, Aligned, 24),
		"class 3 aligned next-entry header fits");
	Check(!AlignUp(~Address{0} - 2, 8, &Aligned),
		"alignment overflow rejected");

	if (Failures != 0)
		return 1;

	std::cout << "NtQueryObject safety regression tests passed\n";
	return 0;
}
