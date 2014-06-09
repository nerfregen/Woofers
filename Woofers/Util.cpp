#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

namespace Util
{
	// Dumps the contents of the given pointer contents to the console.
	void HexDump(char *label, void *ptr, int buflen) {
		unsigned char *buf = (unsigned char*)ptr;
		int i, j;
		printf("%s\n", label);
		for (i = 0; i < buflen; i += 16) {
			printf("%06x: ", i);
			for (j = 0; j < 16; j++)
			if (i + j < buflen)
				printf("%02x ", buf[i + j]);
			else
				printf("   ");
			printf(" ");
			for (j = 0; j < 16; j++)
			if (i + j < buflen)
				printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
			printf("\n");
		}
	}

	int* IpToBytes(const char *ipAddress, int* arrayToFill)
	{
		std::string workingString = ipAddress;

		for (std::size_t end = workingString.find("."), start = 0, index = 0; 
			 end != std::string::npos; 
			 end = workingString.find(".", start), start = end + 1, index++)
		{
			// Get each section and convert the string to an integer
			arrayToFill[index] = atoi(workingString.substr(start, end - start).c_str());
		}

		return arrayToFill;
	}
}
