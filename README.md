# cfutil
Cloud File Utility (for Windows).

This is a small utility with some functionality that may be helpful during
development of a provider for the Windows Cloud File API, or perhaps some
maintenance tasks in general.

## A Warning
Certain operation of this tool may lead to data loss. Make sure you have
some kind of alternative backup if experimenting on files that could be
dear to you (e.g. OneDrive contents).

## Available commands

### `info` command

Displays some information about cloud files. Currently, the ranges of data
present on disk.

### `del` command
Removes a cloud file or directory. Doesn't use the Win32 API but invokes
`NtDeleteFile()` directly; this can help if you're receiving a "the cloud
file metadata is corrupt" error if you try to delete using Explorer or
command line.
