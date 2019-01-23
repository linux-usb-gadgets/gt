SPDX-License-Identifier: CC-BY-SA-4.0

Copyright 2019 Collabora Ltd

Author: Andrzej Pietrasiewicz <andrzej.p@collabora.com>

===========================
gt integration with systemd
===========================

-----------------------------
Automate your gadget creation
-----------------------------

Purpose of this document
========================

The purpose of this document is to explain how gt can be integrated with
systemd.

User story
==========

The associated user story is as follows. There is a Linux-based device which
can act as a USB device - or as USB gadget in the Linux kernel's parlance.
Linux kernel running on the device provides only the modern configfs interface
for composing USB gadgets. What the user wants is to have systemd make a USB
gadget with a predefined composition up and running as soon as the underlying
hardware and configfs become available. While this task can be done using a
shell script, the user would be better off using a declarative-style
configuration file(s). The user decides to use gt and wants systemd to handle
everything for them.

The idea
========

The idea is to have a udev rule fired when a UDC becomes available. This
document assumes there is only one UDC in the USB device's system, but see
discussion_. The rule then triggers reaching a systemd target unit which can
run gt service, which in turn loads appropriate gadget scheme, making the
gadget operational. This indirection via the target provides a way to use
systemctl enable and systemctl disable to manage hardware activation of the
service. Some scenarios which are more complex are possible, but see
discussion_.

What is what of what?
=====================

USB
	Universal Serial Bus
USB host
	A machine whose functionality is extended with attached USB devices
USB device
	A machine which extends functionality of USB host, connected to it
	with USB
USB gadget
	USB device implementation in Linux kernel
configfs
	A pseudo file system which is used to create entities in a running
	Linux kernel. The lifetime of the entities is decided by userspace
gt
	A friendly commandline tool for USB gadget creation and modification
	which internally operates on configfs
gadget scheme
	A declarative-style description presenting composition of a USB gadget
udev
	A userspace daemon which receives events from the kernel and interprets
	udev rule files to trigger appropriate activities
udev rule
	A declarative specification of an activity and conditions which must be
	met in order for the activity to be taken
systemd
	A userspace daemon which orchestrates various aspects of a running
	Linux system
systemd unit
	A service, a socket, a device, a mount point, an automount point,
	a swap file or partition, a start-up target, a watched file system
	path, a timer controlled and supervised by systemd, a resource
	management slice or a group of externally created processes, and its
	accompanying specification
systemd unit template
	A parametrized unit, which can be instantiated if actual parameter
	values are provided by systemd
UDC
	USB device controller. A piece of hardware required for the USB device
	to function

Udev rule
=========

.. code-block:: console

	SUBSYSTEM=="udc", ACTION=="add", \
		 TAG+="systemd", ENV{SYSTEMD_WANTS}+="gt.target"


The condition for taking the activity is that there is a kernel "add" event
from the "udc" subsystem, which means a new udc has just been made available.
The device in question is tagged with "systemd", which makes systemd
aware of the new device, and SYSTEMD_WANTS is set to the name of the target we
want to reach if the condition is met.

Systemd
=======

Target unit
-----------

.. code-block:: console

	[Unit]
	Description=Hardware activated USB gadget

The target is a simple target to be reached when the udev rule fires.
The service unit will use a "WantedBy" dependency to lend itself for being
managed with systemctl enable/disable.

Service unit
------------

The unit file is named gt@.service and is a template unit.

.. code-block:: console

	[Unit]
	Description=Load USB gadget scheme
	Requires=sys-kernel-config.mount
	After=sys-kernel-config.mount

	[Service]
	ExecStart=/bin/gt load %i.scheme %i
	RemainAfterExit=yes
	ExecStop=/bin/gt rm -rf %i
	Type=simple

	[Install]
	WantedBy=gt.target

This unit is a template unit, so system administrator is supposed to issue

.. code-block:: console

	systemctl enable gt@<gadget scheme file basename>

This sets up appropriate symbolic links in gt.target.wants directory, which
in turn triggers executing the service unit the usual systemd way.

The service itself uses gt to load gadget scheme with the name implied from
the template parameter, name the gadget accordingly and activate it. Upon
stopping it removes the gadget altogether.

gt installation and configuration
=================================

By default gt binary is installed in /bin, and its configuration file is
/etc/gt/gt.conf. The configuration file contains a few directives. The most
important ones in the context of gt integration with systemd are "lookup-path"
and "default-template-path", which must not be commented in order to take
effect, and which specify the paths where gadget schemes are read from and
stored to by default. Compiled-in defaults reflect the values suggested by
install-time comments in the config file.

Gadget schemes
==============

In this document two schemes are offered:

* RNDIS ethernet + ACM serial
* ECM

Please note that some of the values are placeholders which must be filled
in order for the scheme to be used. For this reason ready to use scheme files
are not provided with this document.
The scheme file name to be used by the setup described in this file must
correspond to what is used in the service unit in gt invocation
(e.g. default.scheme). A symbolic link to actual file can be used.

RNDIS ethernet + ACM serial
---------------------------

.. code-block:: console

	attrs :
	{
	    bcdUSB = 0x200;
	    bDeviceClass = <YOUR DEVICE CLASS>;
	    bDeviceSubClass = <YOUR DEVICE SUBCLASS>;
	    bDeviceProtocol = <YOUR DEVICE PROTOCOL>;
	    bMaxPacketSize0 = <MAX PACKET SIZE FOR ep0>;
	    idVendor = 0x1d6b; # Linux Foundation
	    idProduct = 0x104; # Multifunction composite gadget
	    bcdDevice = 0x100;
	};
	os_descs :
	{
	    use = 1;
	    qw_sign = "MSFT100";
	    b_vendor_code = 0xcd;
	};
	strings = (
		{
			lang = 0x409;
			manufacturer = "<YOUR NAME>";
			product = "<YOUR PRODUCT NAME>";
		}
	);
	functions :
	{
	    acm_usb0 :
	    {
		instance = "usb0";
		type = "acm";
	    };
	    rndis_usb0 :
	    {
		instance = "usb0";
		type = "rndis";
		os_descs = (
		    {
			interface = "rndis";
			compatible_id = "RNDIS";
			sub_compatible_id = <YOUR SUB COMPATIBLE ID>;
		    } );
	    };
	};
	configs = (
	    {
		id = 1;
		name = "c";
		attrs :
		{
		    bmAttributes = 0x80;
		    bMaxPower = <MAX POWER CONSUMPTION IN 2mA UNITS>;
		};
		functions = (
		    {
			name = "acm.usb0";
			function = "acm_usb0";
		    },
		    {
			name = "rndis.usb0";
			function = "rndis_usb0";
		    } );
	    } );

ECM ethernet
------------

.. code-block:: console

	attrs :
	{
	    bcdUSB = 0x200;
	    bDeviceClass = <YOUR DEVICE CLASS>;
	    bDeviceSubClass = <YOUR DEVICE SUBCLASS>;
	    bDeviceProtocol = <YOUR DEVICE PROTOCOL>;
	    bMaxPacketSize0 = <MAX PACKET SIZE FOR ep0>;
	    idVendor = 0x0525; # NetChip
	    idProduct = 0xa4a1; # Ethernet Gadget, donated id
	    bcdDevice = 0x100;
	};
	strings = (
		{
			lang = 0x409;
			manufacturer = "<YOUR NAME>";
			product = "<YOUR PRODUCT NAME>";
		}
	);
	functions :
	{
	    ecm_usb0 :
	    {
		instance = "usb0";
		type = "ecm";
	    };
	};
	configs = (
	    {
		id = 1;
		name = "c";
		attrs :
		{
		    bmAttributes = 0x80;
		    bMaxPower = <MAX POWER CONSUMPTION IN 2mA UNITS>;
		};
		functions = (
		    {
			name = "ecm.usb0";
			function = "ecm_usb0";
		    } );
	    } );

.. _discussion:

Discussion
==========

This document assumes there is only one UDC available in the gadget system.
If there are more, the first one to appear will cause reaching the gt.target,
so the first UDC becoming available will be used for running the gadget.service
on it. This might or might not be what you want.

This document does not address the case where actual USB device functionality
is implemented in userspace on top of FunctionFS. Additional socket and
service units are probably needed for each such functionality.
