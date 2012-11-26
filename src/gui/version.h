/*
 * version.h
 *
 *  Created on: 24/09/2009
 *      Author: ttaha
 */
#ifndef VERSION_H
#define VERSION_H

#define PIT_VERSION_MAJOR 4
#define PIT_VERSION_MINOR 0
#define PIT_VERSION_RELEASE 0

#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)


#define PIT_VERSION STRINGIFY(PIT_VERSION_MAJOR) \
    "." STRINGIFY(PIT_VERSION_MINOR) \
    "." STRINGIFY(PIT_VERSION_RELEASE)

const char * const APP_VERSION_LONG      = PIT_VERSION;
const char * const AUTHOR                = "ASG - Silvebrook Research";
const char * const YEAR                  = "2011";
const char * const APPLICATION_NAME      = "PrintHead Inspection Tools";

#undef STRINGIFY
#undef STRINGIFY_INTERNAL

#endif // VERSION_H
