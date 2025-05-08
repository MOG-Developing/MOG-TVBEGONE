#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#define IR_LED_PIN 4
#define EEPROM_SIZE 512

const char* ssid = "MOG-TVBEGONE";
const char* password = "TVBEGONE123";

AsyncWebServer server(80);

struct IRCommand {
  String name;
  uint32_t code;
  uint8_t bits;
  uint8_t protocol;
};

struct DeviceCategory {
  String name;
  std::vector<IRCommand> commands;
};

std::vector<DeviceCategory> deviceCategories;

enum IRProtocol {
  PROTO_NEC = 1,
  PROTO_SONY = 2,
  PROTO_RC5 = 3,
  PROTO_RC6 = 4,
  PROTO_SAMSUNG = 5,
  PROTO_LG = 6,
  PROTO_JVC = 7,
  PROTO_PANASONIC = 8,
  PROTO_DISH = 9,
  PROTO_SHARP = 10,
  PROTO_DENON = 11,
  PROTO_MITSUBISHI = 12
};

void setup() {
  Serial.begin(115200);
  pinMode(IR_LED_PIN, OUTPUT);
  digitalWrite(IR_LED_PIN, LOW);
  
  EEPROM.begin(EEPROM_SIZE);
  setupDeviceDatabase();
  
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  setupWebServer();
  
  Serial.println("MOG-TVBEGONE ESP32WROOM32U Ready!");
}

void loop() {
  yield();
}

void setupWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>MOG-TVBEGONE Control</title>
  <style>
    :root {
      --primary: #4a6fa5;
      --secondary: #166088;
      --accent: #4fc3f7;
      --background: #f5f7fa;
      --card: #ffffff;
      --text: #333333;
      --text-light: #666666;
    }
    * {
      box-sizing: border-box;
    }
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      margin: 0;
      padding: 0;
      background-color: var(--background);
      color: var(--text);
      line-height: 1.6;
    }
    .container {
      max-width: 1200px;
      margin: 0 auto;
      padding: 20px;
    }
    header {
      background-color: var(--primary);
      color: white;
      padding: 20px 0;
      text-align: center;
      margin-bottom: 30px;
      box-shadow: 0 2px 5px rgba(0,0,0,0.1);
    }
    h1 {
      margin: 0;
      font-size: 2rem;
    }
    h2 {
      color: var(--secondary);
      border-bottom: 2px solid var(--accent);
      padding-bottom: 8px;
      margin-top: 25px;
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
      gap: 20px;
      margin-bottom: 30px;
    }
    .card {
      background-color: var(--card);
      border-radius: 8px;
      padding: 20px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.05);
      transition: transform 0.3s ease;
    }
    .card:hover {
      transform: translateY(-5px);
      box-shadow: 0 5px 15px rgba(0,0,0,0.1);
    }
    .btn {
      display: inline-block;
      background-color: var(--primary);
      color: white;
      padding: 10px 15px;
      border-radius: 4px;
      text-decoration: none;
      border: none;
      cursor: pointer;
      font-size: 0.9rem;
      margin: 5px;
      transition: background-color 0.3s;
      width: 100%;
      text-align: center;
    }
    .btn:hover {
      background-color: var(--secondary);
    }
    .btn-universal {
      background-color: #e74c3c;
    }
    .btn-universal:hover {
      background-color: #c0392b;
    }
    .status {
      text-align: center;
      padding: 10px;
      margin: 20px 0;
      border-radius: 4px;
      display: none;
    }
    .success {
      background-color: #d4edda;
      color: #155724;
    }
    .error {
      background-color: #f8d7da;
      color: #721c24;
    }
    @media (max-width: 768px) {
      .grid {
        grid-template-columns: 1fr;
      }
    }
  </style>
</head>
<body>
  <header>
    <div class="container">
      <h1>TV-B-Gone Made by MOG-Developing</h1>
    </div>
  </header>
  
  <div class="container">
    <div id="status" class="status"></div>
    
    <div class="card">
      <h2>Universal Commands</h2>
      <button class="btn btn-universal" onclick="sendUniversalOff()">Turn All Devices OFF</button>
    </div>
    
    <div class="grid">
)=====";

    for (size_t i = 0; i < deviceCategories.size(); i++) {
      html += "<div class='card'><h2>" + deviceCategories[i].name + "</h2>";
      for (size_t j = 0; j < deviceCategories[i].commands.size(); j++) {
        html += "<button class='btn' onclick='sendCommand(" + String(i) + "," + String(j) + ")'>" + 
                deviceCategories[i].commands[j].name + "</button>";
      }
      html += "</div>";
    }
    
    html += R"=====(
    </div>
  </div>
  
  <script>
    function showStatus(message, isSuccess) {
      const status = document.getElementById('status');
      status.textContent = message;
      status.className = 'status ' + (isSuccess ? 'success' : 'error');
      status.style.display = 'block';
      setTimeout(() => {
        status.style.display = 'none';
      }, 3000);
    }
    
    function sendCommand(category, command) {
      fetch('/send?category=' + category + '&command=' + command)
        .then(response => response.text())
        .then(text => showStatus(text, true))
        .catch(error => showStatus('Error: ' + error, false));
    }
    
    function sendUniversalOff() {
      fetch('/universal')
        .then(response => response.text())
        .then(text => showStatus(text, true))
        .catch(error => showStatus('Error: ' + error, false));
    }
  </script>
</body>
</html>
)=====";
    request->send(200, "text/html", html);
  });

  server.on("/send", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("category") && request->hasParam("command")) {
      int categoryIndex = request->getParam("category")->value().toInt();
      int commandIndex = request->getParam("command")->value().toInt();
      
      if (categoryIndex >= 0 && categoryIndex < deviceCategories.size()) {
        DeviceCategory& category = deviceCategories[categoryIndex];
        if (commandIndex >= 0 && commandIndex < category.commands.size()) {
          IRCommand& cmd = category.commands[commandIndex];
          sendIRCommand(cmd);
          request->send(200, "text/plain", "Sent " + category.name + " command: " + cmd.name);
          return;
        }
      }
    }
    request->send(400, "text/plain", "Invalid parameters");
  });

  server.on("/universal", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendUniversalOff();
    request->send(200, "text/plain", "Sending universal OFF commands to all devices");
  });

  server.begin();
}

void sendPulse(uint16_t length_us) {
  uint32_t start = micros();
  while (micros() - start < length_us) {
    digitalWrite(IR_LED_PIN, HIGH);
    delayMicroseconds(13);
    digitalWrite(IR_LED_PIN, LOW);
    delayMicroseconds(13);
  }
}

void sendSpace(uint16_t length_us) {
  digitalWrite(IR_LED_PIN, LOW);
  if (length_us > 16383) {
    delayMicroseconds(16383);
    length_us -= 16383;
  }
  delayMicroseconds(length_us);
}

void sendNEC(uint32_t data, uint8_t bits) {
  sendPulse(9000);
  sendSpace(4500);
  
  for (uint32_t mask = 1UL << (bits - 1); mask; mask >>= 1) {
    sendPulse(560);
    if (data & mask) {
      sendSpace(1690);
    } else {
      sendSpace(560);
    }
  }
  
  sendPulse(560);
  sendSpace(560);
}

void sendSony(uint32_t data, uint8_t bits) {
  sendPulse(2400);
  sendSpace(600);
  
  for (uint32_t mask = 1UL << (bits - 1); mask; mask >>= 1) {
    sendPulse(600);
    if (data & mask) {
      sendSpace(1200);
    } else {
      sendSpace(600);
    }
  }
}

void sendSamsung(uint32_t data, uint8_t bits) {
  sendPulse(4500);
  sendSpace(4500);
  
  for (uint32_t mask = 1UL << (bits - 1); mask; mask >>= 1) {
    sendPulse(560);
    if (data & mask) {
      sendSpace(1600);
    } else {
      sendSpace(560);
    }
  }
  
  sendPulse(560);
  sendSpace(560);
}

void sendRC5(uint32_t data, uint8_t bits) {
  uint32_t mask = 1UL << (bits - 1);
  bool toggleBit = false;
  bool field = true;
  
  sendSpace(889);
  sendPulse(889);
  
  while (mask) {
    if (data & mask) {
      if (field) {
        sendSpace(889);
        sendPulse(889);
      } else {
        sendPulse(889);
        sendSpace(889);
      }
    } else {
      if (field) {
        sendPulse(889);
        sendSpace(889);
      } else {
        sendSpace(889);
        sendPulse(889);
      }
    }
    field = !field;
    mask >>= 1;
  }
}

void sendLG(uint32_t data, uint8_t bits) {
  sendPulse(8000);
  sendSpace(4000);
  
  for (uint32_t mask = 1UL << (bits - 1); mask; mask >>= 1) {
    sendPulse(550);
    if (data & mask) {
      sendSpace(1500);
    } else {
      sendSpace(550);
    }
  }
  
  sendPulse(550);
}

void sendPanasonic(uint32_t data, uint8_t bits) {
  sendPulse(3500);
  sendSpace(1750);
  
  for (uint32_t mask = 1UL << (bits - 1); mask; mask >>= 1) {
    sendPulse(450);
    if (data & mask) {
      sendSpace(1300);
    } else {
      sendSpace(450);
    }
  }
  
  sendPulse(450);
}

void sendDish(uint32_t data, uint8_t bits) {
  sendPulse(450);
  sendSpace(450);
  
  for (uint32_t mask = 1UL << (bits - 1); mask; mask >>= 1) {
    if (data & mask) {
      sendPulse(450);
      sendSpace(1650);
    } else {
      sendPulse(450);
      sendSpace(450);
    }
  }
}

void sendJVC(uint32_t data, uint8_t bits) {
  sendPulse(8400);
  sendSpace(4200);
  
  for (uint32_t mask = 1UL << (bits - 1); mask; mask >>= 1) {
    sendPulse(525);
    if (data & mask) {
      sendSpace(1575);
    } else {
      sendSpace(525);
    }
  }
  
  sendPulse(525);
}

void sendIRCommand(IRCommand& cmd) {
  switch (cmd.protocol) {
    case PROTO_NEC:
      sendNEC(cmd.code, cmd.bits);
      delay(40);
      sendNEC(cmd.code, cmd.bits);
      break;
    case PROTO_SONY:
      sendSony(cmd.code, cmd.bits);
      delay(40);
      sendSony(cmd.code, cmd.bits);
      break;
    case PROTO_SAMSUNG:
      sendSamsung(cmd.code, cmd.bits);
      delay(40);
      sendSamsung(cmd.code, cmd.bits);
      break;
    case PROTO_RC5:
      sendRC5(cmd.code, cmd.bits);
      delay(40);
      sendRC5(cmd.code, cmd.bits);
      break;
    case PROTO_LG:
      sendLG(cmd.code, cmd.bits);
      delay(40);
      sendLG(cmd.code, cmd.bits);
      break;
    case PROTO_PANASONIC:
      sendPanasonic(cmd.code, cmd.bits);
      delay(40);
      sendPanasonic(cmd.code, cmd.bits);
      break;
    case PROTO_DISH:
      sendDish(cmd.code, cmd.bits);
      delay(40);
      sendDish(cmd.code, cmd.bits);
      break;
    case PROTO_JVC:
      sendJVC(cmd.code, cmd.bits);
      delay(40);
      sendJVC(cmd.code, cmd.bits);
      break;
    default:
      sendNEC(cmd.code, cmd.bits);
      delay(40);
      sendNEC(cmd.code, cmd.bits);
      break;
  }
}

void sendUniversalOff() {
  for (auto& category : deviceCategories) {
    for (auto& cmd : category.commands) {
      if (cmd.name.indexOf("OFF") >= 0 || cmd.name.indexOf("POWER") >= 0) {
        sendIRCommand(cmd);
        delay(100);
      }
    }
  }
}

void setupDeviceDatabase() {
  DeviceCategory tvs;
  tvs.name = "TVs";
  tvs.commands = {
    {"SAMSUNG TV POWER", 0xE0E040BF, 32, PROTO_SAMSUNG},
    {"LG TV POWER", 0x20DF10EF, 32, PROTO_LG},
    {"SONY TV POWER", 0xA90, 12, PROTO_SONY},
    {"PANASONIC TV POWER", 0x100BCBD, 32, PROTO_PANASONIC},
    {"SHARP TV POWER", 0x41A2D, 15, PROTO_NEC},
    {"TCL/ROKU TV POWER", 0x57E3D8CB, 32, PROTO_NEC},
    {"VIZIO TV POWER", 0xD2F340BF, 32, PROTO_NEC},
    {"PHILIPS TV POWER", 0xA50, 12, PROTO_RC5},
    {"MITSUBISHI TV POWER", 0xF50C03F, 32, PROTO_NEC},
    {"TOSHIBA TV POWER", 0x2FD48B7, 32, PROTO_NEC},
    {"JVC TV POWER", 0xC5E8, 16, PROTO_JVC},
    {"HISENSE TV POWER", 0xF807, 16, PROTO_NEC},
    {"XIAOMI TV POWER", 0x7E8154AB, 32, PROTO_NEC},
    {"SKYWORTH TV POWER", 0x1EE17887, 32, PROTO_NEC},
    {"CHANGHONG TV POWER", 0x4B36D32C, 32, PROTO_NEC},
    {"HAIER TV POWER", 0x7B84C0B8, 32, PROTO_NEC},
    {"COOCAA TV POWER", 0xDE21A35C, 32, PROTO_NEC},
    {"ONIDA TV POWER", 0x41B615C9, 32, PROTO_NEC},
    {"VU TV POWER", 0x1CE340BF, 32, PROTO_NEC},
    {"REALME TV POWER", 0xDD4477BB, 32, PROTO_NEC}
  };
  deviceCategories.push_back(tvs);
  
  DeviceCategory acs;
  acs.name = "Air Conditioners";
  acs.commands = {
    {"SAMSUNG AC POWER", 0x8813440, 28, PROTO_SAMSUNG},
    {"LG AC POWER", 0x880254A, 28, PROTO_LG},
    {"GREE AC POWER", 0xB24D7800, 32, PROTO_NEC},
    {"DAIKIN AC POWER", 0x1FE00D, 24, PROTO_NEC},
    {"MITSUBISHI AC POWER", 0x1C572F80, 32, PROTO_MITSUBISHI},
    {"FUJITSU AC POWER", 0x844B0401, 32, PROTO_NEC},
    {"PANASONIC AC POWER", 0x8272044, 24, PROTO_PANASONIC},
    {"CARRIER AC POWER", 0x8166B47, 32, PROTO_NEC},
    {"TOSHIBA AC POWER", 0xFDA05F, 24, PROTO_NEC},
    {"HITACHI AC POWER", 0x80F0401, 32, PROTO_NEC},
    {"WHIRLPOOL AC POWER", 0x4B36F00F, 32, PROTO_NEC},
    {"VOLTAS AC POWER", 0xA55A50AF, 32, PROTO_NEC},
    {"BLUE STAR AC POWER", 0x10E040BF, 32, PROTO_NEC},
    {"LLOYD AC POWER", 0x40BF00FF, 32, PROTO_NEC},
    {"HAIER AC POWER", 0x77E15080, 32, PROTO_NEC},
    {"GENERAL AC POWER", 0x57E3D807, 32, PROTO_NEC},
    {"KELVINATOR AC POWER", 0x8D520000, 32, PROTO_NEC},
    {"SHARP AC POWER", 0x9DA320DF, 32, PROTO_NEC},
    {"MIDEA AC POWER", 0xDD4477BB, 32, PROTO_NEC},
    {"TCL AC POWER", 0x1CE340BF, 32, PROTO_NEC}
  };
  deviceCategories.push_back(acs);
  
  DeviceCategory projectors;
  projectors.name = "Projectors & Smart Boards";
  projectors.commands = {
    {"EPSON POWER", 0x84D0, 16, PROTO_NEC},
    {"BENQ POWER", 0x40040FB7, 32, PROTO_NEC},
    {"OPTOMA POWER", 0xC24D827D, 32, PROTO_NEC},
    {"VIEWSONIC POWER", 0x7F0C837, 32, PROTO_NEC},
    {"SMARTBOARD POWER", 0x8F32005, 32, PROTO_NEC},
    {"PROMETHEAN POWER", 0x807F807, 32, PROTO_NEC},
    {"SONY PROJECTOR POWER", 0xA8B7, 16, PROTO_SONY},
    {"HITACHI PROJECTOR POWER", 0xC1AA09F6, 32, PROTO_NEC},
    {"NEC PROJECTOR POWER", 0x1EE17887, 32, PROTO_NEC},
    {"CASIO PROJECTOR POWER", 0x4B36D32C, 32, PROTO_NEC},
    {"PANASONIC PROJECTOR POWER", 0x7B84C0B8, 32, PROTO_NEC},
    {"LG PROJECTOR POWER", 0xDE21A35C, 32, PROTO_LG},
    {"DELL PROJECTOR POWER", 0x41B615C9, 32, PROTO_NEC},
    {"ACER PROJECTOR POWER", 0x1CE340BF, 32, PROTO_NEC},
    {"ASK PROJECTOR POWER", 0xDD4477BB, 32, PROTO_NEC}
  };
  deviceCategories.push_back(projectors);
  
  DeviceCategory audio;
  audio.name = "Audio Systems";
  audio.commands = {
    {"SONY AUDIO POWER", 0xA81, 12, PROTO_SONY},
    {"SAMSUNG AUDIO POWER", 0xE0E0D827, 32, PROTO_SAMSUNG},
    {"LG AUDIO POWER", 0x20DF02FD, 32, PROTO_LG},
    {"BOSE POWER", 0x4B36D32C, 32, PROTO_NEC},
    {"YAMAHA POWER", 0x7E8154AB, 32, PROTO_NEC},
    {"DENON POWER", 0x2A4C0, 24, PROTO_DENON},
    {"ONKYO POWER", 0x4B36F00F, 32, PROTO_NEC},
    {"PIONEER POWER", 0xA55A50AF, 32, PROTO_NEC},
    {"JBL POWER", 0x10E040BF, 32, PROTO_NEC},
    {"HARMAN KARDON POWER", 0x40BF00FF, 32, PROTO_NEC},
    {"MARANTZ POWER", 0x77E15080, 32, PROTO_NEC},
    {"KLIPSCH POWER", 0x57E3D807, 32, PROTO_NEC},
    {"SONOS POWER", 0x8D520000, 32, PROTO_NEC},
    {"BANG & OLUFSEN POWER", 0x9DA320DF, 32, PROTO_NEC},
    {"KEF POWER", 0xDD4477BB, 32, PROTO_NEC},
    {"B&W POWER", 0x1CE340BF, 32, PROTO_NEC},
    {"TANNOY POWER", 0x1EE17887, 32, PROTO_NEC},
    {"WHARFEDALE POWER", 0x4B36D32C, 32, PROTO_NEC},
    {"ELAC POWER", 0x7B84C0B8, 32, PROTO_NEC},
    {"FOCAL POWER", 0xDE21A35C, 32, PROTO_NEC}
  };
  deviceCategories.push_back(audio);
  
  DeviceCategory smart;
  smart.name = "Smart Devices";
  smart.commands = {
    {"APPLE TV MENU", 0x77E15080, 32, PROTO_NEC},
    {"ROKU POWER", 0x57E3D807, 32, PROTO_NEC},
    {"AMAZON FIRE TV POWER", 0x8D520000, 32, PROTO_NEC},
    {"CHROMECAST POWER", 0x9DA320DF, 32, PROTO_NEC},
    {"XBOX POWER", 0xDD4477BB, 32, PROTO_NEC},
    {"PLAYSTATION POWER", 0x1CE340BF, 32, PROTO_SONY},
    {"NVIDIA SHIELD POWER", 0x1EE17887, 32, PROTO_NEC},
    {"SMART TOILET POWER", 0x4B36D32C, 32, PROTO_NEC},
    {"SMART LIGHT POWER", 0x7B84C0B8, 32, PROTO_NEC},
    {"SMART FAN POWER", 0xDE21A35C, 32, PROTO_NEC},
    {"SMART PLUG POWER", 0x41B615C9, 32, PROTO_NEC},
    {"SMART CURTAIN POWER", 0x1CE340BF, 32, PROTO_NEC},
    {"SMART AC POWER", 0xDD4477BB, 32, PROTO_NEC},
    {"SMART CAMERA POWER", 0x77E15080, 32, PROTO_NEC},
    {"SMART DOORBELL POWER", 0x57E3D807, 32, PROTO_NEC},
    {"SMART LOCK POWER", 0x8D520000, 32, PROTO_NEC},
    {"SMART THERMOSTAT POWER", 0x9DA320DF, 32, PROTO_NEC},
    {"SMART VACUUM POWER", 0xDD4477BB, 32, PROTO_NEC},
    {"SMART FRIDGE POWER", 0x1CE340BF, 32, PROTO_NEC},
    {"SMART WASHER POWER", 0x1EE17887, 32, PROTO_NEC}
  };
  deviceCategories.push_back(smart);
}