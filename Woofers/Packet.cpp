#include <winsock2.h>
#include <cstring>

#include "Packet.h"

namespace Woofers
{
	Packet::Packet(SOCKET s, int message) : socket(s)
	{
		// Initializing an Auth Packet here, will need a separate
		// constructor for the main protocol later.
		buffer[2] = (char)message;
		idx = 3;
	}

	// Re-initializes the packet so it can be reused.
	void Packet::Reset(int message)
	{
		buffer[2] = (char)message;
		idx = 3;
	}

	// Adds <size> number of bytes to the packet.
	// Returns the current packet location in bytes.
	void Packet::AddValue(int value, int size)
	{
		memcpy(&buffer[idx], &value, size);
		idx += size;
	}

	// Adds <size> number of bytes to the packet.
	// Returns the current packet location in bytes.
	void Packet::AddBytes(char &ptr, int size)
	{
		memcpy(&buffer[idx], &ptr, size);
		idx += size;
	}

	// Sends an packet. Will need a separate version for
	// the main protocol.
	bool Packet::Send()
	{
		int sent = 0;
		int left = idx;

		// Write the packet size to the first two bytes.
		memcpy(&buffer[0], &idx, 2);

		while (left > 0)
		{
			int r = send(socket, &buffer[sent], left, 0);
			if (r < 1)
			{
				return false;
			}
			left -= r;
			sent += r;
		}

		return true;
	}
}
