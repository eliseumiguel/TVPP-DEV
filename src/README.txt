========================================
=                                      =
= TVPP Guide to Installation and Usage =
=                                      =
========================================

1 - INTRODUCTION
================

Dear user,

This is a simple guide to compiling your own TVPP apps (bootstrap and client).
This guide will guide you step-by-step through the process of compiling TVPP
from the source for Windows and Linux OSs.

In the end of the guide you will find an explanation on how to create a simple
local experiment using VLC. Use this example to test your new compiled code.

2 - Dependencies
================

To compile and execute TVPP there is a package requirement that needs to be
met. You need to have: 

* Boost package 1.46.0 or greater.

Linux
-----

Linux users can easily compile TVPP if they have boost installed. In order to
install boost and other dependencies one can use this command on Debian-based
distributions:

* sudo apt-get install gcc g++ make libboost-all-dev

Windows
-------

Windows users can compile TVPP through cygwin.

For that, download Cygwin from here: http://www.cygwin.com/

While installing Cygwin one must choose what packages to install. Below are the
package requirements.

* Devel > gcc gcc-g++ make
* Libs > libboost-devel

3 - Compiling
=============

After installing the dependencies, go to the TVPP source directory. The main
TVPP Makefile points to a bin directory where the executables will be placed
after compilation (the BIN_DIR variable), edit it to match your environment.

Inside the main source directory, run 'make' and enjoy.

NOTE: Windows users will need a dll on the same directory as the binaries in
order to execute them (cygboost_thread-mt-*_**.dll). This is obtainable online.

4 - Usage
=========

TVPP has an up-to-date --help command-line switch to explain each parameter. To
avoid out-of-date information, we won't explain the command line parameters
here. Try out

$ ./bootstrap --help
$ ./client --help

5 - Simple Experiment
=====================

We will now explain a simple local experiment. This does not excercise the P2P
capabilities of TVPP, but it illustrates how to put the system to work. Besides
the TVPP binaries you just compiled, you will also need a copy of VLC
(www.videolan.org/vlc/) installed and a media file to be played. The process
explained below is somewhat abstract; if the suggestions don't work, keep in
mind what must be done and try to adapt to your environment.

In short, we will setup the following workflow.

---------   
|       |   
| media |
|       |   
---------   
    |
    v
--------   ----------   ----------   -------
|      |   |        |   |        |   |     |
| cvlc |-->| server |-->| client |-->| vlc |
|      |   |        |   |        |   |     |
--------   ----------   ----------   -------

CVLC will read the media file and stream it to a socket. A TVPP client, acting
as a server, will read from this socket and distribute the media to any other
peers. We will then connect a TVPP client to the server, and this client will
send the media received from the server to a VLC instance for rendering.

1) A media stream source must be created. Using VLC through command line at
Linux one can do it like this:

$ cvlc <media_file> --sout "#std{access=udp,mux=ts,dst=127.0.0.1:PORT}" --loop

Note that PORT above must be a free port number provided by the user, e.g., 51511.

2) Start bootstrap.

$ ./bootstrap

3) Start the TVPP peer that will work as a server.

$ ./client <bootstrap_ip_addr> -mode 1 -streamingPort PORT

Remember that several parameters are set by default, like the channel_id and
several ports. The --help switch can help you understand all possible options.
The -mode parameter identifies if a client is the source or other kind of peer.
The streaming port number, PORT, must be the same passed to cvlc in step 1
(e.g., 51511). Also, <bootstrap_ip_addr> is the IP address where bootstrap was
initialized.

4) Start a client peer.

$ ./client <bootstrap_ip_addr> -peerPort PEERPORT -streamingPort STREAMPORT

This client is configured as a client peer. Since its a local experiment
PEERPORT and STREAMPORT need to be set to avoid conflict with the other local
client. PEERPORT is used as a socket for P2P communication, while STREAMPORT is
used to communicate with encoder/decoder (e.g. VLC). You will need to declare a
different PEERPORT and STREAMPORT for each logical client in a host.

5) Start a VLC and capture the client peer stream.

$ vlc

Once the visual interface appears, choose the option Media > Open network
stream > http://127.0.0.1:STREAMPORT

Note that the STREAMPORT parameter should be the same as in step 4.
