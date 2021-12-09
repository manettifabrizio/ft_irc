#include "headers.hpp"

/*
	Command: MODE

	The MODE command is a dual-purpose command in IRC.  It allows both
	usernames and channels to have their mode changed.  The rationale for
	this choice is that one day nicknames will be obsolete and the
	equivalent property will be the channel.

	When parsing MODE messages, it is recommended that the entire message
	be parsed first and then the changes which resulted then passed on.

	1. Channel modes

		Parameters: <channel> {[+|-]|o|p|s|i|t|n|b|v} [<limit>] [<user>]
			[<ban mask>]

		The MODE command is provided so that channel operators may change the
		characteristics of `their' channel.  It is also required that servers
		be able to change channel modes so that channel operators may be
		created.

		The various modes available for channels are as follows:

			o - give/take channel operator privileges;
			p - private channel flag;
			s - secret channel flag;
			i - invite-only channel flag;
			t - topic settable by channel operator only flag;
			n - no messages to channel from clients on the outside;
			m - moderated channel;
			l - set the user limit to channel;
			b - set a ban mask to keep users out;
			v - give/take the ability to speak on a moderated channel;
			k - set a channel key (password).

		When using the 'o' and 'b' options, a restriction on a total of three
		per mode command has been imposed.  That is, any combination of 'o'
		and ...

		Numeric Replies:

           ERR_NEEDMOREPARAMS              RPL_CHANNELMODEIS
           ERR_CHANOPRIVSNEEDED            ERR_NOSUCHNICK
           ERR_NOTONCHANNEL                ERR_KEYSET
           RPL_BANLIST                     RPL_ENDOFBANLIST
           ERR_UNKNOWNMODE                 ERR_NOSUCHCHANNEL

		Use of Channel Modes:

			MODE #Finnish +im               ; Makes #Finnish channel moderated and
                                				'invite-only'.
			MODE #Finnish +o Kilroy         ; Gives 'chanop' privileges to Kilroy on
			MODE #Finnish +v Wiz            ; Allow WiZ to speak on #Finnish.
			MODE #Fins -s                   ; Removes 'secret' flag from channel
												#Fins.
			MODE #42 +k oulu                ; Set the channel key to "oulu".
			MODE #eu-opers +l 10            ; Set the limit for the number of users
												on channel to 10.
			MODE &oulu +b                   ; list ban masks set for channel.
			MODE &oulu +b *!*@*             ; prevent all users from joining.
			MODE &oulu +b *!*@*.edu         ; prevent any user from a hostname
												matching *.edu from joining.

	2. User modes

		Parameters: <nickname> {[+|-]|i|w|s|o}

		The user MODEs are typically changes which affect either how the
		client is seen by others or what 'extra' messages the client is sent.
		A user MODE command may only be accepted if both the sender of the
		message and the nickname given as a parameter are both the same.

		The available modes are as follows:

           i - marks a users as invisible;
           s - marks a user for receipt of server notices;
           w - user receives wallops;
           o - operator flag.

		Additional modes may be available later on.

		If a user attempts to make themselves an operator using the "+o"
		flag, the attempt should be ignored.  There is no restriction,
		however, on anyone `deopping' themselves (using "-o").  
		
		Numeric Replies:

			ERR_USERSDONTMATCH              RPL_UMODEIS
			ERR_UMODEUNKNOWNFLAG

		Use of user Modes:

			:MODE WiZ -w                    ; turns reception of WALLOPS messages
												off for WiZ.
			:Angel MODE Angel +i            ; Message from Angel to make themselves
												invisible.
			MODE WiZ -o                     ; WiZ 'deopping' (removing operator
												status).  The plain reverse of this
												command ("MODE WiZ +o") must not be
												allowed from users since would bypass
												the OPER command.
*/

void		cnl_mode( vector<string> args, User &usr, Server &srv ) {

	string		usr_mode = usr.getMode();
	string		knw_mode = "opsitnbv";
	Channel *	cnl;

	if (args[0][0] == '#')
		cnl = srv.getChannelByName( &args[0][1] );
	else if (args[0][0] == '&')
		cnl = srv.getChannelByKey( &args[0][1] );
	else 
		cnl = NULL;
	
	if ( !cnl ) {
		send_error(usr, ERR_NOSUCHNICK, args[0]);
		return ;
	}

	string		cnl_mode = cnl->getMode();
	char 		flag = args[1][0];
	string 		mode = &args[1][1];

	for (size_t i = 0; i < mode.size(); i++) {
		if ( knw_mode.find(mode[i]) == string::npos ) {
			send_error(usr, ERR_UNKNOWNMODE, args[0]);
			return ;
		}
	}

	if ( flag == '+' ) {
		for (size_t i = 0; i < mode.size(); i++) {
			if ( cnl_mode.find(mode[i]) == string::npos && mode[i] != 'o')
				cnl_mode += mode[i];
		}
	}
	else if ( flag == '-' ) {
		for (size_t i = 0; i < mode.size(); i++) {
			size_t to_remove = cnl_mode.find(mode[i]);
			if ( to_remove != string::npos )
				cnl_mode.erase(cnl_mode.begin() + to_remove);
		}
	}

	cnl->setMode(cnl_mode);

	send_reply(usr, 324, RPL_CHANNELMODEIS(cnl->getName(), cnl->getMode()));
}

void		usr_mode( vector<string> args, User &usr, Server &srv ) {

	(void)srv;

	string	usr_mode = usr.getMode();
	string	knw_mode = "iswo";

	char flag = args[1][0];
	string mode = &args[1][1];

	if (usr.getNick() != args[0]) {
		send_error(usr, ERR_USERSDONTMATCH, args[0]);
		return ;
	}

	for (size_t i = 0; i < mode.size(); i++) {
		if ( knw_mode.find(mode[i]) == string::npos ) {
			send_error(usr, ERR_UNKNOWNMODE, args[0]);
			return ;
		}
	}

	if ( flag == '+' ) {
		for (size_t i = 0; i < mode.size(); i++) {
			if ( usr_mode.find(mode[i]) == string::npos && mode[i] != 'o')
				usr_mode += mode[i];
		}
	}
	else if ( flag == '-' ) {
		for (size_t i = 0; i < mode.size(); i++) {
			size_t to_remove = usr_mode.find(mode[i]);
			if ( to_remove != string::npos )
				usr_mode.erase(usr_mode.begin() + to_remove);
		}
	}

	usr.setMode(usr_mode);
	send_reply(usr, 221, RPL_UMODEIS(usr.getMode()));
}

void		mode( vector<string> args, User &usr, Server &srv ) {
	
	string cnl_id = "#&";

	if (args.size() < 2) {
		send_error(usr, ERR_NEEDMOREPARAMS, args[0]);
		return ;
	}

	if ( cnl_id.find(args[0]) != string::npos )
		cnl_mode(args, usr, srv);
	else
		usr_mode(args, usr, srv);

}