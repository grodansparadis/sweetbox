/********************************************************************/
/*              Header for SPI module library functions             */
/********************************************************************/
#ifndef __SPI_H
#define __SPI_H

/* List of SFRs for SPI */
/* This list contains the SFRs with default (POR) values to be used for configuring SPI */
/* The user can modify this based on the requirement */
#define SPI1STAT_VALUE         0x0000
#define SPI2STAT_VALUE         0x0000
#define SPI1CON_VALUE          0x0000
#define SPI2CON_VALUE          0x0000
#define SPI1BUF_VALUE          0x0000
#define SPI2BUF_VALUE          0x0000

/* SPIxCON REGISTER */

#define  FRAME_ENABLE_ON        0xffff  /* Frame SPI support enable        */
#define  FRAME_ENABLE_OFF       0xbfff  /* Frame SPI support Disable       */

#define  FRAME_SYNC_INPUT       0xffff  /* Frame sync pulse Input (slave)  */
#define  FRAME_SYNC_OUTPUT      0xdfff  /* Frame sync pulse Output (master)*/

#define  DISABLE_SDO_PIN        0xffff  /* SDO pin is not used by module   */
#define  ENABLE_SDO_PIN         0xf7ff  /* SDO pin is  used by module      */

#define  SPI_MODE16_ON          0xffff  /* Communication is word wide      */
#define  SPI_MODE16_OFF         0xfbff  /* Communication is byte wide      */

#define  SPI_SMP_ON             0xffff  /* Input data sampled at end of data output time */
#define  SPI_SMP_OFF            0xfdff  /* Input data sampled at middle of data output time */

#define  SPI_CKE_ON             0xffff  /* Transmit happens from active clock 
                                           state to idle clock state*/
#define  SPI_CKE_OFF            0xfeff  /* Transmit happens on transition from
                                           idle clock state to active clock state */

#define  SLAVE_ENABLE_ON        0xffff  /* Slave Select enbale               */
#define  SLAVE_ENABLE_OFF       0xff7f  /* Slave Select not used by module   */

#define  CLK_POL_ACTIVE_LOW     0xffff  /* Idle state for clock is high, active is low */
#define  CLK_POL_ACTIVE_HIGH    0xffbf  /* Idle state for clock is low, active is high */

#define  MASTER_ENABLE_ON       0xffff  /* Master Mode              */
#define  MASTER_ENABLE_OFF      0xffdf  /* Slave Mode               */

#define  SEC_PRESCAL_1_1        0xffff  /* Secondary Prescale 1:1   */
#define  SEC_PRESCAL_2_1        0xfffb  /* Secondary Prescale 2:1   */
#define  SEC_PRESCAL_3_1        0xfff7  /* Secondary Prescale 3:1   */
#define  SEC_PRESCAL_4_1        0xfff3  /* Secondary Prescale 4:1   */
#define  SEC_PRESCAL_5_1        0xffef  /* Secondary Prescale 5:1   */
#define  SEC_PRESCAL_6_1        0xffeb  /* Secondary Prescale 6:1   */
#define  SEC_PRESCAL_7_1        0xffe7  /* Secondary Prescale 7:1   */
#define  SEC_PRESCAL_8_1        0xffe3  /* Secondary Prescale 8:1   */

#define  PRI_PRESCAL_1_1        0xffff  /* Primary Prescale 1:1     */
#define  PRI_PRESCAL_4_1        0xfffe  /* Primary Prescale 4:1     */
#define  PRI_PRESCAL_16_1       0xfffd  /* Primary Prescale 16:1    */
#define  PRI_PRESCAL_64_1       0xfffc  /* Primary Prescale 64:1    */


/* SPIxSTAT REGISTER */

#define  SPI_ENABLE             0xffff  /* Enable module */
#define  SPI_DISABLE            0x7fff  /* Disable module */

#define  SPI_IDLE_CON           0xdfff  /* Continue module operation in idle mode */
#define  SPI_IDLE_STOP          0xffff  /* Discontinue module operation in idle mode */ 

#define  SPI_RX_OVFLOW_CLR     0xffbf   /* Clear receive overflow bit.*/
 
/* SPI Interrupt defines */

#define  SPI_INT_EN             0xffff  /* SPI Interrupt Enable     */
#define  SPI_INT_DIS            0xfff7  /* SPI Interrupt Disable    */

#define  SPI_INT_PRI_0          0xfff8  /* SPI Interrupt Prior Level_0 */
#define  SPI_INT_PRI_1          0xfff9  /* SPI Interrupt Prior Level_1 */
#define  SPI_INT_PRI_2          0xfffa  /* SPI Interrupt Prior Level_2 */
#define  SPI_INT_PRI_3          0xfffb  /* SPI Interrupt Prior Level_3 */
#define  SPI_INT_PRI_4          0xfffc  /* SPI Interrupt Prior Level_4 */
#define  SPI_INT_PRI_5          0xfffd  /* SPI Interrupt Prior Level_5 */
#define  SPI_INT_PRI_6          0xfffe  /* SPI Interrupt Prior Level_6 */
#define  SPI_INT_PRI_7          0xffff  /* SPI Interrupt Prior Level_7 */


/* Macros to  Enable/Disable interrupts and set Interrupt priority of SPI1 */
#define EnableIntSPI1                    asm("BSET IEC0,#8")
#define DisableIntSPI1                   asm("BCLR IEC0,#8")
#define SetPriorityIntSPI1(priority)     (IPC2bits.SPI1IP = priority)

/* FUNCTION PROTOTYPES */

/* CloseSPI. Disables SPI module */
void  CloseSPI1() __attribute__ ((section (".libperi")));

/* ConfigINtSPI1. Configure Interrupt enable and priorities */
void ConfigIntSPI1(unsigned int config)  __attribute__ ((section
(".libperi")));


/* DataRdySPI */
 
char DataRdySPI1() __attribute__ ((section (".libperi"))); 

/* getcSPI. Read byte from SPIBUF register */
#define  getcSPI1    ReadSPI1


/* getsSPI.Write string to SPIBUF */
unsigned int getsSPI1(unsigned int length, unsigned int *rdptr, unsigned int spi_data_wait) 
__attribute__ ((section (".libperi")));


/* OpenSPI */
void OpenSPI1(unsigned int config1,unsigned int config2 ) __attribute__ ((section (".libperi")));

/* putcSPI.Write byte/word to SPIBUF register */
#define  putcSPI1    WriteSPI1


/* putsSPI Read string from SPIBUF */
void putsSPI1(unsigned int length, unsigned int *wrptr) 
__attribute__ ((section (".libperi")));


/* ReadSPI.Read byte/word from SPIBUF register */
unsigned int ReadSPI1() __attribute__ ((section (".libperi")));


/* WriteSPI. Write byte/word to SPIBUF register */
void WriteSPI1(unsigned int data_out) __attribute__ ((section (".libperi")));



/* The following devices support SPI2 */
#if defined(__dsPIC30F6010A2__) || defined(__dsPIC30F5011A2__)  || defined(__dsPIC30F6011A2__) || \
    defined(__dsPIC30F6012A2__) || defined(__dsPIC30F5013A2__) ||  defined(__dsPIC30F6013A2__) || \
    defined(__dsPIC30F6014A2__) || \
    defined(__dsPIC30F6010__) || defined(__dsPIC30F5011__)  || defined(__dsPIC30F6011__) || \
    defined(__dsPIC30F6012__) || defined(__dsPIC30F5013__) ||  defined(__dsPIC30F6013__) || \
    defined(__dsPIC30F6014__) || defined(__dsPIC30F5015__) 

/* Macros to  Enable/Disable interrupts and set Interrupt priority of SPI2 */
#define EnableIntSPI2                    asm("BSET IEC1,#10")
#define DisableIntSPI2                   asm("BCLR IEC1,#10")
#define SetPriorityIntSPI2(priority)     (IPC6bits.SPI2IP = priority)

/* CloseSPI2.Disables SPI module */
void  CloseSPI2()  __attribute__ ((section (".libperi")));

/* ConfigINtSPI2. Configures Interrupt enable and priorities */
void ConfigIntSPI2(unsigned int config)  __attribute__ ((section
(".libperi")));

/* OpenSPI */
void OpenSPI2(unsigned int config1,unsigned int config2 )  __attribute__ ((section
(".libperi")));

/* DataRdySPI. Test if SPIBUF register is full */
char DataRdySPI2()  __attribute__ ((section (".libperi"))); 

/* getcSPI.Read byte from SPIBUF register */
#define  getcSPI2    ReadSPI2

/* getsSPI.Write string to SPIBUF */
unsigned int getsSPI2(unsigned int length, unsigned int *rdptr, unsigned int spi_data_wait) 
 __attribute__ ((section(".libperi")));

/* putcSPI.Write byte/word to SPIBUF register */
#define  putcSPI2    WriteSPI2

/* putsSPI. Read string from SPIBUF */
void putsSPI2(unsigned int length, unsigned int *wrptr) 
__attribute__ ((section(".libperi")));


/* ReadSPI.Read byte/word from SPIBUF register */
unsigned int ReadSPI2() __attribute__ ((section (".libperi")));


/* WriteSPI. Write byte/word to SPIBUF register */
void WriteSPI2( unsigned int data_out) __attribute__ ((section
(".libperi")));



#endif 
#endif  /* __SPI_H */
