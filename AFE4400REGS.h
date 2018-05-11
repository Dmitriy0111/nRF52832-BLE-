uint32_t AFE4400_REGS[49] = {
	/* Reg0: CONTROL0: Control Register 0 */
        0x000008, // �����: ��� 4
   	/* Reg1: LED2STC: Sample LED2 Start Count */ 
	0x007580, //(0x001770) ������ ������� LED2 �������� (6000)
        /* Reg2: LED2ENDC: Sample LED2 End Count */
	0x0076BE, //(001F3E) ����� ������� LED2 �������� (7998) 
        /* Reg3: LED2LEDSTC: LED2 LED Start Count */
	0x007530, //(001770) ������ LED2 �������� (6000)  
        /* Reg4: LED2LEDENDC: LED2 LED End Count */
	0x0076BF, //(001F3F) ����� LED2 ��������(7999)
	/* Reg5: ALED2STC: Sample Ambient LED2 Start Count */
	0x000050, //(000000) ������ ������� LED2 ��������� ��������  
	/* Reg6: ALED2ENDC: Sample Ambient LED2 End Count */
	0x00018E, //(0007CE) ����� ������� LED2 ��������� �������� (1998)
	/* Reg7: LED1STC: Sample LED1 Start Count */
	0x002760, //(0007D0) ������ ������� LED1 �������� (2000)
        /* Reg8: LED1ENDC: Sample LED1 End Count */ 
	0x00289E, //(000F9E) ����� ������� LED1 �������� (3998)
        /* Reg9: LED1LEDSTC: LED1 LED Start Count */
	0x002710, //(0007D0) ������ LED1 �������� (2000)
	/* Reg10: LED1LEDENDC: LED1 LED End Count */
	0x00289F, //(000F9F) ����� LED1 �������� (3999)
	/* Reg11: ALED1STC: Sample Ambient LED1 Start Count */
	0x004E70, //(000FA0) ������ ������� LED1 ��������� �������� (4000)
        /* Reg12: ALED1ENDC: Sample Ambient LED1 End Count */
	0x004FAE, //(00176E) ����� ������� LED1 ��������� �������� (5598)
	/* Reg13: LED2CONVST: LED2 Convert Start Count */ 
	0x000006, //(000002) ������ �������������� LED2 �������� 
	/* Reg14: LED2CONVEND: LED2 Convert End Count */
	0x00270F, //(0007CF) ����� �������������� LED2 �������� (1999)
	/* Reg15: ALED2CONVST: LED2 Ambient Convert Start Count */ 
	0x002716, //(0007D2) ������ �������������� LED2 ��������� �������� (2002)
	/* Reg16: ALED2CONVEND: LED2 Ambient Convert End Count */ 
	0x004E1F, //(000F9F) ����� �������������� LED2 ��������� �������� (3999)
	/* Reg17: LED1CONVST: LED1 Convert Start Count */ 
	0x004E26, //(000FA2) ������ �������������� LED1 �������� (4002) 
	/* Reg18: LED1CONVEND: LED1 Convert End Count */ 
	0x00752F, //(00176F) ����� �������������� LED1 �������� (5999) 
	/* Reg19: ALED1CONVST: LED1 Ambient Convert Start Count */ 
	0x007536, //(001772) ������ �������������� LED1 ��������� �������� (6002)
	/* Reg20: ALED1CONVEND: LED1 Ambient Convert End Count */
	0x009C3F, //(001F3F) ����� �������������� LED1 ��������� �������� (7999)
	/* Reg21: ADCRSTSTCT0: ADC Reset 0 Start Count */
	0x000000, //(000000) ������ ������� �������� ������ �������������� ���
	/* Reg22: ADCRSTENDCT0: ADC Reset 0 End Count */ 
	0x000005, //(000000) ����� ������� �������� ������ �������������� ���
	/* Reg23: ADCRSTSTCT1: ADC Reset 1 Start Count */ 
	0x002710, //(0007D0) ������ ������� �������� ������ �������������� ��� (2000)
	/* Reg24: ADCRSTENDCT1: ADC Reset 1 End Count */
	0x002715, //(0007D0) ����� ������� �������� ������ �������������� ��� (2000)
	/* Reg25: ADCRSTSTCT2: ADC Reset 2 Start Count */ 
	0x004E20, //(000FA0) ������ �������� �������� ������ �������������� ��� (4000)
	/* Reg26: ADCRSTENDCT2: ADC Reset 2 End Count */
	0x004E25, //(000FA0) ����� �������� �������� ������ �������������� ��� (4000)
	/* Reg27: ADCRSTSTCT3: ADC Reset 3 Start Count */
	0x007530, //(001770) ������ ��������� �������� ������ �������������� ��� (6000)
	/* Reg28: ADCRSTENDCT3: ADC Reset 3 End Count */
	0x007535, //(001770) ����� ��������� �������� ������ �������������� ��� (6000)
	/* Reg29: PRPCOUNT: Pulse Repetition Period Count */ 
	0x009C3F, //(001F3F) ����� ������� ���������� ��������� (7999)
	/* Reg30: CONTROL1: Control Register 1 */
	0x00101,  // ������ �������, LED2 sample and LED1 sample �� ������� PD_ALM � LED_ALM �����.
        /* Reg31: SPARE1: SPARE1 Register For Future Use */
	0x00000,
	/* Reg32: TIAGAIN: Transimpedance Amplifier Gain Setting */ 
	0x000000,
	/* Reg33: TIA_AMB_GAIN: Transimpedance Amplifier and Ambient Cancellation Stage Gain */
	0x0041FE,
	/* Reg34: LEDCNTRL: LED Control */ 
	0x014E4E, // ���. ���� ���. (50��), LED1 = LED2 = 5,88235 ��
	/* Reg35: CONTROL2: Control Register 2 */ 
	0x020100, // H-����, ������� Tx, Rx � AFE ��������
	/* Reg36: SPARE2: SPARE2 Register For Future Use */
	0x000000,
	/* Reg37: SPARE3: SPARE3 Register For Future Use */ 
	0x000000,
	/* Reg38: SPARE4: SPARE4 Register For Future Use */
	0x000000,
	/* Reg39: RESERVED1: RESERVED1 Register For Factory Use Only */
	0x000000,
	/* Reg40: RESERVED2: RESERVED2 Register For Factory Use Only */
	0x000000,
	/* Reg41: ALARM: Alarm Register */ 
	0x000000, // PD_ALM � LED_ALM ���. �����. ��� ��������������� ����� ������
	/* Reg42: LED2VAL: LED2 Digital Sample Value */
	0x000000,
	/* Reg43: ALED2VAL: Ambient LED2 Digital Sample Value */
	0x000000,
	/* Reg44: LED1VAL: LED1 Digital Sample Value */
	0x000000,
	/* Reg45: ALED1VAL: Ambient LED1 Digital Sample Value */
	0x000000,
	/* Reg46: LED2-ALED2VAL: LED2-Ambient LED2 Digital Sample Value */
	0x000000,
	/* Reg47: LED1-ALED1VAL: LED1-Ambient LED1 Digital Sample Value */
	0x000000,
	/* Reg48: DIAG: Diagnostics Flag */
	0x000000
};


