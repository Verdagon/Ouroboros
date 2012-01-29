/*
 * Protocol.cpp
 *
 *  Created on: Dec 16, 2011
 *      Author: Evan
 */

#include "Protocol.h"
#include <windows.h>
#include <string>
#include <TCHAR.h>

bool Networking::WinsockTCP::ProtocolImpl::winsockInitialized = false;

std::string Networking::WinsockTCP::GetLastErrorMessage(DWORD last_error, bool stripTrailingLineFeed) {
	TCHAR errmsg[512];

	if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, last_error, 0, errmsg, 511, NULL)) {
		/* if we fail, call ourself to find out why and return that error */
		return (GetLastErrorMessage(GetLastError(), stripTrailingLineFeed));
	}

	if (stripTrailingLineFeed) {
		const size_t length = _tcslen(errmsg);

		if (errmsg[length-1] == '\n') {
			errmsg[length-1] = 0;

			if (errmsg[length-2] == '\r')
				errmsg[length-2] = 0;
		}
	}

	return std::string(errmsg);
}
