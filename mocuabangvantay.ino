#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
int relay = 10;
Servo myServo;
unsigned long startTime;  // Biến để lưu thời gian bắt đầu quét vân tay
const unsigned long timeout = 10000;  // Thời gian chờ tối đa 30 giây

// Cấu hình keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Mật khẩu mở khóa qua keypad
String password = "1234";
String inputPassword = "";
int id = 0; 
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)

SoftwareSerial mySerial(11, 12);

#else

#define mySerial Serial2

#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  myServo.attach(10); // Gắn servo vào chân 10
  myServo.write(0);
  displayWaitFinger();
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit Kiem tra nhan dang van tay !");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Da tim thay cam bien van tay!");
  } else {
    Serial.println("Khong tim thay cam bien van tay!(");
    while (1) {
      delay(1);
    }
  }

  Serial.println(F("Doc thong so"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Khong tim thay dau van tay.");
  }
  else {
    Serial.println("Cho van tay");
    Serial.print("Cam bien chua "); Serial.print(finger.templateCount); Serial.println(" mẫu");
  }
}

void loop()                     
{
  while (true) {  // Liên tục quét vân tay
    char key = keypad.getKey();  // Kiểm tra phím được nhấn

    if (key == 'A') {  // Nếu phím A được nhấn, chuyển sang nhập mật khẩu
      requestKeypadPassword();  // Chuyển sang mở khóa bằng keypad
    }
    if (getFingerprintIDez() != -1) {
      Serial.println("Mo cua bang van tay!");
      displayFingerOK();
      delay(3000);
      
      Serial.println("Dong cua!");
      displayWaitFinger();
    } else if (getFingerprintID() == FINGERPRINT_NOTFOUND) {
      displayInvalidFinger();  // Hiển thị thông báo lỗi nếu không tìm thấy vân tay
    }
    delay(50);  // Giới hạn tốc độ vòng lặp
  }
}

void requestKeypadPassword() {
  lcd.clear();
  lcd.print("Nhap ma keypad:");
  inputPassword = "";  // Xóa mật khẩu đã nhập

  while (inputPassword.length() < 4) {
    char key = keypad.getKey();
    if (key) {
      inputPassword += key;
      lcd.setCursor(inputPassword.length() - 1, 1);  // Hiển thị ký tự đã nhập
      lcd.print('*');
    }
  }

  if (inputPassword == password) {  // Nếu mật khẩu đúng
    lcd.clear();
    lcd.print("Mo cua");
    myServo.write(180);  // Mở cửa
    delay(3000);
    myServo.write(0);  // Đóng cửa
    displayWaitFinger();
  } else {
    lcd.clear();
    lcd.print("Ma sai!!!");
    delay(2000);
    requestKeypadPassword();  // Hiển thị lại yêu cầu vân tay
  }
}

//=======================================
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // =========================OK success!========================

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted || Hinh anh duoc chuyen doi");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // ========================OK converted!=======================
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("\nDid not find a match");
    displayInvalidFinger();
    delay(2000);
    displayWaitFinger();
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // ======================found a match!=================
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if fairelay, otherwise returns ID #
int getFingerprintIDez() 
{
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

//======================LCD=================================
void displayWaitFinger()
{
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.setCursor(3, 0);
  lcd.print("NHAP VAN TAY");
  lcd.setCursor(1, 1);
  lcd.print("LEN MAN HINH");
}
void displayInvalidFinger()
{
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("VAN TAY");
  lcd.setCursor(3, 1);
  lcd.print("BI LOI!!!");
}
void displayFingerOK()

{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" WELCOME TO YOU");
  myServo.write(180); // Mở cửa
  delay(3000); // Giữ 2 giây
  myServo.write(0); // Đóng cửa
}
