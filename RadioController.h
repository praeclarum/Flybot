#pragma once

#include "ConfigValue.h"

void rcBegin();
void rcUpdate();

bool rcDidReceiveData();
float rcGetInitialThrottle();

bool rcIsArming();
bool rcIsNoInput();
