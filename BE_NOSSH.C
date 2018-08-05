#include <windows.h>         
#include <stdio.h>         
#include "putty.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/*
 * Linking module for PuTTYtel: list the available backends not
 * including ssh.
 */

struct backend_list backends[] = {
    {PROT_TELNET, "telnet", &telnet_backend},
    {PROT_RAW, "raw", &raw_backend},
    {0, NULL}
};

/*
 * Stub implementations of functions not used in non-ssh versions.
 */
void random_save_seed(void) {
}

void random_destroy_seed(void) {
}

void noise_ultralight(DWORD data) {
}

void noise_regular(void) {
}
