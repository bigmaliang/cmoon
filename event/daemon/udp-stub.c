
/* UDP stub file, used when UDP is not compiled in. */

int udp_init(void)
{
    return -1;
}

void udp_close(int fd)
{
    return;
}

void udp_recv(int fd, short event, void *arg)
{
    return;
}

