/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@laptop   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "testcard.h"

TestCard::TestCard(QString _name, int _id, int _coord_x, int _coord_y)
	: id(_id), coord_x(_coord_x), coord_y(_coord_y), name(_name), counters(0), tapped(false), attacking(false), facedown(false), annotation(QString()), doesntUntap(false)
{
}


TestCard::~TestCard()
{
}

void TestCard::resetState()
{
	setCoords(0, 0);
	setCounters(0);
	setTapped(false);
	setAttacking(false);
	setFaceDown(false);
	setAnnotation(QString());
	setDoesntUntap(false);
}

bool TestCard::setAttribute(const QString &aname, const QString &avalue, bool allCards)
{
	if (!aname.compare("counters")) {
		bool ok;
		int tmp_int = avalue.toInt(&ok);
		if (!ok)
			return false;
		setCounters(tmp_int);
	} else if (!aname.compare("tapped")) {
		bool value = !avalue.compare("1");
		if (!(!value && allCards && doesntUntap))
			setTapped(value);
	} else if (!aname.compare("attacking")) {
		setAttacking(!avalue.compare("1"));
	} else if (!aname.compare("facedown")) {
		setFaceDown(!avalue.compare("1"));
	} else if (!aname.compare("annotation")) {
		setAnnotation(avalue);
	} else if (!aname.compare("doesnt_untap")) {
		setDoesntUntap(!avalue.compare("1"));
	} else
		return false;
	
	return true;
}
