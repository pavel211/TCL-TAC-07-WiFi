/**
* Create by Miguel Ángel López on 20/07/19
* and modify by xaxexa
* and modify by Solovey 13.08.2023
* and finally made it by RU4PAA (https://github.com/pavel211/) 20.03.2024
**/

#define SET_TEMP_MASK 0b00001111

#define MODE_POS      7
#define MODE_MASK     0b00111111

#define MODE_AUTO     0b00110101
#define MODE_COOL     0b00110001
#define MODE_DRY      0b00110011
#define MODE_FAN_ONLY 0b00110010
#define MODE_HEAT     0b00110100

#define FAN_SPEED_POS 8
#define FAN_QUIET_POS 33

#define FAN_AUTO      0b10000000  //auto
#define FAN_QUIET     0x80    //silent
#define FAN_LOW       0b10010000  //  |
#define FAN_MIDDLE    0b11000000  //  ||
#define FAN_MEDIUM    0b10100000  //  |||
#define FAN_HIGH      0b11010000  //  ||||
#define FAN_FOCUS     0b10110000  //  |||||
#define FAN_DIFFUSE   0b10000000  //  POWER [7]
#define FAN_SPEED_MASK 0b11110000  //FAN SPEED MASK

#define SWING_POS        10
#define SWING_OFF        0b00000000
#define SWING_HORIZONTAL 0b00100000
#define SWING_VERTICAL   0b01000000
#define SWING_BOTH       0b01100000
#define SWING_MODE_MASK  0b01100000

// +++++++++++++++++++++++++++++++++++

class TCL {

private:

  byte poll[8] = {0xBB,0x00,0x01,0x04,0x02,0x01,0x00,0xBD};

  byte checksum;
  byte dataTX[38];
  byte dataRX[61];

public:

  enum ClimateMode : uint8_t {
       CLIMATE_MODE_OFF = 0, CLIMATE_MODE_AUTO = 1, CLIMATE_MODE_COOL = 2, CLIMATE_MODE_HEAT = 3, CLIMATE_MODE_FAN_ONLY = 4, CLIMATE_MODE_DRY = 5  };
  enum ClimateFanMode : uint8_t {
       CLIMATE_FAN_OFF = 1, CLIMATE_FAN_AUTO = 2, CLIMATE_FAN_LOW = 3, CLIMATE_FAN_MEDIUM = 4, CLIMATE_FAN_HIGH = 5, CLIMATE_FAN_MIDDLE = 6, 
       CLIMATE_FAN_FOCUS = 7, CLIMATE_FAN_DIFFUSE = 8, CLIMATE_FAN_QUIET = 9  }; 
  enum ClimateSwingMode : uint8_t { 
       CLIMATE_SWING_OFF = 0, CLIMATE_SWING_BOTH = 1, CLIMATE_SWING_VERTICAL = 2, CLIMATE_SWING_HORIZONTAL = 3  };

  // Параметры, считываемые непосредственно с кондиционера
  
  uint8_t real_current_temperature; // Реальная температура
  uint8_t real_target_temperature;  // Установленная температура
  uint8_t real_ac_mode;             // Режим
  uint8_t real_fan_mode;            // Режим вентилятора
  uint8_t real_swing_mode;          // Режим качания шторок
  uint8_t real_ac_turbo;            // Режим "турбо"
  uint8_t real_ac_power;            // Питание

  // Следующие параметры не считываются с кондиционера, должны хранится локально

  uint8_t target_temperature = 255; // Установленная температура
  uint8_t ac_mode = 255;            // Режим
  uint8_t fan_mode = 255;           // Режим вентилятора
  uint8_t swing_mode = 255;         // Режим качания шторок
  uint8_t ac_turbo = 255;           // Режим "турбо"
  uint8_t ac_power = 255;           // Питание

  uint8_t ac_display = 1;           // Дисплей
  uint8_t ac_beep = 0;              // Пищалка

  uint8_t v_swing_mode;             // Качания вертикальной заслонки
  uint8_t h_swing_mode;             // Качания горизонтальных заслонок
  uint8_t v_fixing_mode;            // Фиксация вертикальной заслонки
  uint8_t h_fixing_mode;            // Фиксация горизонтальных заслонок
    
  bool normal_receive_date = false;   // С кондиционера получены верные данные

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // Программа основного цикла получсения данных от конд-ра
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  String TCL_loop()
    {
       if (Serial.available() > 0) // Есть принятые байты UART
       {
          if (Serial.read() != 0xBB) { return ""; }  // Ошибочный первый байт - выходим и ждем следующего байта
      
          dataRX[0] = 0xBB;          delay(5);
          dataRX[1] = Serial.read(); delay(5);
          dataRX[2] = Serial.read(); delay(5);
          dataRX[3] = Serial.read(); delay(5);
          dataRX[4] = Serial.read();
          auto raw = getHex(dataRX, 5);
          Serial.readBytes(dataRX+5, dataRX[4]+1);
          byte check = getChecksum(dataRX, sizeof(dataRX));
          raw = getHex(dataRX, sizeof(dataRX));

          if (check != dataRX[60]) {      // Invalid checksum
             normal_receive_date = false;
             return raw.c_str(); //return "";
          } else {
             normal_receive_date = true;
             processing_readData();  // Обрабатываем полученные данные
             return raw.c_str();
          }
       }
    }    

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // Обработка полученных данных
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void processing_readData() {
 
    // real_current_temperature - реальная температура
    // real_target_temperature  - установленная температура
    // real_ac_mode             - режим код-ра
    // real_fan_mode            - режим вентилятора
    // real_swing_mode          - режим качания шторок
    // real_ac_turbo            - режим турбо
    // real_ac_power            - питание
    
    real_current_temperature = float((( (dataRX[17] << 8) | dataRX[18] ) / 374 - 32)/1.8);
    real_target_temperature = (dataRX[FAN_SPEED_POS] & SET_TEMP_MASK) + 16;
    
    ///ESP_LOGD("TCL", "TEMP: %f ", current_temperature);

    uint8_t modeswitch = MODE_MASK & dataRX[MODE_POS];
    switch (modeswitch) {
                case MODE_AUTO:     real_ac_mode = CLIMATE_MODE_AUTO;     break;
                case MODE_COOL:     real_ac_mode = CLIMATE_MODE_COOL;     break;
                case MODE_DRY:      real_ac_mode = CLIMATE_MODE_DRY;      break;
                case MODE_FAN_ONLY: real_ac_mode = CLIMATE_MODE_FAN_ONLY; break;
                case MODE_HEAT:     real_ac_mode = CLIMATE_MODE_HEAT;     break;
                default:            real_ac_mode = CLIMATE_MODE_AUTO;
    }

    if (!(dataRX[MODE_POS] & ( 1 << 4)))   // ++++++++++++++ Power OFF
    {
       //real_ac_mode    = CLIMATE_MODE_OFF;
       //real_fan_mode   = CLIMATE_FAN_OFF;
       //real_swing_mode = CLIMATE_SWING_OFF;
       
       real_ac_power = 0;
       real_ac_mode  = ac_mode; // При выключенном кондиционере режим всегода 'Авто', поэтому берем с установленного
    } else 
    {  real_ac_power = 1; }
    
    uint8_t fanspeedswitch = FAN_SPEED_MASK & dataRX[FAN_SPEED_POS];
    uint8_t swingmodeswitch = SWING_MODE_MASK & dataRX[SWING_POS];

    if      (dataRX[FAN_QUIET_POS] & FAN_QUIET) { real_fan_mode = CLIMATE_FAN_QUIET;   } 
    else if (dataRX[MODE_POS] & FAN_DIFFUSE)    { real_fan_mode = CLIMATE_FAN_DIFFUSE; } 
    else {
          switch (fanspeedswitch) {
             case FAN_AUTO:   real_fan_mode = CLIMATE_FAN_AUTO;   break;
             case FAN_LOW:    real_fan_mode = CLIMATE_FAN_LOW;    break;
             case FAN_MIDDLE: real_fan_mode = CLIMATE_FAN_MIDDLE; break;
             case FAN_MEDIUM: real_fan_mode = CLIMATE_FAN_MEDIUM; break;
             case FAN_HIGH:   real_fan_mode = CLIMATE_FAN_HIGH;   break;
             case FAN_FOCUS:  real_fan_mode = CLIMATE_FAN_FOCUS;  break;
             default:         real_fan_mode = CLIMATE_FAN_AUTO;          
          }
      }
      
    switch (swingmodeswitch) {
          case SWING_OFF:        real_swing_mode = CLIMATE_SWING_OFF;        break;
          case SWING_HORIZONTAL: real_swing_mode = CLIMATE_SWING_HORIZONTAL; break;
          case SWING_VERTICAL:   real_swing_mode = CLIMATE_SWING_VERTICAL;   break;
          case SWING_BOTH:       real_swing_mode = CLIMATE_SWING_BOTH;       break;
    } 

    // Режим турбо
    if (dataRX[MODE_POS] & ( 1 << 7)) { real_ac_turbo = 1; } else { real_ac_turbo = 0; }

    // Если параметры не заданы - берем с кондея
    if (target_temperature == 255) target_temperature = real_target_temperature;
    if (ac_mode == 255) ac_mode = real_ac_mode;
    if (fan_mode == 255) fan_mode = real_fan_mode;
    if (swing_mode == 255) swing_mode = real_swing_mode;
    if (ac_turbo == 255) ac_mode = real_ac_turbo;
    if (ac_power == 255) ac_mode = real_ac_power;
  }

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // Установка состояния кондиционера
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  void control() {
    
    uint8_t switchvar = 0;

    dataTX[7]  = 0b00000000; //eco,display,beep,ontimerenable, offtimerenable,power,0,0
    dataTX[8]  = 0b00000000; //mute,0,turbo,health,mode(4)  0=cool 1=fan  2=dry 3=heat 4=auto 
    dataTX[9]  = 0b00000000; //[9] = 0,0,0,0,temp(4) 31 - value
    dataTX[10] = 0b00000000; //[10] = 0,timerindicator,swingv(3),fan(3) 0=auto 1=low 2=med 3=high
    //                                {0,2,3,5,0};
    dataTX[11] = 0b00000000;
    dataTX[32] = 0b00000000;
    dataTX[33] = 0b00000000;
    
    switchvar = ac_mode;   // Режим  <--------------------------
        
    // Включаем пищалку если включен переключатель в настройках
    if (ac_beep) { dataTX[7] += 0b00100000; } // ON
    else         { dataTX[7] += 0b00000000; } // OFF
       
    // Включаем или отключаем дисплей на кондиционере в зависимости от переключателя в настройках
    // Включаем дисплей только если кондиционер в одном из рабочих режимов
    if (ac_display && switchvar != CLIMATE_MODE_OFF){ dataTX[7] += 0b01000000; } // ON
    else                                            { dataTX[7] += 0b00000000; } // OFF
   
    // Режим турбо
    if (ac_turbo == 1) { dataTX[8] += 0x40; } // ON
    else               { dataTX[8] += 0x00; } // OFF

    // Питание кондиционера
    if (ac_power == 1) { dataTX[7] += 0b00000100; } // ON
    else               { dataTX[7] += 0b00000000; } // OFF

    // Настраиваем режим работы кондиционера
    switch (switchvar) {
        case CLIMATE_MODE_OFF:      dataTX[8] += 0b00000000; break;
        case CLIMATE_MODE_AUTO:     dataTX[8] += 0b00001000; break;
        case CLIMATE_MODE_COOL:     dataTX[8] += 0b00000011; break;
        case CLIMATE_MODE_DRY:      dataTX[8] += 0b00000010; break;
        case CLIMATE_MODE_FAN_ONLY: dataTX[8] += 0b00000111; break;
        case CLIMATE_MODE_HEAT:     dataTX[8] += 0b00000001; break;
      }

    // Скорость вентилятора
    if     (fan_mode == CLIMATE_FAN_AUTO)   { dataTX[8] += 0b00000000;  dataTX[10]  += 0b00000000; } 
    else if(fan_mode == CLIMATE_FAN_QUIET)  { dataTX[8] += 0b10000000;  dataTX[10]  += 0b00000000; } 
    else if(fan_mode == CLIMATE_FAN_LOW)    { dataTX[8] += 0b00000000;  dataTX[10]  += 0b00000001; } 
    else if(fan_mode == CLIMATE_FAN_MIDDLE) { dataTX[8] += 0b00000000;  dataTX[10]  += 0b00000110; } 
    else if(fan_mode == CLIMATE_FAN_MEDIUM) { dataTX[8] += 0b00000000;  dataTX[10]  += 0b00000011; } 
    else if(fan_mode == CLIMATE_FAN_HIGH)   { dataTX[8] += 0b00000000;  dataTX[10]  += 0b00000111; } 
    else if(fan_mode == CLIMATE_FAN_FOCUS)  { dataTX[8] += 0b00000000;  dataTX[10]  += 0b00000101; } 
    else if(fan_mode == CLIMATE_FAN_DIFFUSE){ dataTX[8] += 0b01000000;  dataTX[10]  += 0b00000000; }

    // Режим качания заслонок
    switch(swing_mode) {
       case CLIMATE_SWING_OFF:        dataTX[10]  += 0b00000000; dataTX[11]  += 0b00000000; break;
       case CLIMATE_SWING_VERTICAL:   dataTX[10]  += 0b00111000; dataTX[11]  += 0b00000000; break;
       case CLIMATE_SWING_HORIZONTAL: dataTX[10]  += 0b00000000; dataTX[11]  += 0b00001000; break;
       case CLIMATE_SWING_BOTH:       dataTX[10]  += 0b00111000; dataTX[11]  += 0b00001000; break;
    }

    // Выбираем режим для качания вертикальной заслонки
    switch(v_swing_mode) {
      case 0:  dataTX[32]  += 0b00001000; break;   ///ESP_LOGD("TCL", "Vertical swing: up-down");
      case 1:  dataTX[32]  += 0b00010000; break;   ///ESP_LOGD("TCL", "Vertical swing: upper");
      case 2:  dataTX[32]  += 0b00011000; break;   ///ESP_LOGD("TCL", "Vertical swing: downer");
    }
    
    // Выбираем режим для качания горизонтальных заслонок
    switch(h_swing_mode) {
      case 0:  dataTX[33]  += 0b00001000; break;  ///ESP_LOGD("TCL", "Horizontal swing: left-right");
      case 1:  dataTX[33]  += 0b00010000; break;  ///ESP_LOGD("TCL", "Horizontal swing: lefter");
      case 2:  dataTX[33]  += 0b00011000; break;  ///ESP_LOGD("TCL", "Horizontal swing: center");
      case 3:  dataTX[33]  += 0b00100000; break;  ///ESP_LOGD("TCL", "Horizontal swing: righter");
    }
    
    //Выбираем положение фиксации вертикальной заслонки
    switch(v_fixing_mode) {
      case 0:  dataTX[32]  += 0b00000000; break;  ///ESP_LOGD("TCL", "Vertical fix: last position");
      case 1:  dataTX[32]  += 0b00000001; break;  ///ESP_LOGD("TCL", "Vertical fix: up");
      case 2:  dataTX[32]  += 0b00000010; break;  ///ESP_LOGD("TCL", "Vertical fix: upper");
      case 3:  dataTX[32]  += 0b00000011; break;  ///ESP_LOGD("TCL", "Vertical fix: center");
      case 4:  dataTX[32]  += 0b00000100; break;  ///ESP_LOGD("TCL", "Vertical fix: downer");
      case 5:  dataTX[32]  += 0b00000101; break;  ///ESP_LOGD("TCL", "Vertical fix: down");
    }
    
    //Выбираем положение фиксации горизонтальных заслонок
    switch(h_fixing_mode) {
      case 0:  dataTX[33]  += 0b00000000; break;  ///ESP_LOGD("TCL", "Horizontal fix: last position");
      case 1:  dataTX[33]  += 0b00000001; break;  ///ESP_LOGD("TCL", "Horizontal fix: left");
      case 2:  dataTX[33]  += 0b00000010; break;  ///ESP_LOGD("TCL", "Horizontal fix: lefter");
      case 3:  dataTX[33]  += 0b00000011; break;  ///ESP_LOGD("TCL", "Horizontal fix: center");
      case 4:  dataTX[33]  += 0b00000100; break;  ///ESP_LOGD("TCL", "Horizontal fix: righter");
      case 5:  dataTX[33]  += 0b00000101; break;  ///ESP_LOGD("TCL", "Horizontal fix: right");
    }

    // Расчет и установка температуры
    dataTX[9] = 31-(int)target_temperature;

    //Собираем массив байт для отправки в кондиционер
    dataTX[0] = 0xBB; //start byte
    dataTX[1] = 0x00; //start byte
    dataTX[2] = 0x01; //start byte
    dataTX[3] = 0x03; //0x03 - управление, 0x04 - опрос
    dataTX[4] = 0x20; //0x20 - управление, 0x19 - опрос
    dataTX[5] = 0x03; //??
    dataTX[6] = 0x01; //??
    //dataTX[7] = 0x64;   //eco,display,beep,ontimerenable, offtimerenable,power,0,0
    //dataTX[8] = 0x08;   //mute,0,turbo,health, mode(4) mode 01 heat, 02 dry, 03 cool, 07 fan, 08 auto, health(+16), 41=turbo-heat 43=turbo-cool (turbo = 0x40+ 0x01..0x08)
    //dataTX[9] = 0x0f;   //0 -31 ;    15 - 16 0,0,0,0, temp(4) settemp 31 - x
    //dataTX[10] = 0x00;  //0,timerindicator,swingv(3),fan(3) fan+swing modes //0=auto 1=low 2=med 3=high
    //dataTX[11] = 0x00;  //0,offtimer(6),0
    dataTX[12] = 0x00;    //fahrenheit,ontimer(6),0 cf 80=f 0=c
    dataTX[13] = 0x01;  //??
    dataTX[14] = 0x00;  //0,0,halfdegree,0,0,0,0,0
    dataTX[15] = 0x00;  //??
    dataTX[16] = 0x00;  //??
    dataTX[17] = 0x00;  //??
    dataTX[18] = 0x00;  //??
    dataTX[19] = 0x00;  //sleep on = 1 off=0
    dataTX[20] = 0x00;  //??
    dataTX[21] = 0x00;  //??
    dataTX[22] = 0x00;  //??
    dataTX[23] = 0x00;  //??
    dataTX[24] = 0x00;  //??
    dataTX[25] = 0x00;  //??
    dataTX[26] = 0x00;  //??
    dataTX[27] = 0x00;  //??
    dataTX[28] = 0x00;  //??
    dataTX[30] = 0x00;  //??
    dataTX[31] = 0x00;  //??
    //dataTX[32] = 0x00;  //0,0,0,режим вертикального качания(2),режим вертикальной фиксации(3)
    //dataTX[33] = 0x00;  //0,0,режим горизонтального качания(3),режим горизонтальной фиксации(3)
    dataTX[34] = 0x00;  //??
    dataTX[35] = 0x00;  //??
    dataTX[36] = 0x00;  //??
    dataTX[37] = 0xFF;  //Checksum
    dataTX[37] = getChecksum(dataTX, sizeof(dataTX));

    Serial.write(dataTX, sizeof(dataTX));   // Отправляем на кондей

    }
    
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // Запрос состояния кондиционера
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  void GetValue (){
     Serial.write(poll, 8);
  }

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  String getHex(byte *message, byte size) {
        String raw;
        for (int i = 0; i < size; i++) {
            raw += "\n" + String(message[i]);
        }
        raw.toUpperCase();
        return raw;
  }

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  byte getChecksum(const byte * message, size_t size) {
        byte position = size - 1;
        byte crc = 0;
        for (int i = 0; i < position; i++)
            crc ^= message[i];
        return crc;
  }
  
};
