#pragma once

#include <stddef.h>

namespace NtQueryObjectSafety
{
	using Address = unsigned __int64;

	inline bool CheckedRangeEnd(Address Start, size_t Length, Address* End)
	{
		constexpr Address MaximumAddress = ~static_cast<Address>(0);
		if (End == nullptr || Length > MaximumAddress - Start)
			return false;

		*End = Start + Length;
		return true;
	}

	inline bool RangeContains(Address Buffer, size_t BufferLength, Address Value, size_t ValueLength)
	{
		Address BufferEnd = 0;
		Address ValueEnd = 0;
		return CheckedRangeEnd(Buffer, BufferLength, &BufferEnd) &&
			CheckedRangeEnd(Value, ValueLength, &ValueEnd) &&
			Value >= Buffer && ValueEnd <= BufferEnd;
	}

	inline bool RangesOverlap(Address First, size_t FirstLength, Address Second, size_t SecondLength)
	{
		Address FirstEnd = 0;
		Address SecondEnd = 0;
		if (!CheckedRangeEnd(First, FirstLength, &FirstEnd) ||
			!CheckedRangeEnd(Second, SecondLength, &SecondEnd))
		{
			// An overflowing range is never safe to post-process.
			return true;
		}

		return First < SecondEnd && Second < FirstEnd;
	}

	inline bool AlignUp(Address Value, size_t Alignment, Address* Result)
	{
		if (Result == nullptr || Alignment == 0 || (Alignment & (Alignment - 1)) != 0)
			return false;

		constexpr Address MaximumAddress = ~static_cast<Address>(0);
		const Address Mask = Alignment - 1;
		if (Value > MaximumAddress - Mask)
			return false;

		*Result = (Value + Mask) & ~Mask;
		return true;
	}
}
