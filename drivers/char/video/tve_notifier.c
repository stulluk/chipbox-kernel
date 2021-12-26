#include <linux/config.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <linux/proc_fs.h>

#include <linux/fb.h>

static struct sock *netlink_tve_sock;

static char buffer[64];
static int buffer_tail = 0;

static int tve_event_notify(struct notifier_block *self, unsigned long action, void *data)
{
        struct sk_buff * skb = alloc_skb(1024, GFP_ATOMIC);

	if (FB_EVENT_MODE_CHANGE == action) 
	{
		NETLINK_CB(skb).pid = 0;
		NETLINK_CB(skb).dst_pid = 0;
		netlink_broadcast(netlink_tve_sock, skb, 0, 1, GFP_KERNEL);
	}

	return 0;
}

static struct notifier_block tve_event_notifier = {
	.notifier_call	= tve_event_notify,
};

// DELME static void recv_handler(struct sock * sk, int length)
// DELME {
// DELME         wake_up_interruptible(sk->sk_sleep);
// DELME }

static int netlink_tve_readproc(char *page, char **start, off_t off,
                          int count, int *eof, void *data)
{
        int len;

        if (off >= buffer_tail) 
	{
                * eof = 1;
                return 0;
        }
        else {
                len = count;
                if (count > PAGE_SIZE) {
                        len = PAGE_SIZE;
                }
                if (len > buffer_tail - off) {
                        len = buffer_tail - off;
                }
                memcpy(page, buffer + off, len);
                *start = page;

                return len;
        }
}

static int tve_notifier_init(void)
{
        netlink_tve_sock = netlink_kernel_create(NETLINK_TVE_CHANGE, NULL /* no recv handler */); 

        if (NULL == netlink_tve_sock) return -1;

	fb_register_client(&tve_event_notifier);
        create_proc_read_entry("tve_status", 0444, NULL, netlink_tve_readproc, 0);
 
        return 0;
}

static void tve_notifier_exit(void)
{
	fb_unregister_client(&tve_event_notifier);

	wake_up(netlink_tve_sock->sk_sleep);
	sock_release(netlink_tve_sock->sk_socket);
}

