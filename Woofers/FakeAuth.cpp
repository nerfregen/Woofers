#include <iostream>
#include "FakeAuth.h"

namespace AccountManager
{
	FakeAuth::FakeAuth() {};

	bool FakeAuth::Initialize()
	{
		std::cout << "Initialized FakeAuth" << std::endl;
		return true;
	}
	bool FakeAuth::LogIn()
	{
		std::cout << "Logged In FakeAuth" << std::endl;
		return true;
	}
	void FakeAuth::LogOut()
	{
		std::cout << "Logged Out FakeAuth" << std::endl;
	}
}
