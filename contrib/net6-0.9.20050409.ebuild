# Copyright 1999-2004 Gentoo Technologies, Inc.
# Distributed under the terms of the GNU General Public License v2
# $Header: /home/cvsroot/gentoo-x86/dev-util/darcs/darcs-0.9.17.ebuild,v 1.2 2004/03/18 08:27:31 kosmikus Exp $

DESCRIPTION="Network access framework for IPv4/IPv6 written in c++"
HOMEPAGE="http://darcs.0x539.de/net6"
LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86 ~ppc ~amd64"
IUSE=""
EDARCS_REPOSITORY="http://darcs.0x539.de/net6"
EDARCS_GET_CMD="get --verbose"

DEPEND=">=dev-libs/libsigc++-2.0"

RDEPEND=""

inherit darcs

src_compile() {
	sh ./autogen.sh
	econf || die "./configure failed"
	emake || die "make failed"
}

src_install() {
	make DESTDIR=${D} install || die
}
