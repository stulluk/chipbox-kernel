#include <linux/version.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/crc32.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include <asm/irq.h>
#include <asm/io.h>

#include "cn100_default.h"
#include "cn100.h"
#include "cn100_dbg.h"
#include "cn100_mem.h"
#include "cn100_hw.h"
#include "cn100_time.h"
#include "cn100_hash.h"
#include "cn100_phy.h"
#include "cn100_int.h"

#define DRV_NAME        "cn100"
#define DRV_VERSION     "1.4"

/*
 * Transmit timeout, default 5 seconds.
 */
static int watchdog 				= 5000;
struct net_device *cn100_devs 			= NULL;
static u8 MacAddr[] 				= {00, 0x1a, 0xcc, 0x00, 0x00, 0x00};

/* function declaration ------------------------------------- */
static int 				cn100_probe			(void);
static int 				cn100_probe_help	(struct net_device *dev);
static int 				cn100_open			(struct net_device *);
static int 				cn100_close			(struct net_device *);
static struct 			net_device_stats *cn100_get_stats(struct net_device *);
/* --------------------------------------------------------- */

static int netdev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
        struct board_info *np = netdev_priv(dev);
        int rc;

        if (!netif_running(dev))
                return -EINVAL;

        spin_lock_irq(&np->lock);
        rc = generic_mii_ioctl(&np->mii, if_mii(rq), cmd, NULL);
        spin_unlock_irq(&np->lock);

        return rc;
}

static void cn100_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
        strcpy(info->driver, DRV_NAME);
        strcpy(info->version, DRV_VERSION);
        strcpy(info->bus_info, "platform");
        info->regdump_len = 0x1000;
}

static int cn100_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
        struct board_info *np = netdev_priv(dev);
        spin_lock_irq(&np->lock);
        mii_ethtool_gset(&np->mii, cmd);
        spin_unlock_irq(&np->lock);
        return 0;
}

static int cn100_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
        struct board_info *np = netdev_priv(dev);
        int rc;
        spin_lock_irq(&np->lock);
        rc = mii_ethtool_sset(&np->mii, cmd);
        spin_unlock_irq(&np->lock);
        return rc;
}

static int cn100_nway_reset(struct net_device *dev)
{
        struct board_info *np = netdev_priv(dev);
        return mii_nway_restart(&np->mii);
}

static u32 cn100_get_msglevel(struct net_device *dev)
{
        struct board_info *np = netdev_priv(dev);
        return np->msg_enable;
}

static void cn100_set_msglevel(struct net_device *dev, u32 datum)
{
        struct board_info *np = netdev_priv(dev);
        np->msg_enable = datum;
}

static struct ethtool_ops cn100_ethtool_ops = {
        .get_drvinfo            = cn100_get_drvinfo,
        .get_settings           = cn100_get_settings,
        .set_settings           = cn100_set_settings,
        .nway_reset             = cn100_nway_reset,
        .get_link               = ethtool_op_get_link,
	.get_msglevel		= cn100_get_msglevel,
	.set_msglevel		= cn100_set_msglevel,
};

static void
cn100_release_board ( struct net_device *dev )
{
	struct board_info *db = NULL;
	PRINTK3 ( "cn100_release_board be call\n");
	if ( NULL == dev )
	{
		return;
	}
	if ( dev->irq )
	{
		free_irq(dev->irq, dev);
		dev->irq = 0;
	}
	db = (struct board_info *) dev->priv;	
	if ( NULL != db )
	{
		release_eth_mem ( db );
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	kfree(dev);
#else
	free_netdev(dev);
#endif
	dev = NULL;
	
	return;
}

static int 				
cn100_probe_help ( struct net_device *ndev )
{
	struct board_info *db = NULL;
	int ret = 0;

	PRINTK3 ( "cn100_probe_help ...\n");
	
	/* setup board info structure */
	memset(ndev->priv, 0, sizeof(struct board_info));
	db = (board_info_t *)ndev->priv;
	cn100_devs = ndev;
	
	spin_lock_init(&db->lock);
	spin_lock_init(&db->rx_lock);
	
	if ( 0 != ( ret = init_eth_mem ( db ) ) )
	{
		return ret;
	}
	/* fill in parameters for net-dev structure */
	ndev->open		 	= &cn100_open;
	ndev->hard_start_xmit    	= &cn100_start_xmit;
	ndev->poll			= &cn100_poll;
	ndev->weight			= 64;
	ndev->tx_timeout         	= &cn100_timeout;
	ndev->watchdog_timeo 		= msecs_to_jiffies(watchdog);
	ndev->stop		 	= &cn100_close;
	ndev->get_stats		 	= &cn100_get_stats;
	ndev->set_multicast_list 	= &cn100_hash_table;
	ndev->do_ioctl			= &netdev_ioctl;
	ndev->ethtool_ops		= &cn100_ethtool_ops;
	ndev->irq			= CN100_IRQ;
	ndev->base_addr 		= (unsigned long)(db->io_addr);	
#ifdef _CN100_MII_
	db->msg_enable       		= NETIF_MSG_LINK;
	db->mii.phy_id_mask  		= 0x1f;
	db->mii.reg_num_mask 		= 0x1f;
	db->mii.force_media  		= 0;
	db->mii.full_duplex  		= 1;
	db->mii.dev	     		= ndev;
	db->mii.mdio_read    		= &cn100_phy_read;
	db->mii.mdio_write   		= &cn100_phy_write;
	set_hw_clock(db);

#endif	
    set_mac_addr ( ndev, MacAddr );
	return 0;
}

static int
cn100_probe ( void )
{
	struct net_device *ndev = NULL;
	struct board_info *db = NULL;
	int ret = 0;

	PRINTK3 ("cn100_probe()\n");
	/* Alloc network device object and init it */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	ndev = init_etherdev(NULL, sizeof(struct board_info));
	ether_setup(ndev);
#else
	ndev = alloc_etherdev(sizeof(struct board_info));
#endif
	if ( NULL == ndev )
	{
		printk ( "%s: could not allocate device.\n", CARDNAME);
		return -ENOMEM;
	}

	/* set callback function and init board eth resource */
	ret = cn100_probe_help ( ndev );
	if ( 0 != ret ) 
	{
		goto out_release;
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0)
	ret = register_netdev(ndev);
	if ( 0 == ret ) 
	{
		db = netdev_priv(ndev);
		printk(KERN_INFO "ORION %s: 0x%08x IRQ %d MAC:%02x:%02x:%02x:%02x:%02x:%02x\n",
		                 ndev->name, CN100_BASEADDRESS, (u32)(ndev->irq),
				 ndev->dev_addr[0], ndev->dev_addr[1], ndev->dev_addr[2],
				 ndev->dev_addr[3], ndev->dev_addr[4], ndev->dev_addr[5]);
	}
	else
	{
		goto out_release;
	}
#endif

	return 0;

out_release:
	printk ("%s: not found (%d).\n", CARDNAME, ret);
	cn100_release_board ( ndev );
	return ret;
}

/*
 *  Open the interface.
 *  The interface is opened whenever "ifconfig" actives it.
 */
static int
cn100_open(struct net_device *dev)
{
	board_info_t *db = netdev_priv(dev);
	s32 ret = 0;
	int i;

	PRINTK3 ("entering cn100_open\n");

	if ( 0 != ( ret = alloc_eth_mem ( db ) ) )
	{
		printk ( "alloc eth memory resource fail\n" );
		goto ERROR_ALLOC_ETH_MEM;
	}
	
	/* add intterrupt request */
#ifdef _RESERVED_MEM_
	if ( request_irq ( dev->irq, &cn100_interrupt, 0, dev->name, dev ) ) 
	{
		printk ( "request_irq fail\n" );
		goto ERROR_REQUEST_IRQ;
	}
#endif

	/* init eth hard register */
	if ( 0 != ( ret = init_eth_hw ( dev, db ) ) )
	{
		printk ( "init eth hw resource fail\n");
		goto ERROR_INIT_ETH_HW;
	}

	/* init phy hard register */
	if ( 0 != ( ret = init_phy_hw ( dev, db ) ) )
	{
		printk ( "init phy resource fail\n");
		goto ERROR_INIT_PHY_HW;
	}
#ifdef _CN100_MII_	
	for(i = 0; i < 32; i++)	{	// Search External PHY
		ret = db->mii.mdio_read(dev, i, 1);
                if (ret != 0xffff && ret != 0x0000) {
			db->mii.phy_id = i;
                        printk(KERN_INFO "ORION %s: MII transceiver %d status 0x%4.4x "
                                           "advertising %4.4x.\n",
                                           dev->name, i, ret, db->mii.mdio_read(dev, i, 4));
                }
	}

	mii_check_media (&db->mii, netif_msg_link(db), 1);
	if(db->mii.full_duplex)
		write_reg32(db->io_addr, ETH_MAC_CFG1, read_reg32(db->io_addr, ETH_MAC_CFG1) |  0x1);
	else
		write_reg32(db->io_addr, ETH_MAC_CFG1, read_reg32(db->io_addr, ETH_MAC_CFG1) & ~0x1);

	if ( 0 != ( ret = init_eth_timer ( dev ) ) )
	{
		goto ERROR_INIT_ETH_TIME;
	}
#endif
	netif_start_queue(dev);
	return 0;

#ifdef _CN100_MII_
ERROR_INIT_ETH_TIME:
	release_phy_hw ( dev, db );
#endif
ERROR_INIT_PHY_HW:
	release_eth_hw ( dev, db );
ERROR_INIT_ETH_HW:
	free_irq(dev->irq, dev);
ERROR_REQUEST_IRQ:
	release_eth_mem ( db );
ERROR_ALLOC_ETH_MEM:
	return -EAGAIN;
}

/*
 * Stop the interface.
 * The interface is stopped when it is brought.
 */
static int
cn100_close(struct net_device *ndev)
{
	board_info_t *db = (board_info_t *) ndev->priv;
	PRINTK3 ( "entering %s\n",__FUNCTION__ );
#ifdef _CN100_MII_
	release_eth_timer	( db );
#endif	
	netif_stop_queue 	( ndev );
	release_phy_hw		( ndev, db );
	release_eth_hw		( ndev, db );
	release_eth_mem 	( db );
	free_irq(ndev->irq, ndev);

	return 0;
}

/*
 *  Get statistics from driver.
 */
static struct net_device_stats *
cn100_get_stats(struct net_device *dev)
{
	board_info_t *db = (board_info_t *) dev->priv;
	return &db->stats;
}

static int __init
cn100_init(void)
{
	PRINTK3 ("%s Ethernet Driver\n", CARDNAME);

	return cn100_probe ( );
}

static void __exit
cn100_cleanup(void)
{
	board_info_t *db = NULL;
	PRINTK3 ( "cn100: cn100_cleanup()\n" );

	if ( NULL != cn100_devs )
	{
		db = (board_info_t *) cn100_devs->priv;
		unregister_netdev ( cn100_devs );
		uninit_eth_mem ( db );
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
		kfree( cn100_devs );
#else
		free_netdev( cn100_devs );
#endif
		cn100_devs = NULL;
	}
}

/**
 *	mac_setup - process command line options
 *	@options: string of options
 *
 *	Process command line options for MAC address.
 *
 *	NOTE: This function is a __setup and __init function.
 *            It only stores the options. 
 *
 *	Returns zero.
 *
 */
static int __init mac_setup(char *options)
{
	int i=0;
    if ((options != NULL) && (*options != '\0')) {
        while((i<6) ) {

            MacAddr[i]=(u8)simple_strtoul(options,&options,16);
            i++;
            options=strchr(options,':');
            options++;
        }
    }
    else{ 	/* set delault Mac address */
        printk("Mac Address seems is wrong, set the mac addr to default\n");
        MacAddr[0]=0x00;
        MacAddr[1]=0x1a;
        MacAddr[2]=0xcc;
        MacAddr[3]=0x00;
        MacAddr[4]=0x00;
        MacAddr[5]=0x00;
    }
    printk("ORION Readed MAC:%02x:%02x:%02x:%02x:%02x:%02x\n",
           MacAddr[0], MacAddr[1], MacAddr[2],
           MacAddr[3], MacAddr[4], MacAddr[5]);
	return 0;
}
__setup("ethaddr=", mac_setup);




module_init(cn100_init);
module_exit(cn100_cleanup);

MODULE_AUTHOR("Celestialsemi");
MODULE_DESCRIPTION("Celestialsemi CN100 network driver");
MODULE_LICENSE("GPL");

