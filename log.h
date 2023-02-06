function void
error(const char* msg, ...)
{
    fprintf(stderr, "error: ");
    
    va_list valist;
    va_start(valist, msg);
    
    const char *msg_ptr = msg;
    while (*msg_ptr != 0)
    {
        fputc(*msg_ptr, stderr);
        msg_ptr++;
    }
    
    fputc('\n', stderr);
}