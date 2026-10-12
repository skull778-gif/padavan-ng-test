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

	// reset EPHY
	esw_ephy_reset();

	phy_val = sysRegRead(RALINK_REG_GPIOMODE2);
	phy_val &= 0xf003f003;
#if defined (CONFIG_RAETH_ESW_ONE_PORT)
	phy_val |= 0x05500550; // set P0 EPHY LED mode
#endif
	sysRegWrite(RALINK_REG_GPIOMODE2, phy_val);

	udelay(5000);

	mii_mgr_write(0, 31, 0x2000);		// change G2 page
	mii_mgr_write(0, 26, 0x0000);

	for(i=0; i<5; i++) {
		mii_mgr_write(i, 31, 0x8000);	// change L0 page
#if !defined (CONFIG_RAETH_ESW_CONTROL)
		/* disable EEE */
		mii_mgr_write_cl45(i, 0x07, 0x3c, 0x0000);
#endif
		mii_mgr_write(i, 30, 0xa000);
		mii_mgr_write(i, 31, 0xa000);	// change L2 page
		mii_mgr_write(i, 16, 0x0606);
		mii_mgr_write(i, 23, 0x0f0e);
		mii_mgr_write(i, 24, 0x1610);
		mii_mgr_write(i, 30, 0x1f15);
		mii_mgr_write(i, 28, 0x6111);

		mii_mgr_read(i, 4, &phy_val);
		phy_val |= (1 << 10);
		mii_mgr_write(i, 4, phy_val);
	}

	/* 100Base AOI setting */
	mii_mgr_write(0, 31, 0x5000);		// change G5 page
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

	/* Fix EPHY idle state abnormal behavior */
	mii_mgr_write(0, 31, 0x4000);		// change G4 page
	mii_mgr_write(0, 29, 0x000d);
	mii_mgr_write(0, 30, 0x0500);

	/* ============================================================ */
	/* LED Control — на основе Programming Guide MT7628 и DTS OpenWrt */
	/* ============================================================ */

	/* 1. Перевести пин P2_LED_AN (GPIO 41) в режим GPIO.
	 *    Биты 7:6 в GPIOMODE2 (0x64): 1 = GPIO.
	 *    Это то, что делает OpenWrt через pinctrl.
	 */
	phy_val = sysRegRead(0xB0000064);
	phy_val &= ~(0x3 << 6);   /* очистить биты 7:6 */
	phy_val |= (0x1 << 6);    /* установить режим GPIO (1) */
	sysRegWrite(0xB0000064, phy_val);

	/* 2. Настроить LEDC (0x168): LED_SEL = 0 (ESW LED Control).
	 *    Это заставляет свитч управлять светодиодами.
	 *    Биты 10:8 = 0.
	 *    LED_POLARITY (биты 4:0) = 0x1F (все порты, инверсия как в OpenWrt).
	 */
	sysRegWrite(RALINK_ETH_SW_BASE + 0x168, 0x1F | (0 << 8));

	/* 3. Маска портов LEDP_BASE (0xA4): 0x1E = порты 1,2,3,4 (LAN).
	 *    Это ровно то, что в OpenWrt: ucidef_set_led_switch "lan" ... "0x1e".
	 */
	sysRegWrite(RALINK_ETH_SW_BASE + REG_ESW_LEDP_BASE, 0x0000001E);
}

void mt7628_esw_eee_enable(int is_eee_enabled)
{
	u32 i;

	for (i = 0; i <= 4; i++) {
		/* EEE 100 LPI */
		mii_mgr_write_cl45(i, 0x07, 0x003c, (is_eee_enabled) ? 0x0002 : 0x0000);
	}
}
#endif
