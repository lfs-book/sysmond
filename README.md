# sysmond
The server portion of the client/server application sysmon3/sysmond.  

This is a follow on to sysmon-qt.  A server using sysmond allows 
multiple workstations to monitor system resoures.  A workstation,
in turn, can monitor multiple hosts using multiple instances of
sysmon3.

There are several ways to monitor the system performace on a Linux system.
This includes console based applications like top, htop, free, df, and ps. 

For graphical applications there are desktop based programs including 
gnome-system-monitor, qps (from LXQt), and plasma-systemmonitor.

The problem with the above programs is that they take up a lot of screen 
real estate.  A grapical program that can run continuously but can be 
configured to use a small amount of screen space is GKrellM. 
See http://gkrellm.srcbox.net/

GKrellM is a powerful application, but it no longer seems to be maintained.
It is gtk2 based and that has reached its end-of-life.  There have been
no substantial changes to the program since 2016.

sysmon-qt is a program similar to GKrellM but based on Qt6.  It does not
have all the capabilities of GKrellM, but provides the capability of 
monitoring CPU usage, memory use, and system temperatures.  It is also 
highly configurable with the ability choose which items to monitor and
what colors and fonts to use.

