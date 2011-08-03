
/* SCTP stub file, used when SCTP is not compiled in. */

int sctp_init(void)
{
    return -1;
}

void sctp_close(int fd)
{
    return;
}

void sctp_recv(int fd, short event, void *arg)
{
    return;
}

