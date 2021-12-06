#include "headers.hpp"

# include "User.hpp"

/*
	Command: NICK
	Parameters: <nickname>

	NICK command is used to give user a nickname or change the existing
	one.

	Numeric Replies:

		ERR_NONICKNAMEGIVEN             ERR_ERRONEUSNICKNAME
		ERR_NICKNAMEINUSE               ERR_NICKCOLLISION
		ERR_UNAVAILRESOURCE             ERR_RESTRICTED
*/

string		implode( vector<string>::iterator begin, vector<string>::iterator end )
{
	const char* const delim = " ";

	ostringstream str;
	copy(begin, end, ostream_iterator<string>(str, delim));

	return (str.str());
}

bool		is_alpha( string s )
{
	for (string::iterator it = s.begin(); it != s.end(); it++)
		if (!isalpha(*it))
			return true;
	
	return false;
}

void		nick( vector<string> args, User &usr, Server &srv )
{
	if (args.size() == 1)
	{
		send_error(usr, ERR_NEEDMOREPARAMS, args[0]);
		return ;
	}
	else if (args.size() > 2 || !is_alpha(args[1]))
	{
		send_error(usr, ERR_ERRONEUSNICKNAME, implode(args.begin() + 1, args.end()));
		return ;
	}
	args[1].pop_back();
	if (srv.is_registered(usr) && usr.getNick() == args[1])
		return ;

	map<int, User>	usrs = srv.getUsers();

	for (map<int, User>::iterator it = usrs.begin(); it != usrs.end(); it++)
		if (it->second.getNick() == args[1])
		{
			send_error(usr, ERR_NICKNAMEINUSE, args[1]);
			return ;
		}

	if (srv.is_registered(usr)) {
		cout << MAGENTA << usr.getNick() << ": Nick changed to " << args[1] << RESET << endl;
		usr.setNick(args[1]);
	}
	// else
	// 	user();
}