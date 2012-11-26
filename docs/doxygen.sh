#! bash

# Parameters:
#  $1  The directory into which the doxoygen HTML will be copied.

# Creates public interface documentation for this module. This documentation is not the released documentation at this
# time. There is separate MS-Word doc files which are delivered as PDF files. However, this may be converged at some
# time in the future.

doxygen.exe nha_manual.dox
mv docs/manual/* "$1"
