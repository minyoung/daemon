What is this?
=============
This is me playing around in C.


Why should I care?
==================
No reason.


Why did you start this?
=======================
For fun and for a change of pace from my usual day to day work.

Really though, the more pertinent question you should be asking is:


What started this?
------------------
1. [Let's Write a Linux Daemon](http://narcissisticme.com/2011/01/31/lets-write-a-linux-daemon-part-i/)
2. [Understanding Dynamo with Andy Gross](http://nosqltapes.com/video/understanding-dynamo-with-andy-gross)
3. [Graphite - Enterprise Scalable Realtime Graphing](http://graphite.wikidot.com/)


So what does it do right now?
=============================
* command line argument parsing
* config file loading
* logging
* daemonizing
* unit tests
* listens for UDP traffic
* reads to and writes to files


What do you plan on doing with this?
====================================
* make it useful
* make it robust
* make some web graphing frontends


How do I build this?
====================
`./build.py`


What are the dependencies?
==========================
For building:
[fabricate](http://code.google.com/p/fabricate/ "fabricate: The better build tool.")

Ini file loading:
[inih](http://code.google.com/p/inih/ "INI Not Invented Here")

Unit tests:
[check](http://check.sourceforge.net/ "Check: A unit testing framework for C")

