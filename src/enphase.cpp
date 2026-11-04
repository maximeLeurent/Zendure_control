#include "enphase.h"

String enphaseToken;
int64_t enphaseTokenDeadline = 0;
unsigned long lastTokenAttemptMs = 0;
bool tokenRequestAttempted = false;
