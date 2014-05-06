#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

namespace AccountManager
{
	void L2Hash(char *password, char *hash);
	void MD5Hash(char *password, char *hash, int salt);

	class Account
	{
	public:
		Account();

		// Initialize should do startup-time validation of settings,
		// attempt to connect to a given server (if any) and return
		// false if for some reason the module won't work correctly.
		virtual bool Initialize();

		// LogIn should validate the given username and password and
		// return true if the account was logged in successfully.
		virtual bool LogIn();

		// LogOut should perform cleanup and free any resources.
		// It doesn't matter if something fails, the user is assumed
		// to be logged off anyway.
		virtual void LogOut();

	private:
		char *account;
		char *password;
		int uid;
		int cookie;
		int salt = 0;
	};
}

#endif
