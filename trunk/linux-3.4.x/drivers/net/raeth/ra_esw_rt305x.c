#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/string.h>

#include "ra_eth_reg.h"
#include "ra_esw_reg.h"
#include "ra_rfrw.h"
#include "mii_mgr.h"
#include "ra_eth.h"
#include "ra_phy.h"

#if defined (CONFIG_RAETH_ESW)
int rt305x_esw_wait_wt_mac(void)
{
	int i;
	u32 value;

	for (i = 0; i < 200; i++) {
		udelay(100);
		value = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_WT_MAC_AD0);
		if (value & 0x2)
			return 0;
	}

	return -1;
}

void rt305x_esw_mac_table_clear(int static_only)
{
	int i, j;
	u32 value, mac_esw[2];

	sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_TABLE_SEARCH, 0x1);

	for (i = 0; i < 0x400; i++) {
		for (j = 0; j < 200; j++) {
			udelay(100);
			value = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_TABLE_STATUS0);
			if (value & 0x1) {
				if ((value & 0x70) == 0)
					goto next_entry;
				if (static_only) {
					if ((value & 0x70) != 0x70)
						goto next_entry;
				}
				mac_esw[0] = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_TABLE_STATUS2);
				mac_esw[1] = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_TABLE_STATUS1) & 0xffff;
				sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_WT_MAC_AD2, mac_esw[0]);
				sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_WT_MAC_AD1, mac_esw[1]);
				sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_WT_MAC_AD0, (value & 0x780) | 0x01);
				
				rt305x_esw_wait_wt_mac();
				
				if (value & 0x2) {
					return;
				}
				break;
			}
			else if (value & 0x2) {
				return;
			}
		}
next_entry:
		sysRegWrite(RALINK_ETH_SW_BASE+REG_ESW_TABLE_SEARCH, 0x2);
	}
}

static void esw_ephy_reset(void)
{
	u32 val = sysRegRead(REG_RSTCTRL);

	val |= RALINK_EPHY_RST;
	sysRegWrite(REG_RSTCTRL, val);
	udelay(10);

	val &= ~(RALINK_EPHY_RST);
	sysRegWrite(REG_RSTCTRL, val);
	udelay(1000);
}

#if defined (CONFIG_RT3052_ASIC)
static void rt3052_ephy_init(void)
{
	int i;
	u32 phy_val, phy_val2;

	rw_rf_reg(0, 0, &phy_val);
	phy_val = phy_val >> 4;

	if (phy_val > 0x5) {
		rw_rf_reg(0, 26, &phy_val);
		phy_val2 = (phy_val | (0x3 << 5));
		rw_rf_reg(1, 26, &phy_val2);
		
		esw_ephy_reset();
		
		rw_rf_reg(1, 26, &phy_val);
		
		mii_mgr_write(0, 31, 0x8000);
		for(i=0;i<5;i++){
			mii_mgr_write(i, 26, 0x1600);
			mii_mgr_write(i, 29, 0x7058);
			mii_mgr_write(i, 30, 0x0018);
		}
		
		mii_mgr_write(0, 31, 0x0);
		mii_mgr_write(0,  1, 0x4a40);
		mii_mgr_write(0,  2, 0x6254);
		mii_mgr_write(0,  3, 0xa17f);
#if defined (ENABLE_LDPS)
		mii_mgr_write(0, 12, 0x7eaa);
		mii_mgr_write(0, 22, 0x252f);
#else
		mii_mgr_write(0, 12, 0x0);
		mii_mgr_write(0, 22, 0x052f);
#endif
		mii_mgr_write(0, 14, 0x65);
		mii_mgr_write(0, 16, 0x0684);
		mii_mgr_write(0, 17, 0x0fe0);
		mii_mgr_write(0, 18, 0x40ba);
		mii_mgr_write(0, 27, 0x2fce);
		mii_mgr_write(0, 28, 0xc410);
		mii_mgr_write(0, 29, 0x598b);
		mii_mgr_write(0, 31, 0x8000);
		
		for(i=0;i<5;i++){
			mii_mgr_read(i, 26, &phy_val);
			phy_val |= 0x0001;
			mii_mgr_write(i, 26, phy_val);
		}
	} else {
		mii_mgr_write(0, 31, 0x8000);
		for(i=0;i<5;i++){
			mii_mgr_write(i, 26, 0x1600);
			mii_mgr_write(i, 29, 0x7058);
			mii_mgr_write(i, 30, 0x0018);
		}
		
		mii_mgr_write(0, 31, 0x0);
		mii_mgr_write(0,  1, 0x4a40);
		mii_mgr_write(0,  2, 0x6254);
		mii_mgr_write(0,  3, 0xa17f);
		mii_mgr_write(0, 14, 0x65);
		mii_mgr_write(0, 16, 0x0684);
		mii_mgr_write(0, 17, 0x0fe0);
		mii_mgr_write(0, 18, 0x40ba);
		mii_mgr_write(0, 22, 0x052f);
		mii_mgr_write(0, 27, 0x2fce);
		mii_mgr_write(0, 28, 0xc410);
		mii_mgr_write(0, 29, 0x598b);
		mii_mgr_write(0, 31, 0x8000);
		
		for(i=0;i<5;i++){
			mii_mgr_read(i, 26, &phy_val);
			phy_val |= 0x0001;
			mii_mgr_write(i, 26, phy_val);
		}
	}
}
#endif

#if defined (CONFIG_RT3352_ASIC)
static void rt3352_ephy_init(void)
{
	int i;
	u32 phy_val;

	esw_ephy_reset();

	mii_mgr_write(0, 31, 0x8000);
	for(i=0;i<5;i++){
		mii_mgr_write(i, 26, 0x1600);
		mii_mgr_write(i, 29, 0x7016);
		mii_mgr_write(i, 30, 0x0038);
	}

	mii_mgr_write(0, 31, 0x0);
	mii_mgr_write(0,  1, 0x4a40);
	mii_mgr_write(0,  2, 0x6254);
	mii_mgr_write(0,  3, 0xa17f);
	mii_mgr_write(0, 12, 0x7eaa);
	mii_mgr_write(0, 14, 0x65);
	mii_mgr_write(0, 16, 0x0684);
	mii_mgr_write(0, 17, 0x0fe0);
	mii_mgr_write(0, 18, 0x40ba);
	mii_mgr_write(0, 22, 0x253f);
	mii_mgr_write(0, 27, 0x2fda);
	mii_mgr_write(0, 28, 0xc410);
	mii_mgr_write(0, 29, 0x598b);
	mii_mgr_write(0, 31, 0x8000);

	for(i=0;i<5;i++){
		mii_mgr_read(i, 26, &phy_val);
		phy_val |= 0x0001;
		mii_mgr_write(i, 26, phy_val);
	}
}
#endif

#if defined (CONFIG_RT5350_ASIC)
static void rt5350_ephy_init(void)
{
	int i;
	u32 phy_val;

	esw_ephy_reset();

	mii_mgr_write(0, 31, 0x8000);
	for(i=0;i<5;i++){
		mii_mgr_write(i, 26, 0x1600);
		mii_mgr_write(i, 29, 0x7015);
		mii_mgr_write(i, 30, 0x0038);
	}

	mii_mgr_write(0, 31, 0x0);
	mii_mgr_write(0,  1, 0x4a40);
	mii_mgr_write(0,  2, 0x6254);
	mii_mgr_write(0,  3, 0xa17f);
	mii_mgr_write(0, 12, 0x7eaa);
	mii_mgr_write(0, 14, 0x65);
	mii_mgr_write(0, 16, 0x0684);
	mii_mgr_write(0, 17, 0x0fe0);
	mii_mgr_write(0, 18, 0x40ba);
	mii_mgr_write(0, 22, 0x253f);
	mii_mgr_write(0, 27, 0x2fda);
	mii_mgr_write(0, 28, 0xc410);
	mii_mgr_write(0, 29, 0x598b);
	mii_mgr_write(0, 31, 0x8000);

	for(i=0;i<5;i++){
		mii_mgr_read(i, 26, &phy_val);
		phy_val |= 0x0001;
		mii_mgr_write(i, 26, phy_val);
	}
}
#endif

#if defined (CONFIG_MT7628_ASIC)
static void mt7628_ephy_init(void)
{
	int i;
	u32 phy_val;

	phy_val = sysRegRead(REG_AGPIOCFG);
	phy_val &= ~(MT7628_P0_EPHY_AIO_EN);
#if defined (CONFIG_RAETH_ESW_ONE_PORT)
	phy_val |=  (MT7628_P1_EPHY_AIO_EN | MT7628_P2_EPHY_AIO_EN | MT7628_P3_EPHY_AIO_EN | MT7628_P4_EPHY_AIO_EN);
#else
	phy_val &= ~(MT7628_P1_EPHY_AIO_EN | MT7628_P2_EPHY_AIO_EN | MT7628_P3_EPHY_AIO_EN | MT7628_P4_EPHY_AIO_EN);
#endif
	sysRegWrite(REG_AGPIOCFG, phy_val);

	esw_ephy_reset();

	phy_val = sysRegRead(RALINK_REG_GPIOMODE2);
	phy_val &= 0xf003f003;
#if defined (CONFIG_RAETH_ESW_ONE_PORT)
	phy_val |= 0x05500550;
#endif
	sysRegWrite(RALINK_REG_GPIOMODE2, phy_val);

	udelay(5000);

	mii_mgr_write(0, 31, 0x2000);
	mii_mgr_write(0, 26, 0x0000);

	for(i=0; i<5; i++) {
		mii_mgr_write(i, 31, 0x8000);
#if !defined (CONFIG_RAETH_ESW_CONTROL)
		mii_mgr_write_cl45(i, 0x07, 0x3c, 0x0000);
#endif
		mii_mgr_write(i, 30, 0xa000);
		mii_mgr_write(i, 31, 0xa000);
		mii_mgr_write(i, 16, 0x0606);
		mii_mgr_write(i, 23, 0x0f0e);
		mii_mgr_write(i, 24, 0x1610);
		mii_mgr_write(i, 30, 0x1f15);
		mii_mgr_write(i, 28, 0x6111);

		mii_mgr_read(i, 4, &phy_val);
		phy_val |= (1 << 10);
		mii_mgr_write(i, 4, phy_val);
	}

	mii_mgr_write(0, 31, 0x5000);
	mii_mgr_write(0, 19, 0x004a);
	mii_mgr_write(0, 20, 0x015a);
	mii_mgr_write(0, 21, 0x00ee);
	mii_mgr_write(0, 22, 0x0033);
	mii_mgr_write(0, 23, 0x020a);
	mii_mgr_write(0, 24, 0x0000);
	mii_mgr_write(0, 25, 0x024a);
	mii_mgr_write(0, 26, 0x035a);
	mii_mgr_write(0, 27, 0x02ee);
	mii_mgr_write(0, 28, 0x0233);
	mii_mgr_write(0, 29, 0x000a);
	mii_mgr_write(0, 30, 0x0000);

	mii_mgr_write(0, 31, 0x4000);
	mii_mgr_write(0, 29, 0x000d);
	mii_mgr_write(0, 30, 0x0500);

	/* ============================================================ */
	/* LED Control */
	/* ============================================================ */

	sysRegWrite(0xB0000064, 0x0);                              // EPHY LED mode
	sysRegWrite(RALINK_ETH_SW_BASE + 0x168, 0x1F | (2 << 8));  // LED_SEL = 2
	sysRegWrite(RALINK_ETH_SW_BASE + REG_ESW_LEDP_BASE, 0x0000001E);  // маска 0x1E
}

void mt7628_esw_eee_enable(int is_eee_enabled)
{
	u32 i;

	for (i = 0; i <= 4; i++) {
		mii_mgr_write_cl45(i, 0x07, 0x003c, (is_eee_enabled) ? 0x0002 : 0x0000);
	}
}
#endif
#endif /* CONFIG_RAETH_ESW */

void rt305x_esw_init(void)
{
#if defined (CONFIG_RT5350_ASIC)
	sysRegWrite(RALINK_ETH_SW_BASE+0x0168, 0x17);
#endif

	sysRegWrite(RALINK_ETH_SW_BASE+0x0008, 0xC8A07850);
	sysRegWrite(RALINK_ETH_SW_BASE+0x00E4, 0x0000003f);
	sysRegWrite(RALINK_ETH_SW_BASE+0x0014, 0x00405555);
	sysRegWrite(RALINK_ETH_SW_BASE+0x0050, 0x00002001);
	sysRegWrite(RALINK_ETH_SW_BASE+0x0090, 0x00007f7f);
	sysRegWrite(RALINK_ETH_SW_BASE+0x0098, 0x00007f3f);
	sysRegWrite(RALINK_ETH_SW_BASE+0x00CC, 0x0002500c);
	sysRegWrite(RALINK_ETH_SW_BASE+0x009C, 0x0008a301);

	sysRegWrite(RALINK_ETH_SW_BASE+0x008C, 0x02404040);
	sysRegWrite(RALINK_ETH_SW_BASE+0x00C8, 0x3f502b28);
	sysRegWrite(RALINK_ETH_SW_BASE+0x0084, 0x00000000);
	sysRegWrite(RALINK_ETH_SW_BASE+0x0110, 0x7d000000);

#if defined (CONFIG_P5_RGMII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(1 << 9);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3fff;
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0xf<<20);
#elif defined (CONFIG_P5_MII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(1 << 9);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0x3fff);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3ffd;
#if defined (CONFIG_RALINK_RT3352)
	ge1_set_mode(1, 0);
#endif
#elif defined (CONFIG_P5_MAC_TO_PHY_MODE)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(1 << 9);
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~RALINK_GPIOMODE_MDIO;
	ext_gphy_init(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR);
#if defined (CONFIG_RT3052_ASIC) || defined (CONFIG_RT3352_ASIC)
	enable_autopoll_phy(1);
#endif
#elif defined (CONFIG_P5_RMII_TO_MAC_MODE)
	*(volatile u32 *)(RALINK_REG_GPIOMODE) &= ~(1 << 9);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1<<29);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(0x3fff);
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) |= 0x3ffd;
#if defined (CONFIG_RALINK_RT3352)
	ge1_set_mode(2, 0);
#endif
#else

#if defined (CONFIG_RALINK_RT3052)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1 << 29);
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_MDIO;
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_RGMII;
	*(volatile u32 *)(0xb0000674) = 0xFFF;
	*(volatile u32 *)(0xb000067C) = 0x0;
#elif defined (CONFIG_RALINK_RT3352)
	*(volatile u32 *)(RALINK_ETH_SW_BASE+0x00C8) &= ~(1 << 29);
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_MDIO;
	*(volatile u32 *)(RALINK_REG_GPIOMODE) |= RALINK_GPIOMODE_GE1;
	*(volatile u32 *)(0xb0000624) = 0xC0000000;
	*(volatile u32 *)(0xb000062C) = 0xC0000000;
	*(volatile u32 *)(0xb000064C) = 0xFFF;
	*(volatile u32 *)(0xb0000654) = 0xFFF;
#elif defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_MT7628)
	/* do nothing */
#endif
#endif

#if defined (CONFIG_RAETH_ESW)
#if defined (CONFIG_RT3052_ASIC)
	rt3052_ephy_init();
#elif defined (CONFIG_RT3352_ASIC)
	rt3352_ephy_init();
#elif defined (CONFIG_RT5350_ASIC)
	rt5350_ephy_init();
#elif defined (CONFIG_MT7628_ASIC)
	mt7628_ephy_init();
#endif
#endif
}
