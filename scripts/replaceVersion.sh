#! /usr/bin/env bash

## Command line parameters
if [[ $# != 2 ]]; then
    cat <<USAGE
usage:
  $0 <old> <new>
example:
  $0 '1.0.0' '1.0.1'
USAGE
    exit 1
fi


## Process and show version
OLD=`sed 's/\./\\\\./g' <<<"$1"`
NEW=`sed 's/\./\\\\./g' <<<"$2"`

OLD_MAJOR=`sed 's/^\([0-9]*\)\.[0-9]*\.[0-9]*/\1/' <<<"$1"`
NEW_MAJOR=`sed 's/^\([0-9]*\)\.[0-9]*\.[0-9]*/\1/' <<<"$2"`

OLD_MINOR=`sed 's/^[0-9]*\.\([0-9]*\)\.[0-9]*/\1/' <<<"$1"`
NEW_MINOR=`sed 's/^[0-9]*\.\([0-9]*\)\.[0-9]*/\1/' <<<"$2"`

OLD_RELEASE=`sed 's/^[0-9]*\.[0-9]*\.\([0-9]*\)/\1/' <<<"$1"`
NEW_RELEASE=`sed 's/^[0-9]*\.[0-9]*\.\([0-9]*\)/\1/' <<<"$2"`

OLD_DOT_THREE="${OLD_MAJOR}\\.${OLD_MINOR}\\.${OLD_RELEASE}"
NEW_DOT_THREE="${NEW_MAJOR}\\.${NEW_MINOR}\\.${NEW_RELEASE}"

echo "#==============================================="
echo "# Plain    '${OLD}'     -> '${NEW}'"
echo "#-----------------------------------------------"
echo "# Major    '${OLD_MAJOR}'           -> '${NEW_MAJOR}'"
echo "# Minor    '${OLD_MINOR}'           -> '${NEW_MINOR}'"
echo "# Release  '${OLD_RELEASE}'		-> '${NEW_RELEASE}'"
echo "#==============================================="
echo


## Make script safe to call from anywhere by going to the right location first
SCRIPT_DIR=`dirname "${PWD}/$0"`
echo "Entering directory \`${SCRIPT_DIR}'"
pushd "${SCRIPT_DIR}" &>/dev/null || exit 1

## Patch version.h
TMPFILE=`mktemp versionPatch.XXXXXX`
VERSION_H="${SCRIPT_DIR}/src/gui/version.h"
echo "Patching \`${VERSION_H}'"
sed \
        -e 's/^\(#define NHA_VERSION_MAJOR \)'"${OLD_MAJOR}"'/\1'"${NEW_MAJOR}"'/' \
        -e 's/^\(#define NHA_VERSION_MINOR \)'"${OLD_MINOR}"'/\1'"${NEW_MINOR}"'/' \
        -e 's/^\(#define NHA_VERSION_RELEASE \)'"${OLD_RELEASE}"'/\1'"${NEW_RELEASE}"'/' \
    "${VERSION_H}" > "${TMPFILE}"
mv -f "${TMPFILE}" "${VERSION_H}"

## Patch nha_manual.dox
TMPFILE=`mktemp versionPatch.XXXXXX`
NHADOX_H="${SCRIPT_DIR}/nha_manual.dox"
echo "Patching \`${NHADOX_H}'"
sed \
        -e 's/^\(PROJECT_NUMBER         = \)'"${OLD_DOT_THREE}"'/\1'"${NEW_DOT_THREE}"'/' \
    "${NHADOX_H}" > "${TMPFILE}"
mv -f "${TMPFILE}" "${NHADOX_H}"

## Patch nha_setup.iss
TMPFILE=`mktemp versionPatch.XXXXXX`
SETUP_H="${SCRIPT_DIR}/releases/nha_setup.iss"
echo "Patching \`${SETUP_H}'"
sed \
        -e 's/^\(#define Version "\)'"${OLD_DOT_THREE}"'/\1'"${NEW_DOT_THREE}"'/' \
    "${SETUP_H}" > "${TMPFILE}"
mv -f "${TMPFILE}" "${SETUP_H}"

## Go back to original $PWD
echo "Leaving directory \`${SCRIPT_DIR}'"
popd &>/dev/null || exit 1
exit 0
