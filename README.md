# SysAsstLast

Version control program with functioning server and client that communicate through sockets. All files are compressed before sending to minimize size of file transfer. MD5 hash is used to check which files have been changed. Server stores all version history and can rollback to any previous version. 
The client can pull, commit, push, add, delete, request history, rollback, fetch, and even create new projects much like git.
