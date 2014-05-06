#ifndef PACKET_H
#define PACKET_H

#include <winsock2.h>

namespace Woofers
{
	class Packet
	{
	public:
		Packet(SOCKET s, int message);
		void Reset(int message);
		bool Send();
		void AddValue(int value, int size);
		void AddBytes(char &ptr, int size);
	private:
		SOCKET socket;
		int idx;
		char buffer[1500];
	};
}

#endif
