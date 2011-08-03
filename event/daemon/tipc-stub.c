
/* TIPC stub file, used when TIPC is not compiled in. */

int tipc_init(void)
{
    return -1;
}

void tipc_close(int fd)
{
    return;
}

void tipc_recv(int fd, short event, void *arg)
{
    return;
}

