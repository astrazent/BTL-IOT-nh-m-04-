#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

// Khai báo chân điều khiển servo
Servo myServo;

// Khai báo LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Khai báo bàn phím
const byte ROWS = 4; // 4 hàng
const byte COLS = 4; // 4 cột
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; // Chân hàng
byte colPins[COLS] = {5, 4, 3, 2}; // Chân cột

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Mật khẩu
String password = "1234"; // Mật khẩu mặc định
String inputPassword = "";

void setup() {
  lcd.init();
  lcd.backlight();
  myServo.attach(10); // Gắn servo vào chân 9
  myServo.write(0); // Đặt servo ở vị trí đóng

  lcd.setCursor(0, 0);
  lcd.print("Nhap mat khau:");
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    if (key == '#') { // Nhấn '#' để kiểm tra mật khẩu
      if (inputPassword == password) {
        lcd.clear();
        lcd.print("Mat khau dung!");
        myServo.write(180); // Mở cửa
        delay(3000); // Giữ 2 giây
        myServo.write(0); // Đóng cửa
        lcd.clear();
        lcd.print("Nhap mat khau:");
        inputPassword = ""; // Reset mật khẩu đã nhập
      } else {
        lcd.clear();
        lcd.print("Mat khau sai!");
        delay(2000); // Hiển thị 2 giây
        lcd.clear();
        lcd.print("Nhap mat khau:");
        inputPassword = ""; // Reset mật khẩu đã nhập
      }
    } else if (key == '*') { // Nhấn '*' để xóa mật khẩu đã nhập
      inputPassword = "";
      lcd.clear();
      lcd.print("Nhap mat khau:");
    } else { // Thêm ký tự vào mật khẩu đã nhập
      inputPassword += key;
      lcd.setCursor(0, 1);
      lcd.print(inputPassword);
    }
  }
}
