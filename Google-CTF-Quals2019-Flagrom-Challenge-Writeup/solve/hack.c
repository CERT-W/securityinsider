__sfr __at(0xff) POWEROFF;
__sfr __at(0xfe) DEBUG;
__sfr __at(0xfd) CHAROUT;
__xdata __at(0xff00) unsigned char FLAG[0x100];

__sfr __at(0xfa) RAW_I2C_SCL;
__sfr __at(0xfb) RAW_I2C_SDA;

__xdata __at(0xfe00) unsigned char I2C_ADDR; 
__xdata __at(0xfe01) unsigned char I2C_LENGTH;  
__xdata __at(0xfe02) unsigned char I2C_RW_MASK;  
__xdata __at(0xfe03) unsigned char I2C_ERROR_CODE;  
__xdata __at(0xfe08) unsigned char I2C_DATA[8]; 
__sfr __at(0xfc) I2C_STATE;  

const unsigned char SEEPROM_I2C_ADDR_MEMORY = 0b10100000;
const unsigned char SEEPROM_I2C_ADDR_SECURE = 0b01010000;

#define SEEPROM_I2C_CTRL_READ   (SEEPROM_I2C_ADDR_MEMORY | 0b1)
#define SEEPROM_I2C_CTRL_WRIT   (SEEPROM_I2C_ADDR_MEMORY | 0b0)

void print(const char *str) {
  while (*str) {
    CHAROUT = *str++;
  }
}

#define I2CSPEED 1
void I2C_delay(void);
unsigned char read_SCL(void) {
  return RAW_I2C_SCL;
}
unsigned char read_SDA(void) {
  return RAW_I2C_SDA;
}
void set_SCL(void) {
  RAW_I2C_SCL = 1;
}
void clear_SCL(void) {
  RAW_I2C_SCL = 0;
}
void set_SDA(void) {
  RAW_I2C_SDA = 1;
}
void clear_SDA(void) {
  RAW_I2C_SDA = 0;
}
void arbitration_lost(void) {
}

unsigned char started = 0; 

void i2c_start_cond(void) {
  if (started) { 
    set_SDA();
    I2C_delay();
    set_SCL();
    while (read_SCL() == 0) {
    }
    I2C_delay();
  }

  if (read_SDA() == 0) {
    arbitration_lost();
  }

  clear_SDA();
  I2C_delay();
  clear_SCL();
  started = 1;
}

void i2c_stop_cond(void) {
  clear_SDA();
  I2C_delay();

  set_SCL();
  while (read_SCL() == 0) {
  }

  I2C_delay();

  set_SDA();

  if (read_SDA() == 0) {
    arbitration_lost();
  }

  started = 0;
}

void i2c_write_bit(unsigned char bit) {
  if (bit) {
    set_SDA();
  } else {
    clear_SDA();
  }

  I2C_delay();

  set_SCL();

  while (read_SCL() == 0) { 
  }

  if (bit && (read_SDA() == 0)) {
    arbitration_lost();
  }

  clear_SCL();
}

unsigned char i2c_read_bit(void) {
  unsigned char bit;

  set_SDA();

  I2C_delay();

  set_SCL();

  while (read_SCL() == 0) { 
  }

  I2C_delay();

  bit = read_SDA();

  clear_SCL();

  return bit;
}

unsigned char i2c_write_byte(unsigned char send_start,
                    unsigned char send_stop,
                    unsigned char byte) {
  unsigned bit;
  unsigned char     nack;

  if (send_start) {
/*    print("DEBUG i2c_write_byte send start\n");*/
    i2c_start_cond();
  }

  for (bit = 0; bit < 8; ++bit) {
/*    print("DEBUG i2c_write_byte send bit\n");*/
    i2c_write_bit((byte & 0x80) != 0);
    byte <<= 1;
  }

/*  print("DEBUG i2c_write_byte read NACK\n");*/
  nack = i2c_read_bit();
  if (nack) {
/*    print("DEBUG i2c_write_byte got NACK\n");*/
  } else {
/*    print("DEBUG i2c_write_byte got ACK\n");*/
  }

  if (send_stop) {
/*    print("DEBUG i2c_write_byte send stop\n");*/
    i2c_stop_cond();
  }

  return nack;
}

unsigned char i2c_read_byte(unsigned char send_stop) {
  unsigned char byte = 0;
  unsigned char bit;

  for (bit = 0; bit < 8; ++bit) {
    byte = (byte << 1) | i2c_read_bit();
  }

  if (i2c_read_bit()) {
/*    print("DEBUG i2c_read_byte got NACK\n");*/
  } else {
/*    print("DEBUG i2c_read_byte got ACK\n");*/
  }

  if (send_stop) {
    i2c_stop_cond();
  }

  return byte;
}

void I2C_delay(void) { 
  volatile int v;
  int i;

  for (i = 0; i < I2CSPEED / 2; ++i) {
    v=0;
  }
}


void main(void) {
  int i;
  print("start user program\n");
  /* Read at 0 */
  i2c_write_byte(1, 0, SEEPROM_I2C_CTRL_WRIT);
  i2c_write_byte(0, 0, 0);
  /*i2c_write_byte(1, 0, SEEPROM_I2C_CTRL_WRIT);
  i2c_write_byte(0, 0, 60);
  i2c_write_byte(0, 0, 'A');
  i2c_write_byte(1, 0, SEEPROM_I2C_CTRL_WRIT);
  i2c_write_byte(0, 0, 0);*/
  i2c_write_byte(1, 0, SEEPROM_I2C_ADDR_SECURE | 0b1111);
  i2c_write_byte(1, 0, SEEPROM_I2C_CTRL_READ);
  for (i=0; i<255; i++) {
	if (i%64 == 0) {
		print("\n");
	}
  	CHAROUT = i2c_read_byte(0);
  }
  print("\n");
  POWEROFF = 1;
}
