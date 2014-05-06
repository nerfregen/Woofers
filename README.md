Woofers: A Hero's Best Friend
=============================

This project is an attempt to build a translation layer for the City of Heroes
network protocol. I am not calling it a "server" because then players would
expect the main program to provide gameplay functionality. That's not the
purpose here, even if some leaked for the initial release. The purpose of
Woofers is to serve as a link between the game client and a game server; with
a simple, unobfuscated and easy to implement protocol, anyone will be able to
then write modules for it to handle any function of gameplay, in any language
they usually write software with.

What about SEGS and SCORE?
==========================

I started this project because SEGS and SCORE are both making no progress, or
at least not showing any public progress. I believe that this development
should be public in order to attract more developers that can help. SEGS
supports only an old, outdated client; Woofers supports the latest Issue 24
beta client, and all development will happen at this level. Meanwhile SCORE
is conducting all its development in secret, and after a year and a half, the
lack of progress is disheartening. We welcome any developers from either
that wants to contribute to Woofers, or to any module written in it.

How do I connect?
=================

If you have a copy of the Issue 24 beta files, you can connect to Woofers by
opening a command window in the main game directory and executing:

    cityofheroes -project coh -auth 127.0.0.1

You can also create a shortcut with those parameters. Add -console if you want
to see some extra details about what the game is doing.

For the initial release, Woofers is faking the Auth Server functionality by
itself. It creates a hash out of your username, and uses that as a password.
So you can get past the login screen by entering any username and the same
username as a password.

Roadmap
=======

This initial release is not very well written, as most of the code is on the
proof of concept level. We need more programmers that are experienced with
actually writing servers, as well as people who can write documentation for
both the original protocol and Woofer's own. The end goal is for Woofers to
handle requests from the Auth Server, DB Server, and Map Server, and forward
them to the appropriate modules; so someone with experience and expertise in
inter-process communication would be a great help.

Language and platform
=====================

Woofers has been written in Visual Studio 2013 Express, that anyone can
download for free from the Microsoft website. I have no experience in Linux
porting, so someone with the experience to make a gcc port would be a great
asset. I have tested Woofers in Linux using Wine, and it works properly.

Why the name Woofers?
=====================

Woofers is the Freedom Phalanx's canine member. More information in this page:
http://paragonwiki.com/wiki/Woofers
