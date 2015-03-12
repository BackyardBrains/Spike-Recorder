#ifndef BACKYARDBRAINS_BASSERRORS_H
#define BACKYARDBRAINS_BASSERRORS_H

/* libbass does not provide a function to convert
 * error codes to human readable messages.
 *
 * This function is a replacement for that. It gets the
 * error code and returns the explanation given in the
 * bass.h Header.
 */

namespace BackyardBrains {

const char *GetBassStrError();

}

#endif
