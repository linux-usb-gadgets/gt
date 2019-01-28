SPDX-License-Identifier: CC-BY-SA-4.0

Copyright 2019 Collabora Ltd

Author: Andrzej Pietrasiewicz <andrzej.p@collabora.com>

==================
cross compiling gt
==================

Purpose of this document
========================

The purpose of this document is to explaing how gt can be cross-compiled for
arm platform, when not compiling with gadgetd support.

User story
==========

A developer wants to cross compile gt for arm from sources. gt depends on
libusbgx which also needs to be cross compiled. It also depends on libconfig9.

The idea
========

The idea is to have a target root file system with libconfig9 installed and
cross compile and install libusbgx there, and then cross compile and install
gt. The compiler shall be invoked with --sysroot pointing to the target root
file system.

Cross compiling
===============

We assume that ${ROOTFS} contains a path where in the compilation host the
target root file system can be found.

We assume that compilation is for armhf.

The target root file system should already contain libconfig-dev and
libconfig9. They can be installed natively by the target system in its root
file system.

libusbgx
--------

.. code-block:: console

	PKG_CONFIG_PATH=${ROOTFS}/usr/lib/arm-linux-gnueabihf/pkgconfig	\
		./configure 						\
			--host=arm-linux-gnueabihf --prefix=/usr 	\
			--with-sysroot=${ROOTFS}/debian-stretch-armhf
	make CFLAGS="--sysroot=${ROOTFS}"
	make DESTDIR=${ROOTFS} install

The --prefix refers only to the location in the target system, whose root
on the cross compilation host can be found at ${DESTDIR}.

gt
--

As gt is built with cmake, the cross compilation toolchain must be specified
as a text file, let's call it armhf-toolchain.txt:

.. code-block:: console

	set(CMAKE_SYSTEM_NAME Linux)
	set(CMAKE_SYSTEM_PROCESSOR arm)

	set(CMAKE_SYSROOT ${ROOTFS})

	set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)

	set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
	set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

Then the invocation of cmake follows:

.. code-block:: console

	PKG_CONFIG_PATH=${ROOTFS}/usr/lib/pkgconfig:			\
		${ROOTFS}/usr/lib/arm-linux-gnueabihf/pkgconfig 	\
		cmake 							\
			-DCMAKE_INSTALL_PREFIX=${ROOTFS} 		\
			-DCMAKE_RUNTIME_PREFIX=/ 			\
			-DCMAKE_TOOLCHAIN_FILE=armhf-toolchain.txt

And then:

.. code-block:: console

	make
	make install
