#pragma comment(lib, "ws2_32.lib") 
#pragma comment(lib, "cryptlib.lib")
#pragma comment(lib, "minini.lib")

#include <iostream>

#include "cryptopp/rsa.h"
#include "cryptopp/osrng.h"

#include "minini\minIni.h"

#include "IniHelper.h"
#include "Util.h"

#include "Packet.h"
#include "AuthServer.h"

#include "FakeAuth.h"

namespace Woofers
{
	static CryptoPP::AutoSeededRandomPool rsaRng;
	static CryptoPP::InvertibleRSAFunction rsaParameters;
	static CryptoPP::RSA::PrivateKey rsaPrivateKey(rsaParameters);
	static int rsaBufferSize;
	static char *rsaBuffer;

	// Generate a RSA key pair and write the modulus and exponent to rsaBuffer;
	// the same key will be used for all connections in this session.
	void rsa_init()
	{
		rsaPrivateKey.GenerateRandomWithKeySize(rsaRng, 1152);
		byte *rsaExpData;
		byte *rsaModData;
		int rsaExpSize = (int)rsaPrivateKey.GetPublicExponent().MinEncodedSize();
		int rsaModSize = (int)rsaPrivateKey.GetModulus().MinEncodedSize();
		rsaExpData = new byte[rsaExpSize];
		rsaModData = new byte[rsaModSize];
		rsaPrivateKey.GetPublicExponent().Encode(rsaExpData, rsaExpSize);
		rsaPrivateKey.GetModulus().Encode(rsaModData, rsaModSize);

		rsaBufferSize = 8 + rsaExpSize + rsaModSize;
		rsaBuffer = new char[rsaBufferSize];
		memcpy(&rsaBuffer[0], &rsaExpSize, 4);
		memcpy(&rsaBuffer[4], &rsaExpData[0], rsaExpSize);
		memcpy(&rsaBuffer[4 + rsaExpSize], &rsaModSize, 4);
		memcpy(&rsaBuffer[8 + rsaExpSize], &rsaModData[0], rsaModSize);
	}

	class AuthServer
	{
	public:
		static DWORD WINAPI ConnThread(LPVOID lpParam)
		{
			SOCKET sock = (SOCKET)lpParam;
			char authAccount[16] = {};	// Account is max 14 bytes, but FakeAuth uses it as a password; pad to 16 bytes.
			int authUserId = 12345;	// User ID in the database, this obviously should be different for each user.
			char lastServer = 1;	// Last server the user logged in; 1 = Freedom.
			int authCookie = rsaRng.GenerateWord32();
			int packsize = 0;
			char buffer[1500];
			int authSalt = 0;	// If this is non-zero the client will encrypt its packets with Blowfish.

			Packet authPacket(sock, AUTHSERVER_HELLO);
			authPacket.AddValue(authSalt, 4);
			authPacket.AddValue(AUTHSERVER_PROTOCOL_VERSION, 4);
			authPacket.AddBytes(rsaBuffer[0], rsaBufferSize);
			authPacket.Send();

			while (true)
			{
				// Encrypted payload size.
				int rsaPayloadLen = 0;
				int checkUserId, checkCookie;	// To check against authUserId and authCookie.

				// Receive the packet length.
				int r = recv(sock, buffer, 2000, 0);
				if (r < 1) { break; }
				memcpy(&packsize, &buffer[0], 2);

				if (packsize != r)
				{
					std::cout << "Packet size doesn't match data received: " << packsize << " != " << r << std::endl;
					return false;
				}

				if (buffer[2] == AUTHCLIENT_LOGIN)
				{
					// The message is always going to be as long as the RSA key size (1152 bits = 144 bytes)
					memcpy(&rsaPayloadLen, &buffer[3], 4);
					if (rsaPayloadLen != 144)
					{
						authPacket.Reset(AUTHSERVER_LOGIN_FAILED);
						authPacket.AddValue(AUTHREASON_INVALID_AUTH, 1);
						authPacket.Send();
					}

					int rsaPlainSize;
					byte *rsaBuffer;

					rsaBuffer = new byte[rsaPayloadLen];
					memcpy(&rsaBuffer[0], &buffer[7], rsaPayloadLen);

					CryptoPP::SecByteBlock rsaCypher(rsaBuffer, rsaPayloadLen);
					CryptoPP::RSAES_OAEP_SHA_Decryptor rsaDecryptor(rsaPrivateKey);

					rsaPlainSize = (int)rsaDecryptor.MaxPlaintextLength(rsaCypher.size());
					CryptoPP::SecByteBlock rsaPlain(rsaPlainSize);
					CryptoPP::DecodingResult rsaResult = rsaDecryptor.Decrypt(rsaRng, rsaCypher, rsaCypher.size(), rsaPlain);
					rsaPlain.resize(rsaResult.messageLength);

					// Check the password.
					char hashLocal[16];		// This is the locally-calculated hash; usually would come from the database.
					char hashRemote[16];	// This is the hash that the client sent.

					memcpy(&authAccount[0], &rsaPlain[0], 14);
					memcpy(&hashRemote[0], &rsaPlain[14], 16);

					AccountManager::MD5Hash(authAccount, hashLocal, authSalt);	// Take the plaintext account and make a password hash out of it.

					if (!memcmp(hashLocal, hashRemote, 16))
					{
						authPacket.Reset(AUTHSERVER_LOGIN_OK);
						authPacket.AddValue(authUserId, 4);
						authPacket.AddValue(authCookie, 4);
						authPacket.AddValue(0, 4);
						authPacket.AddValue(0, 4);
						authPacket.AddValue(1000, 4);	// Account state: 1000 is VIP.
						authPacket.AddValue(10, 4);		// Loyalty points
						authPacket.AddValue(0, 4);
						authPacket.AddValue(0, 4);
						authPacket.AddValue(0, 4);
						authPacket.AddValue(0, 4);
						authPacket.AddValue(0, 4);		// Going Rogue reactivation status.
						authPacket.Send();
					}
					else
					{
						authPacket.Reset(AUTHSERVER_LOGIN_FAILED);
						authPacket.AddValue(AUTHREASON_INVALID_PASSWORD, 1);
						authPacket.Send();
					}
				}
				else if (buffer[2] == AUTHCLIENT_LOGOUT)
				{
					memcpy(&checkUserId, &buffer[3], 4);
					memcpy(&checkCookie, &buffer[7], 4);
					if (!authUserId || (checkUserId == authUserId) && (checkCookie == authCookie))
					{
						// Only stop this thread if it's a valid logout attempt.
						return true;
					}
				}
				else if (buffer[2] == AUTHCLIENT_SERVER_LIST) {
					char listType;
					memcpy(&checkUserId, &buffer[3], 4);
					memcpy(&checkCookie, &buffer[7], 4);
					memcpy(&listType, &buffer[11], 1);
					if ((checkUserId != authUserId) || (checkCookie != authCookie))
					{
						authPacket.Reset(AUTHSERVER_LOGIN_FAILED);
						authPacket.AddValue(AUTHREASON_INVALID_AUTH, 1);
						authPacket.Send();
					}
					else if (listType == 1)
					{
						int ipAddress[4];
						Util::IpToBytes(Woofers::IniHelper::Instance().GetString("AccountManager", "Server").c_str(), ipAddress);
						int port = Woofers::IniHelper::Instance().GetLong("AccountManager", "Port");
						int serverId = Woofers::IniHelper::Instance().GetLong("AccountManager", "Shard");
						char numServers = 1;
						int bufferLoc = 5;	// Start adding servers at this array index and update it.

						// Return the server from the FakeAuth config for now.
						authPacket.Reset(AUTHSERVER_SERVER_LIST);
						authPacket.AddValue(numServers, 1);
						authPacket.AddValue(lastServer, 1);

						for (int i = 0; i < numServers; i++)
						{
							authPacket.AddValue(serverId, 1);	// Shard ID
							// IP Address (4 bytes)
							for (int j = 0; j < 4; j++)
							{
								authPacket.AddValue(ipAddress[j], 1);
							}
							authPacket.AddValue(port, 4);	// DbServer UDP Port
							authPacket.AddValue(0, 2);		// Age Limit and PVP Server flags.
							authPacket.AddValue((numServers - i) * 100, 2);	// Number of players logged in.
							authPacket.AddValue(2000, 2);	// Max players that can log in.
							authPacket.AddValue(1, 1);		// Server status (1 = online, 0 = offline)
							authPacket.AddValue(0, 1);		// Is this a VIP server?
						}
						authPacket.Send();
					}
					else
					{
						// The server requested no server list (0x00) or an extended server list format (0x03).
						authPacket.Reset(AUTHSERVER_NO_SERVERS);
						authPacket.AddValue(AUTHREASON_NO_SERVERS, 1);
						authPacket.Send();
					}
				}
				else if (buffer[2] == AUTHCLIENT_SERVER_SELECTED)
				{
					lastServer = buffer[11];
					memcpy(&checkUserId, &buffer[3], 4);
					memcpy(&checkCookie, &buffer[7], 4);
					if ((checkUserId != authUserId) || (checkCookie != authCookie))
					{
						authPacket.Reset(AUTHSERVER_LOGIN_FAILED);
						authPacket.AddValue(AUTHREASON_INVALID_AUTH, 1);
						authPacket.Send();
					}
					else
					{
						authPacket.Reset(AUTHSERVER_IN_GAME);
						authPacket.AddValue(authCookie, 4);
						authPacket.AddValue(authUserId, 4);
						authPacket.AddValue(lastServer, 1);
						authPacket.Send();
					}
				}
				else
				{
					Util::HexDump("AuthServer can't process this packet:", buffer, r);
				}
			}
			return true;
		}

		/************************************************************************/
		/* Initializes PacketHandler before starting the main loop; returns false
		/* if any part fails.
		/************************************************************************/
		bool Initialize()
		{
			std::cout << "Initializing Crypto++" << std::endl;
			rsa_init();

			std::cout << "Loading Winsock" << std::endl;
			if (WSAStartup(MAKEWORD(2, 2), &_wsd) != 0)
			{
				std::cout << "Failed to load Winsock!" << std::endl;
				return false;
			}

			// Create our listening socket
			//
			_sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
			if (_sListen == SOCKET_ERROR)
			{
				std::cout << "socket() failed: " << WSAGetLastError() << std::endl;
				return false;
			}
			// Select the local interface and bind to it
			//
			_local.sin_addr.s_addr = htonl(INADDR_ANY);
			_local.sin_family = AF_INET;
			_local.sin_port = htons(AUTHSERVER_PORT);

			if (bind(_sListen, (struct sockaddr *)&_local,
				sizeof(_local)) == SOCKET_ERROR)
			{
				std::cout << "bind() failed: " << WSAGetLastError() << std::endl;
				return false;
			}
			listen(_sListen, 8);
			return true;
		}

		/************************************************************************/
		/* Start main loop, will not return until shutdown.
		/************************************************************************/
		void Start()
		{
			//
			// In a continous loop, wait for incoming clients. Once one
			// is detected, create a thread and pass the handle off to it.
			//
			std::cout << "Waiting for connection." << std::endl;
			while (_isRunning)
			{
				_iAddrSize = sizeof(_client);
				_sClient = accept(_sListen, (struct sockaddr *)&_client, &_iAddrSize);
				if (_sClient == INVALID_SOCKET)
				{
					std::cout << "accept() failed: " << WSAGetLastError() << std::endl;
					break;
				}
				std::cout << "Accepted client: " << inet_ntoa(_client.sin_addr) << ":" << ntohs(_client.sin_port) << std::endl;

				_hThread = CreateThread(NULL, 0, ConnThread, (LPVOID)_sClient, 0, &_dwThreadId);
				if (!_hThread)
				{
					std::cout << "CreateThread() failed: " << GetLastError() << std::endl;
					break;
				}
				CloseHandle(_hThread);
			}
		}

		/************************************************************************/
		/* Sets the isRunning flag to false, signalling the main loop to
		/* exit on next iteration.
		/************************************************************************/
		void Shutdown()
		{
			_isRunning = false;
		}

		static AuthServer &GetInstance()
		{
			static AuthServer handler;
			return handler;
		}

		~AuthServer()
		{
			closesocket(_sListen);
			WSACleanup();
		}

	private:
		AuthServer() :
			_wsd(),
			_sListen(),
			_sClient(),
			_iAddrSize(),
			_hThread(),
			_dwThreadId(),
			_local(),
			_client(),
			_isRunning(true)
		{}

		WSADATA		_wsd;
		SOCKET		_sListen, _sClient;
		int			_iAddrSize;
		HANDLE		_hThread;
		DWORD		_dwThreadId;
		struct sockaddr_in _local, _client;
		bool		_isRunning;
	};
}

int main()
{
	{
		AccountManager::Account *account;

		std::cout << std::string(60, '=') << std::endl;
		std::cout << " Woofers" << std::string(31, ' ') << "A Hero's Best Friend" << std::endl << std::endl;
		std::cout << " Network Protocol Translator for City of Heroes" << std::string(7, ' ') << "v0.01" << std::endl;
		std::cout << std::string(60, '=') << std::endl << std::endl;

		if (Woofers::IniHelper::Instance().GetString("AccountManager", "Protocol") == "FakeAuth")
			account = new AccountManager::FakeAuth();
		else
			account = new AccountManager::Account();

		account->Initialize();

		Woofers::AuthServer &authServer = Woofers::AuthServer::GetInstance();
		if (!authServer.Initialize())
		{
			return 1;
		}

		authServer.Start();
	}

	return 0;
}
