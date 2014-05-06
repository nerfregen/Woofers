#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <iostream>
#include "cryptopp/md5.h"

#include "AccountManager.h"

namespace AccountManager
{
	// Generates a 16-byte hash from a 16-byte string.
	// This is how the password will be stored in the database.
	void L2Hash(char *password, char *hash)
	{
		char temp = 0;
		int tempHash[4];

		memcpy(&tempHash[0], password, 16);

		tempHash[0] = tempHash[0] * 0x3407f + 0x269735;
		tempHash[1] = tempHash[1] * 0x340ff + 0x269741;
		tempHash[2] = tempHash[2] * 0x340d3 + 0x269935;
		tempHash[3] = tempHash[3] * 0x3433d + 0x269ACD;

		memcpy(hash, &tempHash[0], 16);

		for (int i = 0; i < 16; i++)
		{
			temp = temp ^ hash[i] ^ password[i];
			if (temp == 0)
			{
				hash[i] = 0x66;
			}
			else
			{
				hash[i] = temp;
			}
		}
	}

	// Generates the MD5 hash sent by the COH game client.
	// Salt must be the same value sent in the AUTHSERVER_HELLO packet.
	void MD5Hash(char *password, char *hash, int salt)
	{
		char temp[16];

		CryptoPP::Weak::MD5 md5Hash;

		// First, get the Lineage 2 hash.
		L2Hash(password, temp);
		md5Hash.Update((byte *)temp, 16);

		// Now attach the AuthServer salt, in decimal string form.
		_itoa_s(salt, temp, 16, 10);
		md5Hash.Update((byte *)temp, strlen(temp));

		// Get the MD5 hash of the result. This is what the client sends.
		md5Hash.Final((byte *)hash);
	}

	Account::Account() {};

	bool Account::Initialize()
	{
		std::cout << "Initialized Account" << std::endl;
		return true;
	}
	bool Account::LogIn()
	{
		std::cout << "Logged In Account" << std::endl;
		return true;
	}
	void Account::LogOut()
	{
		std::cout << "Logged Out Account" << std::endl;
	}
}
