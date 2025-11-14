/*----------------------------------------------------------
This Source Code Form is subject to the terms of the
Mozilla Public License, v.2.0. If a copy of the MPL
was not distributed with this file, You can obtain one
at http://mozilla.org/MPL/2.0/.
----------------------------------------------------------*/

#include <iostream>
#include <cstdlib>  // для realloc
#include <cstring>  // для memcpy (если потребуется)
#include "TStringList.h"






TStringList* ParseString(std::string Str) {

	return nullptr;
}

bool SameText(const std::string& str1, const std::string& str2) {
    if (str1.length() != str2.length()) {
        return false;
    }

    for (size_t i = 0; i < str1.length(); ++i) {
        if (std::tolower(str1[i]) != std::tolower(str2[i])) {
            return false;
        }
    }
    return true;
}


