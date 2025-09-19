// FILE: auth.h
#pragma once

// authenticateUser prompts for credentials and checks them against the
// hardcoded administrator account. It returns true on success and prints
// status messages for both success and failure cases.
bool authenticateUser();
