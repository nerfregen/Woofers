#ifndef ACCOUNTMANAGER_FAKEAUTH_H
#define ACCOUNTMANAGER_FAKEAUTH_H

#include "AccountManager.h"

namespace AccountManager
{
	class FakeAuth : public Account
	{
	public:
		FakeAuth();

		virtual bool Initialize();
		virtual bool LogIn();
		virtual void LogOut();
	};
}

#endif
