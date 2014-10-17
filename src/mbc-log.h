// log.h; the header file which defines Log(); and LogErr();

#define LOGFILE "/var/log/mail.log" // all Log(); messages will be appended to this file

void Log (char *message); // logs a message to LOGFILE
void LogErr (char *message); // logs a message; execution is interrupted
