#include <ESP8266WiFi.h>

#define SSID_REDE     ""
#define SENHA_REDE    ""
#define INTERVALO_ENVIO_THINGSPEAK  120000
#define INTERVALO_SENSOR_UMIDADE    30000

char EnderecoAPIThingSpeak[] = "api.thingspeak.com";
String ChaveEscritaThingSpeak = "";
long lastConnectionTime;
WiFiClient client;

void EnviaInformacoesThingspeak(String StringDados);
void FazConexaoWiFi(void);
float FazLeituraUmidade(void);

void EnviaInformacoesThingspeak(String StringDados) {
  if (client.connect(EnderecoAPIThingSpeak, 80)) {
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + ChaveEscritaThingSpeak + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(StringDados.length());
    client.print("\n\n");
    client.print(StringDados);

    lastConnectionTime = millis();
    Serial.println("- Informações enviadas ao ThingSpeak!");

    if (client.connected()) {
      client.stop();
      Serial.println("- Desconectado do ThingSpeak");
      Serial.println();
    }
  }

  digitalWrite(LED_BUILTIN, LOW);
  delay(300);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(300);
  digitalWrite(LED_BUILTIN, LOW);
  delay(300);
  digitalWrite(LED_BUILTIN, HIGH);
}

void FazConexaoWiFi(void) {
  client.stop();
  Serial.println("Conectando-se à rede WiFi...");
  Serial.println();
  delay(1000);
  WiFi.begin(SSID_REDE, SENHA_REDE);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connectado com sucesso!");
  Serial.println("IP obtido: ");
  Serial.println(WiFi.localIP());

  delay(1000);
}

float FazLeituraUmidade(void) {
  int ValorADC;
  float UmidadePercentual;

  digitalWrite(LED_BUILTIN, LOW);

  ValorADC = analogRead(0);
  Serial.print("[Leitura ADC] ");
  Serial.println(ValorADC);

  //Quanto maior o numero lido do ADC, menor a umidade.
  //Sendo assim, calcula-se a porcentagem de umidade por:
  //
  //   Valor lido                 Umidade percentual
  //      _    0                           _ 100
  //      |                                |
  //      |                                |
  //      -   ValorADC                     - UmidadePercentual
  //      |                                |
  //      |                                |
  //     _|_  1024                        _|_ 0
  UmidadePercentual = 100 * ((1024 - (float)ValorADC) / 1024);
  Serial.print("[Umidade Percentual] ");
  Serial.print(UmidadePercentual);
  Serial.println("%");
  
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);

  return UmidadePercentual;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);
  lastConnectionTime = 0;
  FazConexaoWiFi();
  Serial.println("Planta IoT com ESP8266 NodeMCU");
}

void loop() {
  float UmidadePercentualLida;
  int UmidadePercentualTruncada;
  char FieldUmidade[11];

  UmidadePercentualLida = FazLeituraUmidade();
  UmidadePercentualTruncada = (int)UmidadePercentualLida;

  if (!client.connected() && (millis() - lastConnectionTime > INTERVALO_ENVIO_THINGSPEAK)) {
    sprintf(FieldUmidade, "field1=%d", UmidadePercentualTruncada);
    EnviaInformacoesThingspeak(FieldUmidade);
  }

  delay(INTERVALO_SENSOR_UMIDADE);
}
