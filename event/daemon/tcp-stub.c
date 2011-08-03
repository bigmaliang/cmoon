
/* TCP stub file, used when TCP is not compiled in. */

int tcp_init(void)
{
    return -1;
}

void tcp_close(int fd)
{
    return;
}

void tcp_recv(int fd, short event, void *arg)
{
    return;
}

