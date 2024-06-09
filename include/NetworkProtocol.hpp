#pragma once

#include <SFML/Config.hpp>
#include <SFML/System/Vector2.hpp>

const unsigned short Server_Port = 7500;
const int REQ_QUEUE_LENGTH = 4; // request queue max length, usually 2-4
const int BUF_LENGTH =
    2048; // server buffer length MUST equals client buffer length
