#ifndef UTIL_H
#define UTIL_H

namespace Util
{
	void HexDump(char *label, void *ptr, int buflen);

	/// <summary>
	/// Turns a IP Address string into an array of integers.
	/// </summary>
	int* IpToBytes(const char* ipAddress, int* arrayToFill);
}

#endif
